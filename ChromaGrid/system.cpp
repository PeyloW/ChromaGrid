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

static int cgg_timer_ref_counts[1] = { 0 };

#define VBL_FUNC_MAX_CNT 4
cgtimer_c::func_t cgg_vbl_functions[VBL_FUNC_MAX_CNT+1] = { nullptr };
volatile uint32_t cgg_vbl_tick = 0;
static cgrect_t cgg_mouse_limit;
static bool cgg_prev_mouse_buton_state[2];
uint8_t cgg_mouse_buttons;
cgpoint_t cgg_mouse_position;

#ifdef __M68000__
    extern cgtimer_c::func_t cgg_system_vbl_interupt;
    extern void cgg_vbl_interupt();
    extern cgtimer_c::func_a_t cgg_system_mouse_interupt;
    extern void cgg_mouse_interupt(void *);
    static _KBDVECS *cgg_keyboard_vectors = nullptr;
#else
    void (*cgg_yield_function)() = nullptr;

    void cgg_vbl_interupt() {
        if (cgg_timer_ref_counts[cgtimer_c::vbl] > 0) {
            cgg_vbl_tick++;
            for (int i = 0; i < 8; i++) {
                if (cgg_vbl_functions[i]) {
                    cgg_vbl_functions[i]();
                } else {
                    break;
                }
            }
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
    assert(timer == vbl);
    cgg_timer_ref_counts[timer]++;
    if (cgg_timer_ref_counts[0] == 1) {
        with_paused_timers([] {
#ifdef __M68000__
        cgg_system_vbl_interupt = *((func_t *)0x0070);
        *((func_t *)0x0070) = &cgg_vbl_interupt;
#endif
        });
    }
}

cgtimer_c::~cgtimer_c() {
    cgg_timer_ref_counts[cgtimer_c::vbl]--;
    assert(cgg_timer_ref_counts[cgtimer_c::vbl] >= 0);
    if (cgg_timer_ref_counts[cgtimer_c::vbl] == 0) {
        with_paused_timers([] {
#ifdef __M68000__
            *((func_t *)0x0070) = cgg_system_vbl_interupt;
#endif
        });
    }
}

void cgtimer_c::add_func(func_t func) {
    with_paused_timers([func] {
        for (int i = 0; i < VBL_FUNC_MAX_CNT; i++) {
            if (cgg_vbl_functions[i] == nullptr) {
                cgg_vbl_functions[i] = func;
                return;
            }
        }
        assert(0);
    });
}

void cgtimer_c::remove_func(func_t func) {
    with_paused_timers([func] {
        int i;
        for (i = 0; i < VBL_FUNC_MAX_CNT; i++) {
            if (cgg_vbl_functions[i] == func) {
                cgg_vbl_functions[i] = func;
                break;
            }
        }
        for ( ; i < VBL_FUNC_MAX_CNT; i++) {
            cgg_vbl_functions[i] = cgg_vbl_functions[i+1];
        }
    });
}

uint32_t cgtimer_c::tick() {
    return cgg_vbl_tick;
}

void cgtimer_c::reset_tick() {
    cgg_vbl_tick = 0;
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

bool cgmouse_c::is_pressed(button_e button) {
    return (cgg_mouse_buttons & (1 << button)) != 0;
}

bool cgmouse_c::was_clicked(button_e button) {
    bool pressed = is_pressed(button);
    bool clicked = cgg_prev_mouse_buton_state[button] != pressed && pressed == false;
    cgg_prev_mouse_buton_state[button] = pressed;
    return clicked;
}

cgpoint_t cgmouse_c::get_postion() {
    cgpoint_t clamped_point = (cgpoint_t){
        (int16_t)(MIN(cgg_mouse_limit.origin.x + cgg_mouse_limit.size.width - 1, MAX(cgg_mouse_position.x, cgg_mouse_limit.origin.x))),
        (int16_t)(MIN(cgg_mouse_limit.origin.y + cgg_mouse_limit.size.height - 1, MAX(cgg_mouse_position.y, cgg_mouse_limit.origin.y)))
    };
    cgg_mouse_position = clamped_point;
    return clamped_point;
}
