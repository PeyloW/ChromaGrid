//
//  system.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-11.
//

#include "system.hpp"

extern "C" {

    using namespace toybox;
    
#ifndef __M68000__
#include <unistd.h>
#include "system_host.hpp"
#endif

static int g_timer_ref_counts[2] = { 0 };

typedef struct __packed_struct {
    uint8_t freq;
    uint8_t cnt;
    timer_c::func_t func;
} cgtimer_func_s;
    
#define TIMER_FUNC_MAX_CNT 4
cgtimer_func_s g_vbl_functions[TIMER_FUNC_MAX_CNT+1] = { {0,0, nullptr} };
volatile uint32_t g_vbl_tick = 0;
cgtimer_func_s g_clock_functions[TIMER_FUNC_MAX_CNT+1] = { {0,0, nullptr} };
volatile uint32_t g_clock_tick = 0;
static rect_s g_mouse_limit;
static uint8_t g_prev_mouse_butons;
uint8_t g_mouse_buttons;
static mouse_c::state_e g_mouse_button_states[2];
point_s g_mouse_position;

#ifdef __M68000__
    extern timer_c::func_t g_system_vbl_interupt;
    extern void g_vbl_interupt();
    extern timer_c::func_t g_system_clock_interupt;
    extern void g_clock_interupt();
    extern timer_c::func_a_t g_system_mouse_interupt;
    extern void g_mouse_interupt(void *);
    static _KBDVECS *g_keyboard_vectors = nullptr;
#else
    void (*g_yield_function)() = nullptr;

    static void g_do_timer(cgtimer_func_s timer_funcs[], int freq) {
        for (int i = 0; i < TIMER_FUNC_MAX_CNT; i++) {
            if (timer_funcs[i].freq) {
                bool trigger = false;
                int cnt = (int)timer_funcs[i].cnt - timer_funcs[i].freq;
                if (cnt <= 0) {
                    trigger = true;
                    cnt += freq;
                }
                timer_funcs[i].cnt = (uint8_t)cnt;
                if (trigger) {
                    timer_funcs[i].func();
                }
            }
        }
    }
    
    void g_vbl_interupt() {
        if (g_timer_ref_counts[timer_c::vbl] > 0) {
            g_vbl_tick++;
            g_do_timer(g_vbl_functions, 50);
        }
    }

    void g_clock_interupt() {
        if (g_timer_ref_counts[timer_c::clock] > 0) {
            g_clock_tick++;
            g_do_timer(g_clock_functions, 200);
        }
    }
    
    // Host must call when mouse state changes
    void g_update_mouse(point_s position, bool left, bool right) {
        g_mouse_position = position;
        g_mouse_buttons = (left ? 2 : 0) | (right ? 1 : 0);
    }
#endif
}


timer_c::timer_c(timer_e timer) : _timer(timer) {
    assert(timer == vbl || timer == clock);
    g_timer_ref_counts[timer]++;
    if (g_timer_ref_counts[timer] == 1) {
        with_paused_timers([timer] {
            switch (timer) {
                case vbl:
                #ifdef __M68000__
                    g_system_vbl_interupt = *((func_t *)0x0070);
                    *((func_t *)0x0070) = &g_vbl_interupt;
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
}

timer_c::~timer_c() {
    g_timer_ref_counts[_timer]--;
    assert(g_timer_ref_counts[_timer] >= 0);
    if (g_timer_ref_counts[_timer] == 0) {
        with_paused_timers([this] {
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
        });
    }
}

uint8_t timer_c::base_freq() const {
    switch (_timer) {
        case vbl:
            return 50;
        case clock:
            return 200;
        default:
            assert(0);
            return 0;
    }
}

void timer_c::add_func(func_t func, uint8_t freq) {
    if (freq == 0) {
        freq = base_freq();
    }
    with_paused_timers([this, func, freq] {
        auto functions = _timer == vbl ? g_vbl_functions : g_clock_functions;
        for (int i = 0; i < TIMER_FUNC_MAX_CNT; i++) {
            if (functions[i].freq == 0) {
                functions[i] = { freq, base_freq(), func };
                return;
            }
        }
        assert(0);
    });
}

void timer_c::remove_func(func_t func) {
    with_paused_timers([this, func] {
        auto functions = _timer == vbl ? g_vbl_functions : g_clock_functions;
        int i;
        for (i = 0; i < TIMER_FUNC_MAX_CNT; i++) {
            if (functions[i].func == func) {
                break;
            }
        }
        for ( ; i < TIMER_FUNC_MAX_CNT; i++) {
            functions[i] = g_vbl_functions[i+1];
        }
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


mouse_c::mouse_c(rect_s limit) {
    g_mouse_limit = limit;
    g_mouse_position = (point_s){
        static_cast<int16_t>(limit.origin.x + limit.size.width / 2),
        static_cast<int16_t>(limit.origin.y + limit.size.height / 2)
    };
#ifdef __M68000__
    g_keyboard_vectors = Kbdvbase();
    g_system_mouse_interupt = g_keyboard_vectors->mousevec;
    g_keyboard_vectors->mousevec = &g_mouse_interupt;
#endif
}

mouse_c::~mouse_c() {
#ifdef __M68000__
    g_keyboard_vectors->mousevec = g_system_mouse_interupt;
#endif
}

void mouse_c::update_state() {
    for (int button = 2; --button != -1; ) {
        if (g_mouse_buttons & (1 << button)) {
            g_mouse_button_states[button] = pressed;
        } else if (g_prev_mouse_butons & (1 << button)) {
            g_mouse_button_states[button] = clicked;
        } else {
            g_mouse_button_states[button] = released;
        }
    }
    g_prev_mouse_butons = g_mouse_buttons;
}

bool mouse_c::is_pressed(button_e button) const {
    return (g_mouse_buttons & (1 << button)) != 0;
}

mouse_c::state_e mouse_c::get_state(button_e button) const {
    return g_mouse_button_states[button];
}

point_s mouse_c::get_postion() {
    point_s clamped_point = (point_s){
        (int16_t)(MIN(g_mouse_limit.origin.x + g_mouse_limit.size.width - 1, MAX(g_mouse_position.x, g_mouse_limit.origin.x))),
        (int16_t)(MIN(g_mouse_limit.origin.y + g_mouse_limit.size.height - 1, MAX(g_mouse_position.y, g_mouse_limit.origin.y)))
    };
    g_mouse_position = clamped_point;
    return clamped_point;
}
