//
//  graphics.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-04.
//

#include "graphics.hpp"
#include "iff_file.hpp"
#include "system.hpp"

extern "C" {
    //void m68_cgimage_set(cgimage_t *image, cgpoint_t point);
    //colorindex_t m68_cgimage_get(cgimage_t *image, cgpoint_t point);
    //void m68_cgimage_draw_aligned(cgimage_t *image, cgimage_t *srcImage, cgpoint_t point);
    //void m68_cgimage_draw(cgimage_t *image, cgimage_t *srcImage, cgpoint_t point);
    //void m68_cgimage_draw_rect(cgimage_t *image, cgimage_t *srcImage, cgrect_t *const rect, cgpoint_t point);
#ifndef __M68000__
    const cgpalette_c *pActivePalette = NULL;
    const cgimage_c *pActiveImage = NULL;
#endif
}

void cgpalette_c::set_active() const {
#ifdef __M68000__
    memcpy(reinterpret_cast<uint16_t*>(0xffff8240), colors, sizeof(colors));
#else
    pActivePalette = this;
#endif
}

static void make_dither_mask_orderred(uint16_t mask[16], int shade) {
    const static uint8_t bayer_8x8[8][8] = {
        { 0, 32,  8, 40,  2, 34, 10, 42},
        {48, 16, 56, 24, 50, 18, 58, 26},
        {12, 44,  4, 36, 14, 46,  6, 38},
        {60, 28, 52, 20, 62, 30, 54, 22},
        { 3, 35, 11, 43,  1, 33,  9, 41},
        {51, 19, 59, 27, 49, 17, 57, 25},
        {15, 47,  7, 39, 13, 45,  5, 37},
        {63, 31, 55, 23, 61, 29, 53, 21}
    };
    for (int y = 8; --y != -1; ) {
        mask[y] = 0;
        for (int x = 8; --x != -1; ) {
            if (bayer_8x8[x][y] < shade) {
                mask[y] |= (0x101 << x);
            }
        }
    }
    for (int y = 8; --y != -1; ) {
        mask[y + 8] = mask[y];
    }
}

void make_dither_mask_blue_noise(uint16_t mask[16], int shade) {
    const static uint8_t blue_16x16[16][16] = {
        { 10,  2, 17, 23, 10, 34,  4, 28, 37,  2, 19,  7,  3,  1,  5, 62 },
        {  0,  8, 55, 46,  1, 61, 19,  0,  1,  9, 45, 25, 34, 16,  0, 24 },
        { 13, 28,  5,  0,  3, 26, 12,  6, 42, 14, 59,  0, 11,  2, 50, 42 },
        {  1, 37, 20, 14, 32,  8,  2, 53, 22,  3,  0, 29,  4, 56, 18,  3 },
        {  0, 57,  2,  6, 50,  0, 38,  0, 31,  8, 17, 39,  1,  6, 32,  9 },
        { 47, 23, 10,  1, 42, 21, 16,  4, 58,  5, 48,  2, 13, 21,  0, 15 },
        {  5, 35,  0, 63,  4,  1, 28, 11,  0,  1, 25,  9, 62, 27, 41,  2 },
        {  8, 29, 18, 12, 26,  7, 53, 14, 43, 19, 36,  0,  4,  0, 54,  1 },
        { 51,  0,  3,  0, 39,  2,  0, 34,  6,  3, 10, 46, 16,  6, 12, 20 },
        { 40, 14, 57, 22, 47, 10,  1, 59, 24,  0, 52, 30,  2, 36, 26,  0 },
        {  9, 32,  2,  6, 16, 31, 20,  4,  1, 15,  7, 21,  1, 60,  7,  3 },
        {  0,  4, 45,  1,  0,  3, 49, 12, 35, 55,  0,  4, 13,  0, 49, 17 },
        { 58, 27, 11, 36, 61,  8, 18,  0, 40, 27,  9, 44, 33,  5, 38, 23 },
        {  0,  7, 19,  0, 24, 29,  2,  0,  6,  3,  1, 17, 25,  2, 11,  1 },
        { 43, 52,  3, 13,  5, 54, 44, 11, 22, 63, 31,  0, 56,  8,  0, 15 },
        {  4, 33,  0, 41,  1,  7,  0, 15, 51,  5,  0, 12, 48, 39, 21, 30 }
    };
    for (int y = 16; --y != -1; ) {
        mask[y] = 0;
        for (int x = 16; --x != -1; ) {
            if (blue_16x16[x][y] < shade) {
                mask[y] |= (0x1 << x);
            }
        }
    }
}


