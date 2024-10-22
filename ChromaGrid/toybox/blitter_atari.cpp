//
//  blitter.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-25.
//

#include "blitter_atari.hpp"

#if !TOYBOX_TARGET_ATARI
#   error "For Atari target only"
#endif

using namespace toybox;

#ifndef __M68000__

#define DEBUG_BLITTER 0

static struct blitter_s _Blitter;
struct blitter_s *toybox::pBlitter = &_Blitter;

//
// Emulate blitter on host machine.
// Details borrowed from Hatari 1.0 sources (https://github.com/hatari/hatari)
//
void blitter_s::start(bool hog) {
#if DEBUG_BLITTER
    printf("BLIT: %d x %d words.\n\r", this->countX, this->countY);
#endif
    
    uint32_t buffer = 0;
    const auto do_shift = [this, &buffer] {
        buffer <<= 16;
    };
    const auto read_src = [this, &buffer] {
        buffer |= *pSrc;
    };
    const auto inc_src = [this, &buffer] (const bool is_last) {
        pSrc += (is_last ? srcIncY : srcIncX) / 2;
    };
    const auto write_dst = [this, &buffer] (const uint16_t mask) {
        uint16_t src;
        switch (HOP) {
            case hop_e::one: src = 0xffff; break;
            case hop_e::halftone: src = halftone(); break;
            case hop_e::src: src = buffer >> get_skew(); break;
            case hop_e::src_and_halftone: src = (buffer >> get_skew()) & halftone(); break;
            default: assert(0); break;
        }
        const uint16_t dst = *pDst;
        uint16_t opd;
        switch (LOP) {
            case lop_e::zero: opd = 0; break;
            case lop_e::one: opd = 0xffff; break;
            case lop_e::src: opd = src; break;
            case lop_e::src_or_dst: opd = src | dst; break;
            case lop_e::notsrc_and_dst: opd = ~src & dst; break;
            default: assert(0); opd = 0; break;
        }
        if (mask == 0xffff) {
            *pDst = opd;
        } else {
            *pDst = (opd & mask) | (dst & ~mask);
        }
    };
    const auto inc_dst = [this] (const bool is_last) {
        pDst += (is_last ? dstIncY : dstIncX) / 2;
    };
    do {
        // Firs word
        do_shift();
        if (is_fxsr()) {
            // Handle first extra, including no reading outside buffer
            read_src();
            inc_src(false);
            do_shift();
            if (countX > 1 || countY > 1) {
                // This is not how the blitter work, but unless skipepd we will read outside buffer
                read_src();
            }
        } else {
            read_src();
        }
        write_dst(endMask[0]);
        
        // Middle words if countX > 2
        for(int x = 2 ; x < countX; x++) {
            inc_src(false);
            inc_dst(false);
            do_shift();
            read_src();
            write_dst(endMask[1]);
        }

        // Last word of countX >= 2
        if (countX >= 2) {
            inc_dst(false);
            do_shift();
            if ((!is_nfsr()) || ((~(0xffff>>skew)) > endMask[2])) {
                // Only inc if not no final read, or skew leaved unmasked bits
                inc_src(false);
                read_src();
            }
            write_dst(endMask[2]);
        }
        
        // Next line
        inc_src(true);
        inc_dst(true);
        mode = (mode + 1) & 0xf;
    } while (--countY > 0);
}

#endif
