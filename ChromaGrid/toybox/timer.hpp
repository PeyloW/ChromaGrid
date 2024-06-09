//
//  timer.hpp
//  toybox
//
//  Created by Fredrik on 2024-04-24.
//

#ifndef timer_hpp
#define timer_hpp

#include "cincludes.hpp"
#include "types.hpp"
#ifndef __M68000__
#include "host_bridge.hpp"
#endif

namespace toybox {
    
    
    class timer_c : public nocopy_c {
    public:
        typedef enum __packed {
            vbl, clock
        } timer_e;
        typedef void(*func_t)(void);
        typedef void(*func_a_t)(void *);
        typedef void(*func_i_t)(int);
        
        static timer_c &shared(timer_e timer);
        
        template<class Commands>
        __forceinline static void with_paused_timers(Commands commands) {
#ifdef __M68000__
            __asm__ volatile ("move.w #0x2700,sr" : : : );
            commands();
            __asm__ volatile ("move.w #0x2300,sr" : : : );
#else
            host_bridge_c::shared().pause_timers();
            commands();
            host_bridge_c::shared().resume_timers();
#endif
        }
        
        uint8_t base_freq() const;
        
        void add_func(func_t func, uint8_t freq = 0);
        void remove_func(func_t func);
        void add_func(func_a_t func, void *context = nullptr, uint8_t freq = 0);
        void remove_func(func_a_t func, void *context = nullptr);
        
        uint32_t tick();
        void reset_tick();
        void wait(int ticks = 0);
        
    private:
        timer_c(timer_e timer);
        ~timer_c();
        timer_e _timer;
    };
    
}

#endif /* timer_hpp */
