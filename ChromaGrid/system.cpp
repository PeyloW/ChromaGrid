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

static int pTimerRefCount[1] = { 0 };

#define VBL_FUNC_MAX_CNT 4
cgtimer_c::func_t pVBLFuncs[VBL_FUNC_MAX_CNT+1] = { nullptr };
volatile uint32_t pVBLTick = 0;
static cgrect_t pMosueLimit;
static bool pLastButtonState[2];
uint8_t pMouseButtons;
cgpoint_t pMousePosition;

#ifdef __M68000__
    extern cgtimer_c::func_t pSystemVBLInterupt;
    extern void pVBLInterupt();
    extern cgtimer_c::func_a_t pSystemMouseInterupt;
    extern void pMouseInterupt(void *);
    static _KBDVECS *pKeyboardVectors = nullptr;
#else
    void (*pYieldFunction)() = nullptr;

    void pVBLInterupt() {
        if (pTimerRefCount[cgtimer_c::vbl] > 0) {
            pVBLTick++;
            for (int i = 0; i < 8; i++) {
                if (pVBLFuncs[i]) {
                    pVBLFuncs[i]();
                } else {
                    break;
                }
            }
        }
    }

    // Host must call when mouse state changes
    void pUpdateMouse(cgpoint_t position, bool left, bool right) {
        pMousePosition = position;
        pMouseButtons = (left ? 2 : 0) | (right ? 1 : 0);
    }
#endif
}


cgtimer_c::cgtimer_c(timer_t timer) : _timer(timer) {
    assert(timer == vbl);
    pTimerRefCount[timer]++;
    if (pTimerRefCount[0] == 1) {
        with_paused_timers([] {
#ifdef __M68000__
        pSystemVBLInterupt = *((func_t *)0x0070);
        *((func_t *)0x0070) = &pVBLInterupt;
#endif
        });
    }
}

cgtimer_c::~cgtimer_c() {
    pTimerRefCount[cgtimer_c::vbl]--;
    assert(pTimerRefCount[cgtimer_c::vbl] >= 0);
    if (pTimerRefCount[cgtimer_c::vbl] == 0) {
        with_paused_timers([] {
#ifdef __M68000__
            *((func_t *)0x0070) = pSystemVBLInterupt;
#endif
        });
    }
}

void cgtimer_c::add_func(func_t func) {
    with_paused_timers([func] {
        for (int i = 0; i < VBL_FUNC_MAX_CNT; i++) {
            if (pVBLFuncs[i] == nullptr) {
                pVBLFuncs[i] = func;
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
            if (pVBLFuncs[i] == func) {
                pVBLFuncs[i] = func;
                break;
            }
        }
        for ( ; i < VBL_FUNC_MAX_CNT; i++) {
            pVBLFuncs[i] = pVBLFuncs[i+1];
        }
    });
}

uint32_t cgtimer_c::tick() {
    return pVBLTick;
}

void cgtimer_c::wait() {
    const auto old_tick = tick();
    while (old_tick == tick()) {
#ifndef __M68000__
        pYieldFunction();
#endif
    }
}


cgmouse_c::cgmouse_c(cgrect_t limit) {
    pMosueLimit = limit;
    pMousePosition = (cgpoint_t){
        static_cast<int16_t>(limit.origin.x + limit.size.width / 2),
        static_cast<int16_t>(limit.origin.y + limit.size.height / 2)
    };
#ifdef __M68000__
    pKeyboardVectors = Kbdvbase();
    pSystemMouseInterupt = pKeyboardVectors->mousevec;
    pKeyboardVectors->mousevec = &pMouseInterupt;
#endif
}

cgmouse_c::~cgmouse_c() {
#ifdef __M68000__
    pKeyboardVectors->mousevec = pSystemMouseInterupt;
#endif
}

bool cgmouse_c::is_pressed(button_t button) {
    return (pMouseButtons & (1 << button)) != 0;
}

bool cgmouse_c::was_clicked(button_t button) {
    bool pressed = is_pressed(button);
    bool clicked = pLastButtonState[button] != pressed && pressed == false;
    pLastButtonState[button] = pressed;
    return clicked;
}

cgpoint_t cgmouse_c::get_postion() {
    cgpoint_t clamped_point = (cgpoint_t){
        static_cast<int16_t>(MIN(pMosueLimit.origin.x + pMosueLimit.size.width - 1, MAX(pMousePosition.x, pMosueLimit.origin.x))),
        static_cast<int16_t>(MIN(pMosueLimit.origin.y + pMosueLimit.size.height - 1, MAX(pMousePosition.y, pMosueLimit.origin.y)))
    };
    pMousePosition = clamped_point;
    return clamped_point;
}
