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
