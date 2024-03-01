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


#ifdef __M68000__
#define __append_int16(p,n) __asm__ volatile ("move.w %[d],(%[a])+" : [a] "+a" (p) : [d] "g" (n) : );
#define __append_int32(p,n) __asm__ volatile ("move.l %[d],(%[a])+" : [a] "+a" (p) : [d] "g" (n) : );

struct cgcodegen_t {
    // Buffer must be 16 bytes
    static void make_trampoline(void *buffer, void *func, bool all_regs) {
        //movem.l d3-d7/a2-a6,-(sp)
        //jsr     _pSystemVBLInterupt.l
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

#endif

static int32_t cgsuper(int32_t v) {
#ifdef __M68000__
    return Super((void *)v);
#else
    return 0;
#endif
}

static int16_t cgget_screen_mode() {
#ifdef __M68000__
    return Getrez();
#else
    return 0;
#endif
}

static int16_t cgset_screen(void *log, void *phys, int16_t mode) {
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

class cgtimer_c : private cgnocopy_c {
public:
    typedef enum __packed {
        vbl
    } timer_e;
    typedef void(*func_t)(void);
    typedef void(*func_a_t)(void *);
    typedef void(*func_i_t)(int);

    cgtimer_c(timer_e timer);
    ~cgtimer_c();

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
    
    void add_func(func_t func);
    void remove_func(func_t func);
    
    uint32_t tick();
    void reset_tick();
    void wait(int ticks = 0);
    
private:
    timer_e _timer;
};

class cgmouse_c : private cgnocopy_c {
public:
    typedef enum __packed {
        right, left
    } button_e;
    
    cgmouse_c(cgrect_t limit);
    ~cgmouse_c();
    
    bool is_pressed(button_e button);
    bool was_clicked(button_e button);
    
    cgpoint_t get_postion();
    
};

#endif /* system_hpp */
