//
//  system.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-11.
//

#include "system.hpp"

extern "C" {

#ifndef __M68000__
#include <unistd.h>
#include "system_host.hpp"
#endif

static int cgg_timer_ref_counts[2] = { 0 };

typedef struct __packed_struct {
    uint8_t freq;
    uint8_t cnt;
    cgtimer_c::func_t func;
} cgtimer_func_t;
    
#define TIMER_FUNC_MAX_CNT 4
cgtimer_func_t cgg_vbl_functions[TIMER_FUNC_MAX_CNT+1] = { {0,0, nullptr} };
volatile uint32_t cgg_vbl_tick = 0;
cgtimer_func_t cgg_timer_c_functions[TIMER_FUNC_MAX_CNT+1] = { {0,0, nullptr} };
volatile uint32_t cgg_timer_c_tick = 0;
static cgrect_t cgg_mouse_limit;
static uint8_t cgg_prev_mouse_butons;
uint8_t cgg_mouse_buttons;
static cgmouse_c::state_e cgg_mouse_button_states[2];
cgpoint_t cgg_mouse_position;

#ifdef __M68000__
    extern cgtimer_c::func_t cgg_system_vbl_interupt;
    extern void cgg_vbl_interupt();
    extern cgtimer_c::func_t cgg_system_timer_c_interupt;
    extern void cgg_timer_c_interupt();
    extern cgtimer_c::func_a_t cgg_system_mouse_interupt;
    extern void cgg_mouse_interupt(void *);
    static _KBDVECS *cgg_keyboard_vectors = nullptr;
#else
    void (*cgg_yield_function)() = nullptr;

    static void cgg_do_timer(cgtimer_func_t timer_funcs[], int freq) {
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
    
    void cgg_vbl_interupt() {
        if (cgg_timer_ref_counts[cgtimer_c::vbl] > 0) {
            cgg_vbl_tick++;
            cgg_do_timer(cgg_vbl_functions, 50);
        }
    }

    void cgg_timer_c_interupt() {
        if (cgg_timer_ref_counts[cgtimer_c::timer_c] > 0) {
            cgg_timer_c_tick++;
            cgg_do_timer(cgg_timer_c_functions, 200);
        }
    }
    
    // Host must call when mouse state changes
    void cgg_update_mouse(cgpoint_t position, bool left, bool right) {
        cgg_mouse_position = position;
        cgg_mouse_buttons = (left ? 2 : 0) | (right ? 1 : 0);
    }
#endif
}


cgtimer_c::cgtimer_c(timer_e timer) : _timer(timer) {
    assert(timer == vbl || timer == timer_c);
    cgg_timer_ref_counts[timer]++;
    if (cgg_timer_ref_counts[timer] == 1) {
        with_paused_timers([timer] {
            switch (timer) {
                case vbl:
                #ifdef __M68000__
                    cgg_system_vbl_interupt = *((func_t *)0x0070);
                    *((func_t *)0x0070) = &cgg_vbl_interupt;
                #endif
                    break;
                case timer_c:
                #ifdef __M68000__
                    cgg_system_timer_c_interupt = *((func_t *)0x0114);
                    *((func_t *)0x0114) = &cgg_timer_c_interupt;
                #endif
                    break;
            }
        });
    }
}

cgtimer_c::~cgtimer_c() {
    cgg_timer_ref_counts[_timer]--;
    assert(cgg_timer_ref_counts[_timer] >= 0);
    if (cgg_timer_ref_counts[_timer] == 0) {
        with_paused_timers([this] {
            switch (_timer) {
                case vbl:
                #ifdef __M68000__
                    *((func_t *)0x0070) = cgg_system_vbl_interupt;
                #endif
                    break;
                case timer_c:
                #ifdef __M68000__
                    *((func_t *)0x0114) = cgg_system_timer_c_interupt;
                #endif
                    break;
            }
        });
    }
}

uint8_t cgtimer_c::base_freq() const {
    switch (_timer) {
        case vbl:
            return 50;
        case timer_c:
            return 200;
        default:
            assert(0);
            return 0;
    }
}

void cgtimer_c::add_func(func_t func, uint8_t freq) {
    if (freq == 0) {
        freq = base_freq();
    }
    with_paused_timers([this, func, freq] {
        auto functions = _timer == vbl ? cgg_vbl_functions : cgg_timer_c_functions;
        for (int i = 0; i < TIMER_FUNC_MAX_CNT; i++) {
            if (functions[i].freq == 0) {
                functions[i] = { freq, base_freq(), func };
                return;
            }
        }
        assert(0);
    });
}

void cgtimer_c::remove_func(func_t func) {
    with_paused_timers([this, func] {
        auto functions = _timer == vbl ? cgg_vbl_functions : cgg_timer_c_functions;
        int i;
        for (i = 0; i < TIMER_FUNC_MAX_CNT; i++) {
            if (functions[i].func == func) {
                break;
            }
        }
        for ( ; i < TIMER_FUNC_MAX_CNT; i++) {
            functions[i] = cgg_vbl_functions[i+1];
        }
    });
}

uint32_t cgtimer_c::tick() {
    return (_timer == vbl) ? cgg_vbl_tick : cgg_timer_c_tick;
}

void cgtimer_c::reset_tick() {
    if (_timer == vbl) {
        cgg_vbl_tick = 0;
    } else {
        cgg_timer_c_tick = 0;
    }
}

void cgtimer_c::wait(int ticks) {
    const auto wait_tick = tick() + ticks;
    while (wait_tick >= tick()) {
#ifndef __M68000__
        cgg_yield_function();
#endif
    }
}


cgmouse_c::cgmouse_c(cgrect_t limit) {
    cgg_mouse_limit = limit;
    cgg_mouse_position = (cgpoint_t){
        static_cast<int16_t>(limit.origin.x + limit.size.width / 2),
        static_cast<int16_t>(limit.origin.y + limit.size.height / 2)
    };
#ifdef __M68000__
    cgg_keyboard_vectors = Kbdvbase();
    cgg_system_mouse_interupt = cgg_keyboard_vectors->mousevec;
    cgg_keyboard_vectors->mousevec = &cgg_mouse_interupt;
#endif
}

cgmouse_c::~cgmouse_c() {
#ifdef __M68000__
    cgg_keyboard_vectors->mousevec = cgg_system_mouse_interupt;
#endif
}

void cgmouse_c::update_state() {
    for (int button = 2; --button != -1; ) {
        if (cgg_mouse_buttons & (1 << button)) {
            cgg_mouse_button_states[button] = pressed;
        } else if (cgg_prev_mouse_butons & (1 << button)) {
            cgg_mouse_button_states[button] = clicked;
        } else {
            cgg_mouse_button_states[button] = released;
        }
    }
    cgg_prev_mouse_butons = cgg_mouse_buttons;
}

bool cgmouse_c::is_pressed(button_e button) const {
    return (cgg_mouse_buttons & (1 << button)) != 0;
}

cgmouse_c::state_e cgmouse_c::get_state(button_e button) const {
    return cgg_mouse_button_states[button];
}

cgpoint_t cgmouse_c::get_postion() {
    cgpoint_t clamped_point = (cgpoint_t){
        (int16_t)(MIN(cgg_mouse_limit.origin.x + cgg_mouse_limit.size.width - 1, MAX(cgg_mouse_position.x, cgg_mouse_limit.origin.x))),
        (int16_t)(MIN(cgg_mouse_limit.origin.y + cgg_mouse_limit.size.height - 1, MAX(cgg_mouse_position.y, cgg_mouse_limit.origin.y)))
    };
    cgg_mouse_position = clamped_point;
    return clamped_point;
}
