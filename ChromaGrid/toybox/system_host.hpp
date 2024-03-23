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
        extern void cgg_vbl_interupt();
        
        // Host must call on a 200hz interval
        extern void cgg_timer_c_interupt();
        
        // Host must provide a yield function
        extern void (*cgg_yield_function)();
        
        // Host must call when mouse state changes
        extern void cgg_update_mouse(cgpoint_t position, bool left, bool right);
        
        class cgpalette_c;
        class cgimage_c;
        class cgsount_c;
        
        extern cgpalette_c *cgg_active_palette;
        extern cgsount_c *cgg_active_sound;
        
    }
    
}

#endif /* system_host_h */
