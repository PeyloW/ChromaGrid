//
//  system_host.hpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-11.
//

#ifdef __M68000__
#error "For host mchine only"
#endif

#ifndef system_host_h
#define system_host_h

#include "system.hpp"

namespace toybox {
    
    using namespace toystd;
    
    
    extern "C" {
        
        // Host must call on a 50hz interval
        extern void g_vbl_interupt();
        
        // Host must call on a 200hz interval
        extern void g_clock_interupt();
        
        // Host must provide a yield function
        extern void (*g_yield_function)();
        
        // Host must call when mouse state changes
        extern void g_update_mouse(point_s position, bool left, bool right);
        
        class palette_c;
        class image_c;
        class sound_c;
        
        extern const palette_c *g_active_palette;
        extern sound_c *g_active_sound;
        
    }
    
}

#endif /* system_host_h */
