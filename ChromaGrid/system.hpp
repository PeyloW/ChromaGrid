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

// Buffer must be 16 bytes
static void generate_safe_trampoline(void *buffer, void *func, bool all_regs) {
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

#endif


static int32_t exec_super(int32_t(*func)(void)) {
#ifdef __M68000__
    return Supexec(func);
#else
    return func();
#endif
}

static int16_t get_screen_mode() {
#ifdef __M68000__
    return Getrez();
#else
    return 0;
#endif
}

static void set_screen(void *log, void *phys, int16_t mode) {
#ifdef __M68000__
    log = log ?: (void *)-1;
    phys = phys ?: (void *)-1;
    Setscreen(log, phys, mode);
    void *new_phys = Physbase();
    //hard_assert(new_phys == phys);
#endif
}

class cgtimer_t {
public:
    enum timer_t {
        vbl
    };
    typedef void(*func_t)(void);
    typedef void(*func_a_t)(void *);
    typedef void(*func_i_t)(int);

    cgtimer_t(timer_t timer);
    ~cgtimer_t();

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
    void wait();
    
private:
    timer_t timer;
};

class cgmouse_t {
public:
    enum button_t {
        right, left
    };
    
    cgmouse_t(cgrect_t limit);
    ~cgmouse_t();
    
    bool is_pressed(button_t button);
    bool was_clicked(button_t button);
    
    cgpoint_t get_postion();
    
};

#endif /* system_hpp */
