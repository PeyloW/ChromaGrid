//
//  graphics_draw_blitter.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-24.
//

#include "canvas.hpp"
#include "blitter_atari.hpp"

#if !TOYBOX_TARGET_ATARI
#   error "For Atari target only"
#endif

using namespace toybox;

static const uint8_t pBlitter_skewflags[4] = {
    blitter_s::nfsr_bit,
    blitter_s::fxsr_bit,
    0,
    blitter_s::nfsr_bit|blitter_s::fxsr_bit,
};

static const uint16_t pBlitter_mask[17] = {
    0xffff, 0x7fff, 0x3fff, 0x1fff,
    0x0fff, 0x07ff, 0x03ff, 0x01ff,
    0x00ff, 0x007f, 0x003f, 0x001f,
    0x000f, 0x0007, 0x0003, 0x0001,
    0x0000
};

static const canvas_c::stencil_t *pActiveStencil = nullptr;

__forceinline static void set_active_stencil(struct blitter_s *blitter, const canvas_c::stencil_t *const stencil) {
    if (pActiveStencil != stencil) {
        memcpy(blitter->halftoneRAM, stencil, 32);
        pActiveStencil = stencil;
    }
}

void canvas_c::imp_fill(uint8_t color, rect_s rect) const {
    uint16_t dummy_src = 0;
    auto blitter = pBlitter;

    const int16_t dst_max_x = rect.max_x();
    const int16_t dst_words_dec_1  = ((dst_max_x / 16) - (rect.origin.x / 16));
    
    // Source
    blitter->srcIncX = 0;
    blitter->srcIncY = 0;
    blitter->pSrc   = &dummy_src;

    // Dest
    blitter->dstIncX  = 8;
    blitter->dstIncY = (_image._line_words * 8 - (dst_words_dec_1 * 8));
    const int16_t dst_word_offset = (rect.origin.y * _image._line_words) + (rect.origin.x / 16);
    uint16_t *dts_bitmap = _image._bitmap + dst_word_offset * 4l;

    // Mask
    uint16_t end_mask_0 = pBlitter_mask[rect.origin.x & 15];
    uint16_t end_mask_2 = ~pBlitter_mask[(dst_max_x & 15) + 1];
    if (dst_words_dec_1 == 0) {
        end_mask_0 &= end_mask_2;
        end_mask_2 = end_mask_0;
    }
    blitter->endMask[0] = end_mask_0;
    blitter->endMask[1] = 0xFFFF;
    blitter->endMask[2] = end_mask_2;

    // Counts
    blitter->countX  = (dst_words_dec_1 + 1);
    
    // Operation flags
    if (_stencil) {
        set_active_stencil(blitter, _stencil);
        blitter->HOP = blitter_s::hop_e::halftone;
    } else {
        blitter->HOP = blitter_s::hop_e::one;
    }
    blitter->skew = 0;
    

    // Color 4 planes
    for (int i = 4; --i != -1; ) {
        if ((color & 1) == 0) {
            blitter->LOP = blitter_s::lop_e::notsrc_and_dst;
        } else {
            blitter->LOP = blitter_s::lop_e::src_or_dst;
        }
        blitter->pDst   = dts_bitmap;
        blitter->countY = rect.size.height;

        blitter->start();

        color >>= 1;
        dts_bitmap++;
    }

}

void canvas_c::imp_draw_aligned(const image_c &srcImage, const rect_s &rect, point_s at) const {
    assert((rect.origin.x & 0xf) == 0);
    assert((rect.size.width & 0xf) == 0);
    assert((at.x & 0xf) == 0);
    assert((srcImage.size().width & 0xf) == 0);
    assert(!rect.size.is_empty());
    assert(rect_s(at, rect.size).contained_by(size()));
    assert(rect.contained_by(srcImage.size()));
        
    auto blitter = pBlitter;
    const int16_t copy_words = (rect.size.width / 16);

    // Source
    blitter->srcIncX  = 2;
    blitter->srcIncY = (srcImage._line_words - copy_words) * 8 + 2;
    const int16_t src_word_offset = (rect.origin.y * srcImage._line_words) + (rect.origin.x / 16);
    blitter->pSrc = srcImage._bitmap + src_word_offset * 4l;

    // Dest
    blitter->dstIncX  = 2;
    blitter->dstIncY = (_image._line_words - copy_words) * 8 + 2;
    const int16_t dst_word_offset = (at.y * _image._line_words) + (at.x / 16);
    blitter->pDst = _image._bitmap + dst_word_offset * 4l;

    // Mask
    blitter->endMask[0] =  0xFFFF;
    blitter->endMask[1] =  0xFFFF;
    blitter->endMask[2] =  0xFFFF;

    // Counts
    blitter->countX  = (copy_words) * 4;
    blitter->countY = rect.size.height;
    blitter->skew = 0;
    
    // Operation flags
    if (_stencil) {
        const bool hog = blitter->countX <= 8;
        const auto countY = blitter->countY;
        blitter->HOP = blitter_s::hop_e::src;
        blitter->LOP = blitter_s::lop_e::src;
        for (int y = 0; y < countY; y++) {
            blitter->countY = 1;
            blitter->endMask[0] = blitter->endMask[1] = blitter->endMask[2] = ((uint16_t *)_stencil)[y & 0xf];
            blitter->start(hog);
        }
    } else {
        blitter->HOP = blitter_s::hop_e::src;
        blitter->LOP = blitter_s::lop_e::src;
        blitter->start();
    }
}

