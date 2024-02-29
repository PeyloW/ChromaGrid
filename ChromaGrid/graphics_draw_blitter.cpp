//
//  graphics_draw_blitter.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-24.
//

#include "graphics.hpp"
#include "blitter.hpp"

static const uint8_t pBlitter_skewflags[4] = {
    cgblitter_nfsr_bit,
    cgblitter_fxsr_bit,
    0,
    cgblitter_nfsr_bit|cgblitter_fxsr_bit,
};

static const uint16_t pBlitter_mask[17] = {
    0xffff, 0x7fff, 0x3fff, 0x1fff,
    0x0fff, 0x07ff, 0x03ff, 0x01ff,
    0x00ff, 0x007f, 0x003f, 0x001f,
    0x000f, 0x0007, 0x0003, 0x0001,
    0x0000
};

static const cgimage_c::stencil_t *pActiveStencil = nullptr;

__forceinline static void set_active_stencil(struct cgblitter_t *blitter, const cgimage_c::stencil_t *const stencil) {
    if (pActiveStencil != stencil) {
        memcpy(blitter->halftoneRAM, stencil, 32);
        pActiveStencil = stencil;
    }
}

void cgimage_c::imp_draw_aligned(const cgimage_c &srcImage, const cgrect_t &rect, cgpoint_t at) const {
    assert(get_size().contains(at));
    assert((rect.origin.x & 0xf) == 0);
    assert((rect.size.width & 0xf) == 0);
    assert((at.x & 0xf) == 0);
    assert((srcImage.get_size().width & 0xf) == 0);
        
    auto blitter = pBlitter;
    const uint16_t copy_words = (rect.size.width / 16);

    // Source
    blitter->srcIncX  = 2;
    blitter->srcIncY = (uint16_t)(srcImage._line_words - copy_words) * 8 + 2;
    const uint16_t src_word_offset = (rect.origin.y * srcImage._line_words) + (rect.origin.x / 16);
    blitter->pSrc = srcImage._bitmap + src_word_offset * 4;

    // Dest
    blitter->dstIncX  = 2;
    blitter->dstIncY = (uint16_t)(_line_words - copy_words) * 8 + 2;
    const uint16_t dst_word_offset = at.y * (_line_words * 4) + (at.x / 16) * 4;
    blitter->pDst = _bitmap + dst_word_offset;

    // Mask
    blitter->endMask[0] =  0xFFFF;
    blitter->endMask[1] =  0xFFFF;
    blitter->endMask[2] =  0xFFFF;

    // Counts
    blitter->countX  = (uint16_t)(copy_words) * 4;
    blitter->countY = rect.size.height;
    blitter->skew = 0;

    // Operation flags
    if (_stencil) {
        set_active_stencil(blitter, _stencil);
        blitter->HOP = cgblitter_hop_halftone;
        blitter->LOP = cgblitter_lop_notsrc_and_dst;
        blitter->mode = at.y & 0xf;
        blitter->start();

        blitter->pSrc = srcImage._bitmap + src_word_offset * 4;
        blitter->pDst = _bitmap + dst_word_offset;
        blitter->countY = rect.size.height;

        blitter->HOP = cgblitter_hop_src_and_halftone;
        blitter->LOP = cgblitter_lop_src_or_dst;
    } else {
        blitter->HOP = cgblitter_hop_src;
        blitter->LOP = cgblitter_lop_src;
    }
    
    blitter->start();
}

