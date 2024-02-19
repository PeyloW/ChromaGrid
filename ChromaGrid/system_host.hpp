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

extern "C" {

// Host must call on a 50hz interval
extern void pVBLInterupt();

// Host must provide a yield function
extern void (*pYieldFunction)();
    
// Host must call when mouse state changes
extern void pUpdateMouse(cgpoint_t position, bool left, bool right);

class cgpalette_c;
class cgimage_c;

extern cgpalette_c *pActivePalette;
extern cgimage_c *pActiveImage;

}

#endif /* system_host_h */