void canvas_c::imp_draw(const image_c &srcImage, const rect_s &rect, point_s at) const {
    assert(!rect.size.is_empty());
    assert(rect_s(at, rect.size).contained_by(size()));
    assert(rect.contained_by(srcImage.size()));
    auto blitter = pBlitter;

    const int16_t src_max_x       = rect.max_x();
    const int16_t dst_max_x       = (at.x + rect.size.width - 1);
    const int16_t src_words_dec_1 = ((src_max_x / 16) - (rect.origin.x / 16));
    const int16_t dst_words_dec_1 = ((dst_max_x / 16) - (at.x / 16));
    
    // Source
    blitter->srcIncX = 8;
    blitter->srcIncY = ((srcImage._line_words - src_words_dec_1) * 8);
    const int16_t src_word_offset = (rect.origin.y * srcImage._line_words) + (rect.origin.x / 16);
    uint16_t *src_bitmap  = srcImage._bitmap + src_word_offset * 4l;
    
    // Dest
    blitter->dstIncX  = 8;
    blitter->dstIncY = ((_image._line_words - dst_words_dec_1) * 8);
    const uint16_t dst_word_offset = (at.y * _image._line_words) + (at.x / 16);
    uint16_t *dts_bitmap  = _image._bitmap + dst_word_offset * 4l;

    // Mask
    uint16_t end_mask_0 = pBlitter_mask[at.x & 15];
    uint16_t end_mask_2 = ~pBlitter_mask[(dst_max_x & 15) + 1];
    uint8_t skew = (uint8_t)(((at.x & 15) - (rect.origin.x & 15)) & 15);
    if (dst_words_dec_1 == 0) {
        end_mask_0 &= end_mask_2;
        end_mask_2 = end_mask_0;
        if (src_words_dec_1 != 0) {
            skew |= blitter_s::fxsr_bit;
        } else if ((rect.origin.x & 15) > (at.x & 15)) {
            skew |= blitter_s::fxsr_bit;
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
    blitter->countX  = (dst_words_dec_1 + 1);
    
    // Operation flags
    blitter->HOP = blitter_s::hop_e::src;
    blitter->LOP = blitter_s::lop_e::src;
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

void canvas_c::imp_draw_masked(const image_c &srcImage, const rect_s &rect, point_s at) const {
    assert(!rect.size.is_empty());
    assert(rect_s(at, rect.size).contained_by(size()));
    assert(rect.contained_by(srcImage.size()));
    auto blitter = pBlitter;

    const int16_t src_max_x       = rect.max_x();
    const int16_t dst_max_x       = (at.x + rect.size.width - 1);
    const int16_t src_words_dec_1 = ((src_max_x / 16) - (rect.origin.x / 16));
    const int16_t dst_words_dec_1 = ((dst_max_x / 16) - (at.x / 16));
    
    // Source
    blitter->srcIncX = 2;
    blitter->srcIncY = ((srcImage._line_words - src_words_dec_1) * 2);
    const int16_t src_word_offset = (rect.origin.y * srcImage._line_words) + (rect.origin.x / 16);
    uint16_t *src_maskmap  = srcImage._maskmap + src_word_offset;

    // Dest
    blitter->dstIncX  = 8;
    blitter->dstIncY = (_image._line_words * 8 - (dst_words_dec_1 * 8));
    const int16_t dst_word_offset = (at.y * _image._line_words) + (at.x / 16);
    uint16_t *dts_bitmap  = _image._bitmap + dst_word_offset * 4l;

    // Mask
    uint16_t end_mask_0 = pBlitter_mask[at.x & 15];
    uint16_t end_mask_2 = ~pBlitter_mask[(dst_max_x & 15) + 1];
    uint8_t skew = (uint8_t)(((at.x & 15) - (rect.origin.x & 15)) & 15);
    if (dst_words_dec_1 == 0) {
        end_mask_0 &= end_mask_2;
        end_mask_2 = end_mask_0;
        if (src_words_dec_1 != 0) {
            skew |= blitter_s::fxsr_bit;
        } else if ((rect.origin.x & 15) > (at.x & 15)) {
            skew |= blitter_s::fxsr_bit;
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
    blitter->countX  = (dst_words_dec_1 + 1);
    
    // Operation flags
    blitter->HOP = blitter_s::hop_e::src;
    blitter->LOP = blitter_s::lop_e::notsrc_and_dst;
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
    blitter->LOP = blitter_s::lop_e::src_or_dst;

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

void canvas_c::imp_draw_color(const image_c &srcImage, const rect_s &rect, point_s at, uint16_t color) const {
    assert(!rect.size.is_empty());
    assert(rect_s(at, rect.size).contained_by(size()));
    assert(rect.contained_by(srcImage.size()));
    auto blitter = pBlitter;

    const int16_t src_max_x       = rect.max_x();
    const int16_t dst_max_x       = (at.x + rect.size.width - 1);
    const int16_t src_words_dec_1 = ((src_max_x / 16) - (rect.origin.x / 16));
    const int16_t dst_words_dec_1 = ((dst_max_x / 16) - (at.x / 16));
    
    // Source
    blitter->srcIncX = 2;
    blitter->srcIncY = ((srcImage._line_words - src_words_dec_1) * 2);
    const int16_t src_word_offset = (rect.origin.y * srcImage._line_words) + (rect.origin.x / 16);
    uint16_t *src_maskmap  = srcImage._maskmap + src_word_offset;
    
    // Dest
    blitter->dstIncX  = 8;
    blitter->dstIncY = (_image._line_words * 8 - (dst_words_dec_1 * 8));
    const int16_t dst_word_offset = (at.y * _image._line_words) + (at.x / 16);
    uint16_t *dts_bitmap  = _image._bitmap + dst_word_offset * 4l;

    // Mask
    uint16_t end_mask_0 = pBlitter_mask[at.x & 15];
    uint16_t end_mask_2 = ~pBlitter_mask[(dst_max_x & 15) + 1];
    uint8_t skew = (uint8_t)(((at.x & 15) - (rect.origin.x & 15)) & 15);
    if (dst_words_dec_1 == 0) {
        end_mask_0 &= end_mask_2;
        end_mask_2 = end_mask_0;
        if (src_words_dec_1 != 0) {
            skew |= blitter_s::fxsr_bit;
        } else if ((rect.origin.x & 15) > (at.x & 15)) {
            skew |= blitter_s::fxsr_bit;
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
    blitter->countX  = (dst_words_dec_1 + 1);
    
    // Operation flags
    blitter->HOP = blitter_s::hop_e::src;
    blitter->skew = skew;

    // Color 4 planes
    for (int i = 4; --i != -1; ) {
        if ((color & 1) == 0) {
            blitter->LOP = blitter_s::lop_e::notsrc_and_dst;
        } else {
            blitter->LOP = blitter_s::lop_e::src_or_dst;
        }
        blitter->pDst   = dts_bitmap;
        blitter->pSrc   = src_maskmap;
        blitter->countY = rect.size.height;

        blitter->start();

        color >>= 1;
        dts_bitmap++;
    }
}

void canvas_c::imp_draw_rect_SLOW(const image_c &srcImage, const rect_s &rect, point_s at) const {
    assert(!rect.size.is_empty());
    assert(rect_s(at, rect.size).contained_by(size()));
    assert(rect.contained_by(srcImage.size()));
    for (int y = rect.size.height; --y != -1; ) {
        for (int x = rect.size.width; --x != -1 ; ) {
            int color = srcImage.get_pixel(point_s{(int16_t)(rect.origin.x + x), (int16_t)(rect.origin.y + y)});
            if (!image_c::is_masked(color)) {
                put_pixel(color, point_s{(int16_t)(at.x + x), (int16_t)(at.y + y)});
            }
        }
    }
}

