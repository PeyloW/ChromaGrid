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

int32_t exec_super(int32_t(*func)(void));


class cgtimer_t {
public:
    enum timer_t {
        vbl
    };
    typedef void(*func_t)(void);
    
    cgtimer_t(timer_t timer, func_t func);
    ~cgtimer_t();
    
    uint32_t tick();
    void wait();
    
private:
    timer_t timer;
};

class cgmouse_t {
public:
    enum button_t {
        left, right
    };
    
    cgmouse_t(cgrect_t limit);
    ~cgmouse_t();
    
    bool is_pressed(button_t button);
    bool was_clicked(button_t button);
    
    cgpoint_t get_postion();
    
};

#endif /* system_hpp */
