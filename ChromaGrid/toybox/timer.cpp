//
//  timer.cpp
//  toybox
//
//  Created by Fredrik on 2024-04-24.
//

#include "timer.hpp"
#include "machine.hpp"
#include "forward_list.hpp"
#include "algorithm.hpp"

using namespace toybox;


#ifndef __M68000__
#include <unistd.h>
#include "system_host.hpp"
#endif

typedef struct __packed_struct {
uint8_t freq;
uint8_t cnt;
timer_c::func_a_t func;
void *context;
} timer_func_s;

#define TIMER_FUNC_MAX_CNT 16
typedef forward_list_c<timer_func_s, TIMER_FUNC_MAX_CNT> timer_func_list_c;
#ifdef __M68000__
static_assert(sizeof(timer_func_list_c::_node_s) == 14, "timer_func_list_c::_node_s) size mismatch");
#endif

timer_func_list_c g_vbl_functions;
volatile uint32_t g_vbl_tick = 0;
timer_func_list_c g_clock_functions;
volatile uint32_t g_clock_tick = 0;

extern "C" {
#ifdef __M68000__
#   if TOYBOX_TARGET_ATARI
    extern timer_c::func_t g_system_vbl_interupt;
    extern void g_vbl_interupt();
    extern int16_t g_system_vbl_freq;
    extern timer_c::func_t g_system_clock_interupt;
    extern void g_clock_interupt();
#   else
#       error "Usupported target"
#   endif
#else
    void (*g_yield_function)() = nullptr;
    
    static void g_do_timer(timer_func_list_c &timer_funcs, int freq) {
        for (auto &timer_func : timer_funcs) {
            bool trigger = false;
            int cnt = (int)timer_func.cnt - timer_func.freq;
            if (cnt <= 0) {
                trigger = true;
                cnt += freq;
            }
            timer_func.cnt = (uint8_t)cnt;
            if (trigger) {
                timer_func.func(timer_func.context);
            }
        }
    }
    
    void g_vbl_interupt() {
        g_vbl_tick++;
        g_do_timer(g_vbl_functions, 50);
    }
    
    void g_clock_interupt() {
        g_clock_tick++;
        g_do_timer(g_clock_functions, 200);
    }
#endif
}

template<>
timer_func_list_c::allocator::type timer_func_list_c::allocator::first_block = nullptr;


timer_c &timer_c::shared(timer_e timer) {
    switch (timer) {
        case vbl: {
            static timer_c s_timer_vbl(vbl);
            return s_timer_vbl;
        }
        case clock: {
            static timer_c s_timer_clock(clock);
            return s_timer_clock;
        }
        default:
            hard_assert(0);
            return *(timer_c*)0x0;
    }
}

timer_c::timer_c(timer_e timer) : _timer(timer) {
    assert(timer == vbl || timer == clock);
    machine_c::shared();
    with_paused_timers([timer] {
        switch (timer) {
            case vbl:
#ifdef __M68000__
                g_system_vbl_interupt = *((func_t *)0x0070);
                *((func_t *)0x0070) = &g_vbl_interupt;
                g_system_vbl_freq = *(uint8_t*)0xffff820a == 0 ? 60 : 50;
#endif
                break;
            case clock:
#ifdef __M68000__
                g_system_clock_interupt = *((func_t *)0x0114);
                *((func_t *)0x0114) = &g_clock_interupt;
#endif
                break;
        }
    });
}

timer_c::~timer_c() {
    with_paused_timers([this] {
#   if TOYBOX_TARGET_ATARI
        switch (_timer) {
            case vbl:
#ifdef __M68000__
                *((func_t *)0x0070) = g_system_vbl_interupt;
#endif
                break;
            case clock:
#ifdef __M68000__
                *((func_t *)0x0114) = g_system_clock_interupt;
#endif
                break;
        }
#else
#  error "Unsupported target"
#endif
    });
}

uint8_t timer_c::base_freq() const {
    switch (_timer) {
        case vbl:
#ifdef __M68000__
            return g_system_vbl_freq;
#else
            return 50;
#endif
            //
        case clock:
            return 200;
        default:
            assert(0);
            return 0;
    }
}

void timer_c::add_func(func_t func, uint8_t freq) {
    add_func((func_a_t)func, nullptr, freq);
}

void timer_c::remove_func(func_t func) {
    remove_func((func_a_t)func, nullptr);
}

void timer_c::add_func(func_a_t func, void *context, uint8_t freq) {
    if (freq == 0) {
        freq = base_freq();
    }
    with_paused_timers([this, func, context, freq] {
        auto &functions = _timer == vbl ? g_vbl_functions : g_clock_functions;
        functions.push_front((timer_func_s){freq, base_freq(), func, context});
    });
}

void timer_c::remove_func(func_a_t func, void *context) {
    with_paused_timers([this, func, context] {
        auto &functions = _timer == vbl ? g_vbl_functions : g_clock_functions;
        auto it = functions.before_begin();
        while (it._node->next) {
            if (it._node->next->value.func == func && it._node->next->value.context == context) {
                functions.erase_after(it);
                return;
            }
            it++;
        }
        assert(0);
    });
}

uint32_t timer_c::tick() {
    return (_timer == vbl) ? g_vbl_tick : g_clock_tick;
}

void timer_c::reset_tick() {
    if (_timer == vbl) {
        g_vbl_tick = 0;
    } else {
        g_clock_tick = 0;
    }
}

void timer_c::wait(int ticks) {
    const auto wait_tick = tick() + ticks;
    while (wait_tick >= tick()) {
#ifndef __M68000__
        g_yield_function();
#endif
    }
}