cgstencil_c::cgstencil_c(type t, int shade) {
    assert(shade >= FULLY_TRANSPARENT);
    assert(shade <= FULLY_OPAQUE);
    switch (t) {
        case orderred:
            make_dither_mask_orderred(_mask, shade);
            break;
        case noise:
            make_dither_mask_blue_noise(_mask, shade);
            break;
        default:
            assert(0);
            break;
    }
}

cgimage_c::cgimage_c(const cgsize_t size, bool masked, cgpalette_c *palette) {
    memset(this, 0, sizeof(cgimage_c));
    palette = palette;
    this->_size = size;
    _line_words = ((size.width + 15) / 16);
    uint16_t bitmap_words = (_line_words * size.height) << 2;
    uint16_t mask_bytes = masked ? (_line_words * size.height) : 0;
    _bitmap = reinterpret_cast<uint16_t*>(calloc(bitmap_words + mask_bytes, 2));
    if (masked) {
        _maskmap = _bitmap + bitmap_words;
    }
    _owns_bitmap = true;
    _clipping = true;
}

cgimage_c::cgimage_c(const cgimage_c &image, cgrect_t rect) {
    assert((rect.origin.x % 0xf) == 0);
    memcpy(this, &image, sizeof(cgimage_c));
    _size = rect.size;
    int word_offset = image._line_words * rect.origin.y + rect.origin.x / 16;
    _bitmap += word_offset * 4;
    if (_maskmap) {
        _maskmap += word_offset;
    }
    _owns_bitmap = false;
}

static void cgimage_read(cgiff_file_c &file, uint16_t line_words, int height, uint16_t *bitmap, uint16_t *maskmap) {
    uint16_t word_buffer[line_words];
    const int bp_count = (maskmap ? 5 : 4);
    while (--height != -1) {
        for (int bp = 0; bp < bp_count; bp++) {
            uint16_t *buffer;
            if (bp < 4) {
                buffer = (uint16_t*)word_buffer;
            } else {
                buffer = (uint16_t*)maskmap;
            }
            if (!file.read(buffer, line_words)) {
                return; // Failed to read line
            }
            if (bp < 4) {
                for (int i = line_words; --i != -1; ) {
                    bitmap[bp + i * 4] = buffer[i];
                }
            }
        }
        bitmap += line_words * 4;
        if (maskmap) {
            maskmap += line_words;
        }
    }
}

static void cgimage_read_packbits(cgiff_file_c &file, uint16_t line_words, int height, uint16_t *bitmap, uint16_t *maskmap) {
    const int bp_count = (maskmap ? 5 : 4);
    uint16_t word_buffer[line_words * bp_count];
    while (--height != -1) {
        int8_t *buffer = (int8_t*)word_buffer;
        int8_t *bufferEnd = buffer + (line_words * bp_count * 2);
        while (buffer < bufferEnd) {
            int8_t cmd;
            if (!file.read(&cmd, 1)) {
                return; // Failed read
            }
            if (cmd >= 0) {
                const int to_read = cmd + 1;
                if (!file.read(buffer, to_read)) {
                    return; // Failed read
                }
                buffer += to_read;
            } else if (cmd != -128) {
                int8_t data;
                if (!file.read(&data, 1)) {
                    return; // Failed read
                }
                while (cmd++ <= 0) {
                    *buffer++ = data;
                }
            }
        }
        for (int bp = 0; bp < bp_count; bp++) {
            if (bp < 4) {
                for (int i = 0; i < line_words; i++) {
                    bitmap[bp + i * 4] = cg_htons(word_buffer[bp * line_words + i]);
                }
            } else {
                for (int i = 0; i < line_words; i++) {
                    maskmap[bp + i * 4] = cg_htons(word_buffer[bp * line_words + i]);
                }
            }
        }
        bitmap += line_words * 4;
        if (maskmap) {
            maskmap += line_words;
        }
    }
}

