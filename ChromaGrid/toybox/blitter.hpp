//
//  blitter.hpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-25.
//

#ifndef blitter_hpp
#define blitter_hpp

#include "cincludes.hpp"
#include "types.hpp"

namespace toybox {
    
    using namespace toystd;

    typedef enum {
        hop_one = 0,
        hop_halftone = 1,
        hop_src = 2,
        hop_src_and_halftone = 3
    } hop_e;
    
    typedef enum {
        lop_zero = 0,
        lop_src = 3,
        lop_notsrc_and_dst = 4,
        lop_src_or_dst = 7,
        lop_one = 15
    } lop_e;
    
    static const uint8_t cgblitter_skew_mask = 0x0f;
    static const uint8_t cgblitter_nfsr_bit = (1<<6);
    static const uint8_t cgblitter_fxsr_bit = (1<<7);
    
    static const uint8_t cgblitter_hog_bit = (1<<6);
    static const uint8_t cgblitter_busy_bit = (1<<7);
    
    struct blitter_s {
        uint16_t halftoneRAM[16];
        int16_t srcIncX;
        int16_t srcIncY;
        uint16_t *pSrc;
        uint16_t endMask[3];
        int16_t dstIncX;
        int16_t dstIncY;
        uint16_t *pDst;
        uint16_t countX;
        uint16_t countY;
        uint8_t  HOP;
        uint8_t  LOP;
        volatile uint8_t  mode;
        uint8_t  skew;
        
        __forceinline uint8_t get_skew() const {
            return skew & cgblitter_skew_mask;
        }
        __forceinline bool is_nfsr() const {
            return (skew & cgblitter_nfsr_bit) != 0;
        }
        __forceinline bool is_fxsr() const {
            return (skew & cgblitter_fxsr_bit) != 0;
        }
        __forceinline uint16_t halftone() const {
            return halftoneRAM[mode & 0xf];
        }
#ifdef __M68000__
        inline void start(bool hog = false) {
            if (hog) {
                __asm__ volatile ("move.b #0xc0,0xffff8A3C.w \n\t"  : : : );
            } else {
                __asm__ volatile (
                                  "move.b #0x80,0xffff8A3C.w \n\t"
                                  "nop \n"
                                  ".Lrestart: bset.b #7,0xffff8A3C.w \n\t"
                                  "nop \n\t"
                                  "bne.s .Lrestart \n\t" : : : );
            }
        }
#else
        bool debug;
        void start(bool hog = false);
#endif
    };
    
#ifdef __M68000__
    static struct blitter_s *pBlitter = (struct blitter_s *)0xffff8a00;
#else
    extern struct blitter_s *pBlitter;
#endif
    
}

#endif /* blitter_hpp */
