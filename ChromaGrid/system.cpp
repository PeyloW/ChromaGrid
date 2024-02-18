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

static bool enabled_timers[1] = { false };

cgtimer_t::func_t pVBLFunc = NULL;
uint32_t pVBLTick = 0;
static cgrect_t pMosueLimit;
static bool pLastButtonState[2];
uint8_t pMouseButtons;
cgpoint_t pMousePosition;

#ifdef __M68000__
    extern cgtimer_t::func_t pSystemVBLInterupt;
    extern void pVBLInterupt();
#else
    void (*pYieldFunction)() = nullptr;

    void pVBLInterupt() {
        if (enabled_timers[0]) {
            pVBLTick++;
            if (pVBLFunc) {
                pVBLFunc();
            }
        }
    }

    // Host must call when mouse state changes
    void pUpdateMouse(cgpoint_t position, bool left, bool right) {
        pMousePosition = position;
        pMouseButtons = (left ? 1 : 0) | (right ? 2 : 0);
    }
#endif
}


cgtimer_t::cgtimer_t(timer_t timer, func_t func) : timer(timer) {
    assert(timer == vbl);
    assert(enabled_timers[timer] == false);
    enabled_timers[timer] = true;
    pVBLFunc = func;
#ifdef __M68000__
    pSystemVBLInterupt = *((func_t *)0x0070);
    *((func_t *)0x0070) = &pVBLInterupt;
#endif
}

cgtimer_t::~cgtimer_t() {
#ifdef __M68000__
    *((func_t *)0x0070) = pSystemVBLInterupt;
#endif
    enabled_timers[timer] = false;
    pVBLFunc = NULL;
}

uint32_t cgtimer_t::tick() {
    return pVBLTick;
}

void cgtimer_t::wait() {
    const auto old_tick = tick();
    while (old_tick == tick()) {
#ifndef __M68000__
        pYieldFunction();
#endif
    }
}


cgmouse_t::cgmouse_t(cgrect_t limit) {
    pMosueLimit = limit;
    pMousePosition = (cgpoint_t){
        static_cast<int16_t>(limit.origin.x + limit.size.width / 2),
        static_cast<int16_t>(limit.origin.y + limit.size.height / 2)
    };
}

cgmouse_t::~cgmouse_t() {
}

bool cgmouse_t::is_pressed(button_t button) {
    return (pMouseButtons & (1 << button)) != 0;
}

bool cgmouse_t::was_clicked(button_t button) {
    bool pressed = is_pressed(button);
    bool clicked = pLastButtonState[button] != pressed && pressed == false;
    pLastButtonState[button] = pressed;
    return clicked;
}

cgpoint_t cgmouse_t::get_postion() {
    cgpoint_t clamped_point = (cgpoint_t){
        static_cast<int16_t>(MIN(pMosueLimit.origin.x + pMosueLimit.size.width - 1, MAX(pMousePosition.x, pMosueLimit.origin.x))),
        static_cast<int16_t>(MIN(pMosueLimit.origin.y + pMosueLimit.size.height - 1, MAX(pMousePosition.y, pMosueLimit.origin.y)))
    };
    return clamped_point;
}