cgimage_c::cgimage_c(const char *path, bool masked, uint8_t masked_cidx) {
    typedef enum  {
        mask_type_none,
        mask_type_plane,
        mask_type_color,
        mask_type_lasso,
    } mask_type_t;
    typedef enum {
        compression_type_none,
        compression_type_packbits,
        compression_type_vertical
    } compression_type_t;
    
    memset(this, 0, sizeof(cgimage_c));

    cgiff_file_c file(path);
    cgiff_header_t header;
    if (!file.read(&header)) {
        hard_assert(0);
        return; // Not a ILBM
    }
    if (!cgiff_id_equals(header.subtype, "ILBM")) {
        printf("subtype: %lx\n\r", header.subtype);
        hard_assert(0);
        return; // Not a ILBM
    }
    cgiff_chunk_t chunk;
    if (!file.find(cgiff_id_make("BMHD"), &chunk)) {
        return; // Did nto find header
    }
    if (!file.read(&_size.width, 4)) {
        return; // Could not read size + offset
    }
    uint8_t bmhd[4];
    if (!file.read(bmhd, 4)) {
        return; // Failed to read header
    }
    assert(bmhd[0] == 4); // Only 4 bitplanes supported
    const mask_type_t mask_type = (mask_type_t)bmhd[1];
    assert(mask_type < mask_type_lasso); // Lasso not supported
    const compression_type_t compression_type = (compression_type_t)bmhd[2];
    assert(compression_type < compression_type_vertical); // DeluxePain ST format not supported
    if (masked_cidx == MASKED_CIDX && masked) {
        uint16_t tmp_masked_cidx;
        if (!file.read(&tmp_masked_cidx, 1)) {
            return; // Failed to read color.
        }
        masked_cidx = tmp_masked_cidx;
    }
    
    if (file.find(cgiff_id_make("CMAP"), &chunk)) {
        int cnt = chunk.size / 3;
        assert(cnt == 16);
        uint8_t cmpa[48];
        if (!file.read(cmpa, 48)) {
            return; // Could not read palette
        }
        _palette = new cgpalette_c(&cmpa[0]);
    }
    if (file.find(cgiff_id_make("GRAB"), &chunk)) {
        if (!file.read(&_offset.x, 2)) {
            return; // Failed ot read grab point
        }
    }
    if (!file.find(cgiff_id_make("BODY"), &chunk)) {
        return; // Could not find body
    }
    _line_words = ((_size.width + 15) / 16);
    const uint16_t bitmap_words = (_line_words * _size.height) << 2;
    const bool needs_mask_words = masked || (mask_type == mask_type_plane);
    const uint16_t mask_words = needs_mask_words ? (bitmap_words >> 2) : 0;
    _bitmap = reinterpret_cast<uint16_t*>(malloc((bitmap_words + mask_words) << 1));
    if (needs_mask_words) {
        _maskmap = _bitmap + bitmap_words;
    } else {
        _maskmap = nullptr;
    }
    _owns_bitmap = true;
    switch (compression_type) {
        case compression_type_none:
            cgimage_read(file, _line_words, _size.height, _bitmap, mask_type == mask_type_plane ? _maskmap : nullptr);
            break;
        case compression_type_packbits:
            cgimage_read_packbits(file, _line_words, _size.height, _bitmap, mask_type == mask_type_plane ? _maskmap : nullptr);
            break;
        default:
            break;
    }
    if (needs_mask_words) {
        if (!masked) {
            _maskmap = nullptr;
        } else if (mask_type == mask_type_color) {
            memset(_maskmap, -1, mask_words << 1);
            remap_table_t table;
            make_noremap_table(table);
            table[masked_cidx] = MASKED_CIDX;
            remap_colors(table, (cgrect_t){ {0, 0}, _size});
        }
    }
}

cgimage_c::~cgimage_c() {
    if (_owns_bitmap) {
        free(_bitmap);
    }
    if (_super_image == nullptr && _palette) {
        delete _palette;
    }
}

