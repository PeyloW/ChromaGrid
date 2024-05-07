//
//  system_helpers.hpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-11.
//

#ifndef system_helpers_hpp
#define system_helpers_hpp

#include "cincludes.hpp"
#include "types.hpp"


namespace toybox {
       
#define DEBUG_CPU_CLOCK_INTERUPT 0x011
#define DEBUG_CPU_MOUSE_INTERUPT 0x101
#if TOYBOX_DEBUG_CPU
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
        
#endif
   
}
#endif /* system_helpers_hpp */
