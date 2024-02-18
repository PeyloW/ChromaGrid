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
#endif
}

class cgtimer_t {
public:
    enum timer_t {
        vbl
    };
    typedef void(*func_t)(void);
    typedef void(*func_a_t)(void *);

    cgtimer_t(timer_t timer, func_t func) asm("_cgtimer_t_init");
    ~cgtimer_t() asm("_cgtimer_t_deinit");
    
    uint32_t tick() asm("_cgtimer_t_tick");
    void wait() asm("_cgtimer_t_wait");
    
private:
    timer_t timer;
};

class cgmouse_t {
public:
    enum button_t {
        left, right
    };
    
    cgmouse_t(cgrect_t limit) asm("_cgmouse_t_init");
    ~cgmouse_t() asm("_cgmouse_t_deinit");
    
    bool is_pressed(button_t button) asm("_cgmouse_t_is_pressed");
    bool was_clicked(button_t button) asm("_cgmouse_t_was_clicked");
    
    cgpoint_t get_postion();
    
};

#endif /* system_hpp */
