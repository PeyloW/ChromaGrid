//
//  system.hpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-11.
//

#ifndef system_hpp
#define system_hpp

#include "cincludes.hpp"
#include "types.hpp"


namespace toybox {
   
    using namespace toystd;
    
#define DEBUG_CPU_CLOCK_INTERUPT 0x011
#define DEBUG_CPU_MOUSE_INTERUPT 0x101
#ifdef DEBUG_CPU
    __forceinline static void debug_cpu_color(uint16_t c) {
        __asm__ volatile ("move.w %[d],0xffff8240.w" :  : [d] "g" (c) : );
    }
#else
    inline static void debug_cpu_color(uint16_t) { }
#endif

#ifdef __M68000__
#define __append_int16(p,n) __asm__ volatile ("move.w %[d],(%[a])+" : [a] "+a" (p) : [d] "g" (n) : );
#define __append_int32(p,n) __asm__ volatile ("move.l %[d],(%[a])+" : [a] "+a" (p) : [d] "g" (n) : );
    
    struct codegen_s {
        // Buffer must be 16 bytes
        static void make_trampoline(void *buffer, void *func, bool all_regs) {
            //movem.l d3-d7/a2-a6,-(sp)
            //jsr     _g_system_vbl_interupt.l
            //movem.l (sp)+,d3-d7/a2-a6
            //rts
            if (all_regs) {
                __append_int32(buffer, 0x48e7fffe);
            } else {
                __append_int32(buffer, 0x48e71f3e);
            }
            __append_int16(buffer, 0x4eb9);
            __append_int32(buffer, func);
            if (all_regs) {
                __append_int32(buffer, 0x4cdf7fff);
            } else {
                __append_int32(buffer, 0x4cdf7cf8);
            }
            __append_int16(buffer, 0x4e75);
        }
    };
    
    extern "C" void g_microwire_write(uint16_t value);
    
#endif
    
    static int32_t super(int32_t v) {
#ifdef __M68000__
        return Super((void *)v);
#else
        return 0;
#endif
    }
    
    static int16_t get_screen_mode() {
#ifdef __M68000__
        return Getrez();
#else
        return 0;
#endif
    }
        
    extern "C" {
        class palette_c;
        class image_c;
        extern const palette_c *g_active_palette;
        extern const image_c *g_active_image;
    }
    
    static int16_t set_screen(void *log, void *phys, int16_t mode) {
        int16_t rez = 0;
#ifdef __M68000__
        log = log ?: (void *)-1;
        phys = phys ?: (void *)-1;
        rez = Getrez();
        Setscreen(log, phys, mode);
        void *new_phys = Physbase();
        //hard_assert(new_phys == phys);
#endif
        return rez;
    }
    
    class timer_c : private nocopy_c {
    public:
        typedef enum __packed {
            vbl, clock
        } timer_e;
        typedef void(*func_t)(void);
        typedef void(*func_a_t)(void *);
        typedef void(*func_i_t)(int);
        
        timer_c(timer_e timer);
        ~timer_c();
        
        template<class Commands>
        __forceinline static void with_paused_timers(Commands commands) {
#ifdef __M68000__
            __asm__ volatile ("move.w #0x2700,sr" : : : );
            commands();
            __asm__ volatile ("move.w #0x2300,sr" : : : );
#else
            static bool is_paused = false;
            assert(is_paused == false);
            is_paused = true;
            commands();
            is_paused = false;
#endif
        }
        
        uint8_t base_freq() const;
        
        void add_func(func_a_t func, void *context = nullptr, uint8_t freq = 0);
        void remove_func(func_a_t func, void *context = nullptr);
        
        uint32_t tick();
        void reset_tick();
        void wait(int ticks = 0);
        
    private:
        timer_e _timer;
    };
    
    class mouse_c : private nocopy_c {
    public:
        typedef enum __packed {
            right, left
        } button_e;
        typedef enum __packed {
            released,
            pressed,
            clicked
        } state_e;
        
        mouse_c(rect_s limit);
        ~mouse_c();
        
        void update_state();  // Call once per vbl
        
        bool is_pressed(button_e button) const;
        state_e get_state(button_e button) const;
        
        point_s get_postion();
        
    };
    
}
#endif /* system_hpp */
