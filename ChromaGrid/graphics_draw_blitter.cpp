//
//  graphics_draw_blitter.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-24.
//

#include "graphics.hpp"

enum {
    blitter_hop_one = 0,
    blitter_hop_src = 2
} cgblitter_hop_t;

enum {
    cgblitter_lop_zero = 0,
    cgblitter_lop_src_and_dts = 1,
    cgblitter_lop_src = 3,
    cgblitter_lop_notsrc_and_dst = 4,
    cgblitter_lop_src_or_dst = 7,
    cgblitter_lop_ = 15
} cgblitter_lop_t;

struct cgblitter_t {
    uint16_t halftoneRAM[16];
    uint16_t srcIncX;
    uint16_t srcIncY;
    uint16_t *pSrc;
    uint16_t endMask[3];
    uint16_t dstIncX;
    uint16_t dstIncY;
    uint16_t *pDst;
    uint16_t countX;
    uint16_t countY;
    uint8_t  HOP;
    uint8_t  LOP;
    volatile uint8_t  mode;
    uint8_t  skew;
};

static const uint8_t cgblitter_skew_mask = 0x0f;
static const uint8_t cgblitter_nfsr_bit = (1<<6);
static const uint8_t cgblitter_fxsr_bit = (1<<7);

static const uint8_t cgblitter_hog_bit = (1<<6);
static const uint8_t cgblitter_busy_bit = (1<<7);

static const uint16_t pBlitter_mask[16] = {
    0xffff, 0x7fff, 0x3fff, 0x1fff,
    0x0fff, 0x07ff, 0x03ff, 0x01ff,
    0x00ff, 0x007f, 0x003f, 0x001f,
    0x000f, 0x0007, 0x0003, 0x0001
};

static const uint8_t pBlitter_skewflags[8] = {
    cgblitter_nfsr_bit,
    cgblitter_fxsr_bit,
    0,
    cgblitter_nfsr_bit|cgblitter_fxsr_bit,
    0,
    cgblitter_fxsr_bit,
    0,
    cgblitter_fxsr_bit
};

#ifdef __M68000__
static struct cgblitter_t *pBlitter = (struct cgblitter_t *)0xffff8a00;
#else
static struct cgblitter_t _Blitter;
static struct cgblitter_t *pBlitter = &_Blitter;
#endif

inline static void blitter_start(struct cgblitter_t *blitter) {
#ifdef __M68000__
    blitter->mode = cgblitter_busy_bit | cgblitter_hog_bit;
#else
    // TODO: Implement this??
#endif
}

inline static void blitter_wait(struct cgblitter_t *blitter) {
#ifdef __M68000__
    while(blitter->mode & cgblitter_busy_bit) {
        __asm__ volatile ("nop" : : : );
    };
#endif
}

void cgimage_c::imp_draw_aligned(const cgimage_c &srcImage, cgpoint_t at) const {
    assert(get_size().contains(at));
    assert((at.x & 0xf) == 0);
    assert((srcImage.get_size().width & 0xf) == 0);
    
    auto blitter = pBlitter;
    const uint16_t copy_words = (srcImage._size.width / 16);

    // Source
    blitter->srcIncX  = 2;
    blitter->srcIncY = (uint16_t)(srcImage._line_words - copy_words) * 8 + 2;
    blitter->pSrc = srcImage._bitmap;

    // Mask
    blitter->endMask[0] =  0xFFFF;
    blitter->endMask[1] =  0xFFFF;
    blitter->endMask[2] =  0xFFFF;

    // Dest
    blitter->dstIncX  = 2;
    blitter->dstIncY = (uint16_t)(_line_words - copy_words) * 8 + 2;
    uint16_t dst_word_offset = at.y * (_line_words * 4) + (at.x / 16) * 4;
    blitter->pDst = _bitmap + dst_word_offset;

    // Counts
    blitter->countX  = (uint16_t)(copy_words) * 4;
    blitter->countY = srcImage._size.height;

    // Operation flags
    blitter->HOP = blitter_hop_src;
    blitter->LOP = cgblitter_lop_src;
    blitter->skew = 0;

    blitter_start(blitter);
    blitter_wait(blitter);
#ifndef __M68000__
    cgrect_t rect = (cgrect_t){ {0, 0}, srcImage._size };
    imp_draw_rect(srcImage, &rect, at);
#endif
}

void cgimage_c::imp_draw_rect(const cgimage_c &srcImage, cgrect_t *const rect, cgpoint_t point) const {
    assert(get_size().contains(point));
    for (int y = 0; y < rect->size.height; y++) {
        for (int x = 0; x < rect->size.width; x++) {
            uint8_t color = srcImage.get_pixel(cgpoint_t{(int16_t)(rect->origin.x + x), (int16_t)(rect->origin.y + y)});
            if (color != MASKED_CIDX) {
                put_pixel(color, cgpoint_t{(int16_t)(point.x + x), (int16_t)(point.y + y)});
            }
        }
    }
}

