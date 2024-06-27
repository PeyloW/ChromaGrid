//
//  timer.cpp
//  toybox
//
//  Created by Fredrik on 2024-04-24.
//

#include "timer.hpp"
#include "machine.hpp"

#if !TOYBOX_TARGET_ATARI
#   error "For Atari target only"
#endif

using namespace toybox;

extern "C" {
#ifdef __M68000__
    extern timer_c::func_t g_system_vbl_interupt;
    extern void g_vbl_interupt();
    extern int16_t g_system_vbl_freq;
    extern timer_c::func_t g_system_clock_interupt;
    extern void g_clock_interupt();
#endif
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