void cgimage_c::imp_draw(const cgimage_c &srcImage, const cgrect_t &rect, cgpoint_t at) const {
    assert(get_size().contains(at));
    assert(srcImage.get_size().contains(rect.origin));
    auto blitter = pBlitter;

    const uint16_t src_max_x    = (uint16_t)(rect.origin.x + rect.size.width - 1);
    const uint16_t dst_max_x    = (uint16_t)(at.x + rect.size.width - 1);
    const uint16_t src_words_dec_1  = (uint16_t)((src_max_x / 16) - (rect.origin.x / 16));
    const uint16_t dst_words_dec_1  = (uint16_t)((dst_max_x / 16) - (at.x / 16));
    
    // Source
    blitter->srcIncX = 8;
    blitter->srcIncY = (uint16_t)((srcImage._line_words - src_words_dec_1) * 8);
    const uint16_t src_word_offset = (rect.origin.y * srcImage._line_words) + (rect.origin.x / 16);
    uint16_t *src_bitmap  = srcImage._bitmap + src_word_offset * 4;
    
    // Dest
    blitter->dstIncX  = 8;
    blitter->dstIncY = (uint16_t)((_line_words - dst_words_dec_1) * 8);
    const uint16_t dst_word_offset = (at.y * _line_words) + (at.x / 16);
    uint16_t *dts_bitmap  = _bitmap + dst_word_offset * 4;

    // Mask
    uint16_t end_mask_0 = pBlitter_mask[at.x & 15];
    uint16_t end_mask_2 = ~pBlitter_mask[(dst_max_x & 15) + 1];
    uint8_t skew = (uint8_t)(((at.x & 15) - (rect.origin.x & 15)) & 15);
    if (dst_words_dec_1 == 0) {
        end_mask_0 &= end_mask_2;
        end_mask_2 = end_mask_0;
        if (src_words_dec_1 != 0) {
            skew |= cgblitter_fxsr_bit;
        } else if ((rect.origin.x & 15) > (at.x & 15)) {
            skew |= cgblitter_fxsr_bit;
            blitter->srcIncY -= 8;
        }
    } else {
        int idx = 0;
        if ((rect.origin.x & 15) > (at.x & 15)) {
            idx |= 1;
        }
        if (src_words_dec_1 == dst_words_dec_1) {
            idx |= 2;
        }
        skew |= pBlitter_skewflags[idx];
    }
    blitter->endMask[0] = end_mask_0;
    blitter->endMask[1] = 0xFFFF;
    blitter->endMask[2] = end_mask_2;

    // Counts
    blitter->countX  = (uint16_t)(dst_words_dec_1 + 1);
    
    // Operation flags
    blitter->HOP = cgblitter_hop_src;
    blitter->LOP = cgblitter_lop_src;
    blitter->skew = skew;

    // Move 4 planes
    for (int i = 4; --i != -1; ) {
        blitter->pDst   = dts_bitmap;
        blitter->pSrc   = src_bitmap;
        blitter->countY = rect.size.height;

        blitter->start();

        src_bitmap++;
        dts_bitmap++;
    }
}

void cgimage_c::imp_draw_masked(const cgimage_c &srcImage, const cgrect_t &rect, cgpoint_t at) const {
    assert(get_size().contains(at));
    assert(srcImage.get_size().contains(rect.origin));
    auto blitter = pBlitter;

    const uint16_t src_max_x    = (uint16_t)(rect.origin.x + rect.size.width - 1);
    const uint16_t dst_max_x    = (uint16_t)(at.x + rect.size.width - 1);
    const uint16_t src_words_dec_1  = (uint16_t)((src_max_x / 16) - (rect.origin.x / 16));
    const uint16_t dst_words_dec_1  = (uint16_t)((dst_max_x / 16) - (at.x / 16));
    
    // Source
    blitter->srcIncX = 2;
    blitter->srcIncY = (uint16_t)((srcImage._line_words - src_words_dec_1) * 2);
    const uint16_t src_word_offset = (rect.origin.y * srcImage._line_words) + (rect.origin.x / 16);
    uint16_t *src_maskmap  = srcImage._maskmap + src_word_offset;
    
    // Dest
    blitter->dstIncX  = 8;
    blitter->dstIncY = (uint16_t)(_line_words * 8 - (dst_words_dec_1 * 8));
    const uint16_t dst_word_offset = (at.y * _line_words) + (at.x / 16);
    uint16_t *dts_bitmap  = _bitmap + dst_word_offset * 4;

    // Mask
    uint16_t end_mask_0 = pBlitter_mask[at.x & 15];
    uint16_t end_mask_2 = ~pBlitter_mask[(dst_max_x & 15) + 1];
    uint8_t skew = (uint8_t)(((at.x & 15) - (rect.origin.x & 15)) & 15);
    if (dst_words_dec_1 == 0) {
        end_mask_0 &= end_mask_2;
        end_mask_2 = end_mask_0;
        if (src_words_dec_1 != 0) {
            skew |= cgblitter_fxsr_bit;
        } else if ((rect.origin.x & 15) > (at.x & 15)) {
            skew |= cgblitter_fxsr_bit;
            blitter->srcIncY -= 2;
        }
    } else {
        int idx = 0;
        if ((rect.origin.x & 15) > (at.x & 15)) {
            idx |= 1;
        }
        if (src_words_dec_1 == dst_words_dec_1) {
            idx |= 2;
        }
        skew |= pBlitter_skewflags[idx];
    }
    blitter->endMask[0] = end_mask_0;
    blitter->endMask[1] = 0xFFFF;
    blitter->endMask[2] = end_mask_2;

    // Counts
    blitter->countX  = (uint16_t)(dst_words_dec_1 + 1);
    
    // Operation flags
    blitter->HOP = cgblitter_hop_src;
    blitter->LOP = cgblitter_lop_notsrc_and_dst;
    blitter->skew = skew;

    // Mask 4 planes
    for (int i = 4; --i != -1; ) {
        blitter->pDst   = dts_bitmap;
        blitter->pSrc   = src_maskmap;
        blitter->countY = rect.size.height;

        blitter->start();

        dts_bitmap++;
    }
    
    // Update source
    blitter->srcIncX *= 4;
    blitter->srcIncY *= 4;
    uint16_t *src_bitmap = srcImage._bitmap + src_word_offset * 4;
    
    // Update dest
    dts_bitmap -= 4;
    
    // Update operation flags
    blitter->LOP = cgblitter_lop_src_or_dst;

    // Draw 4 planes
    for (int i = 4; --i != -1; ) {
        blitter->pDst   = dts_bitmap;
        blitter->pSrc   = src_bitmap;
        blitter->countY = rect.size.height;

        blitter->start();

        src_bitmap++;
        dts_bitmap++;
    }
}