#ifdef __M68000__
static uint16_t pSetActiveVBLCode[20];
#endif

void cgimage_c::set_active() const {
#ifdef __M68000__
    uint16_t word_offset = _offset.y * _line_words + (_offset.x >> 4);
    uint32_t high_bytes = (uint32_t)(_bitmap + (word_offset << 2));
    uint8_t low_byte = (uint8_t)high_bytes;
    __asm__ ("lsr.w #8,%[hb]" : [hb] "+d" (high_bytes) : :);
    uint8_t bit_shift = _offset.x & 0x0f;
    uint8_t word_skip = _line_words - 20;
    if (bit_shift != 0) {
        word_skip--;
    }
    word_skip <<= 2;
    
    // move.l #highBytes,$ffff8204.w
    // move.b #lowByte,$ffff8209.w
    // move.b #wordSkip,$ffff820f.w
    // move.b #shift,$ffff8265.w
    // rts
    uint16_t *code = pSetActiveVBLCode;
    __append_int16(code, 0x21fc);
    __append_int32(code, high_bytes);
    __append_int32(code, 0x820411fcl);
    __append_int16(code, low_byte);
    __append_int32(code, 0x820911fcl);
    __append_int16(code, word_skip);
    __append_int32(code, 0x820f11fcl);
    __append_int16(code, bit_shift);
    __append_int32(code, 0x82654e75);
    cgtimer_c vbl(cgtimer_c::vbl);
    vbl.add_func((cgtimer_c::func_t)pSetActiveVBLCode);
#else
    pActiveImage = this;
#endif
}

cgfont_c::cgfont_c(const cgimage_c &image, cgsize_t character_size) : _image(image) {
    const int cols = image.get_size().width / character_size.width;
    for (int i = 0; i < 96; i++) {
        const int col = i % cols;
        const int row = i / cols;
        _rects[i] = (cgrect_t){{(int16_t)(col * character_size.width), (int16_t)(row * character_size.height)}, character_size };
    }
}

cgfont_c::cgfont_c(const cgimage_c &image, cgsize_t max_size, uint8_t space_width, uint8_t lead_req_space, uint8_t trail_req_space) : _image(image)  {
    const int cols = image.get_size().width / max_size.width;
    for (int i = 0; i < 96; i++) {
        const int col = i % cols;
        const int row = i / cols;
        cgrect_t rect = (cgrect_t){{(int16_t)(col * max_size.width), (int16_t)(row * max_size.height)}, max_size };
        if (i == 0) {
            rect.size.width = space_width;
        } else {
            // Find first non-empty column, count spaces from top
            {
                for (int fc = 0; fc < rect.size.width; fc++) {
                    for (int fcc = 0; fcc < rect.size.height; fcc++) {
                        const cgpoint_t at = (cgpoint_t){ (int16_t)(rect.origin.x + fc), (int16_t)(rect.origin.y + fcc)};
                        if (image.get_pixel(at) != cgimage_c::MASKED_CIDX) {
                            fc = MAX(0, fcc >= lead_req_space ? fc : fc - 1);
                            rect.origin.x += fc;
                            rect.size.width -= fc;
                            goto leading_done;
                        }
                    }
                }
                rect.size.width = space_width;
                goto trailing_done;
            }
        leading_done:
            // Find last non-empty column, count spaces from bottom
            {
                const cgpoint_t max_at = (cgpoint_t){ (int16_t)(rect.origin.x + rect.size.width - 1), (int16_t)(rect.origin.y + rect.size.height - 1)};
                for (int fc = 0; fc < rect.size.width; fc++) {
                    for (int fcc = 0; fcc < rect.size.height; fcc++) {
                        const cgpoint_t at = (cgpoint_t){ (int16_t)(max_at.x - fc), (int16_t)(max_at.y - fcc)};
                        if (image.get_pixel(at) != cgimage_c::MASKED_CIDX) {
                            fc = MAX(0, fcc >= trail_req_space ? fc : fc - 1);
                            rect.size.width -= fc;
                            goto trailing_done;
                        }
                    }
                }
            }
        trailing_done: ;
        }
        _rects[i] = rect;
    }
}