void cgimage_c::imp_draw_color(const cgimage_c &srcImage, const cgrect_t &rect, cgpoint_t at, uint16_t color) const {
    assert(get_size().contains(at));
    assert(srcImage.get_size().contains(rect.origin));
    auto blitter = pBlitter;

    const uint16_t src_max_x    = (uint16_t)(rect.origin.x + rect.size.width - 1);
    const uint16_t dst_max_x    = (uint16_t)(at.x + rect.size.width - 1);
    const uint16_t src_words_dec_1  = (uint16_t)((src_max_x / 16) - (rect.origin.x / 16));
    const uint16_t dst_words_dec_1  = (uint16_t)((dst_max_x / 16) - (at.x / 16));
    
    // Source
    blitter->srcIncX = 2;
    blitter->srcIncY = (uint16_t)((srcImage._line_words - src_words_dec_1) * 2);
    const uint16_t src_word_offset = (rect.origin.y * srcImage._line_words) + (rect.origin.x / 16);
    uint16_t *src_maskmap  = srcImage._maskmap + src_word_offset;
    
    // Dest
    blitter->dstIncX  = 8;
    blitter->dstIncY = (uint16_t)(_line_words * 8 - (dst_words_dec_1 * 8));
    const uint16_t dst_word_offset = (at.y * _line_words) + (at.x / 16);
    uint16_t *dts_bitmap  = _bitmap + dst_word_offset * 4;

    // Mask
    uint16_t end_mask_0 = pBlitter_mask[at.x & 15];
    uint16_t end_mask_2 = ~pBlitter_mask[(dst_max_x & 15) + 1];
    uint8_t skew = (uint8_t)(((at.x & 15) - (rect.origin.x & 15)) & 15);
    if (dst_words_dec_1 == 0) {
        end_mask_0 &= end_mask_2;
        end_mask_2 = end_mask_0;
        if (src_words_dec_1 != 0) {
            skew |= cgblitter_fxsr_bit;
        } else if ((rect.origin.x & 15) > (at.x & 15)) {
            skew |= cgblitter_fxsr_bit;
            blitter->srcIncY -= 2;
        }
    } else {
        int idx = 0;
        if ((rect.origin.x & 15) > (at.x & 15)) {
            idx |= 1;
        }
        if (src_words_dec_1 == dst_words_dec_1) {
            idx |= 2;
        }
        skew |= pBlitter_skewflags[idx];
    }
    blitter->endMask[0] = end_mask_0;
    blitter->endMask[1] = 0xFFFF;
    blitter->endMask[2] = end_mask_2;

    // Counts
    blitter->countX  = (uint16_t)(dst_words_dec_1 + 1);
    
    // Operation flags
    blitter->HOP = cgblitter_hop_src;
    blitter->skew = skew;

    // Color 4 planes
    for (int i = 4; --i != -1; ) {
        if ((color & 1) == 0) {
            blitter->LOP = cgblitter_lop_notsrc_and_dst;
        } else {
            blitter->LOP = cgblitter_lop_src_or_dst;
        }
        blitter->pDst   = dts_bitmap;
        blitter->pSrc   = src_maskmap;
        blitter->countY = rect.size.height;

        blitter->start();

        color >>= 1;
        dts_bitmap++;
    }
}

void cgimage_c::imp_draw_rect_SLOW(const cgimage_c &srcImage, const cgrect_t &rect, cgpoint_t point) const {
    assert(get_size().contains(point));
    for (int y = 0; y < rect.size.height; y++) {
        for (int x = 0; x < rect.size.width; x++) {
            uint8_t color = srcImage.get_pixel(cgpoint_t{(int16_t)(rect.origin.x + x), (int16_t)(rect.origin.y + y)});
            if (color != MASKED_CIDX) {
                put_pixel(color, cgpoint_t{(int16_t)(point.x + x), (int16_t)(point.y + y)});
            }
        }
    }
}

