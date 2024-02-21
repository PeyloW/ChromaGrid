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

cgimage_c::cgimage_c(const cgimage_c *image, cgrect_t rect) {
    assert((rect.origin.x % 0xf) == 0);
    memcpy(this, image, sizeof(cgimage_c));
    _size = rect.size;
    int word_offset = image->_line_words * rect.origin.y + rect.origin.x / 16;
    _bitmap += word_offset * 4;
    if (_maskmap) {
        _maskmap += word_offset;
    }
    _owns_bitmap = false;
}

static void cgimage_read(cgiff_file_c &file, uint16_t line_words, int height, uint16_t *bitmap, uint16_t *maskmap) {
    uint16_t word_buffer[line_words];
    const int bp_count = (maskmap ? 5 : 4);
    while (height--) {
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
                for (int i = 0; i < line_words; i++) {
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
    while (height--) {
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

cgimage_c::cgimage_c(const char *path, bool masked, int8_t trans_cidx) {
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
    if (trans_cidx == -1 && masked)
    if (!file.read(&trans_cidx, 1)) {
        return; // Failed to read color.
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
            cgcolor_remap_table_t table;
            make_noremap_table(table);
            table[trans_cidx + 1] = cgtrans_cidx;
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

void cgimage_c::put_pixel(cgcolorindex_t ci, cgpoint_t at) {
    if (_clipping) {
        if (!_size.contains(at)) return;
    }
    assert(_size.contains(at));
    int word_offset = (at.x / 16) + at.y * _line_words;
    const uint16_t bit = 1 << (15 - at.x & 15);
    const uint16_t mask = ~bit;
    if (_maskmap != nullptr) {
        uint16_t *maskmap = this->_maskmap + word_offset;
        if (ci < 0) {
            *maskmap &= mask;
            return;
        } else {
            *maskmap |= bit;
        }
        ci = 0;
    } else if (ci < 0) {
        return;
    }
    uint16_t *bitmap = this->_bitmap + (word_offset << 2);
    for (int bp = 0; bp < 4; bp++) {
        if (ci & (1 << bp)) {
            *bitmap++ |= bit;
        } else {
            *bitmap++ &= mask;
        }
    }
}

cgcolorindex_t cgimage_c::get_pixel(cgpoint_t at) {
    if (!_clipping || _size.contains(at)) {
        int word_offset = (at.x / 16) + at.y * _line_words;
        const uint16_t bit = 1 << (15 - at.x & 15);
        if (_maskmap != nullptr) {
            uint16_t *maskmap = this->_maskmap + word_offset;
            if (!(*maskmap & bit)) {
                return cgtrans_cidx;
            }
        }
        cgcolorindex_t ci = 0;
        cgcolorindex_t cb = 1;
        uint16_t *bitmap = this->_bitmap + (word_offset << 2);
        for (int bp = 0; bp < 4; bp++) {
            if (*bitmap++ & bit) {
                ci |= cb;
            }
            cb <<= 1;
        }
        return ci;
    }
    return _maskmap != nullptr ? cgtrans_cidx : 0;
}

void cgimage_c::remap_colors(cgcolor_remap_table_t table, cgrect_t rect) {
    with_clipping(false, [this, table, rect] {
        for (int16_t y = rect.origin.y; y < rect.origin.y + rect.size.height; y++) {
            for (int16_t x = rect.origin.x; x < rect.origin.x + rect.size.width; x++) {
                const int8_t c = get_pixel((cgpoint_t){ x, y});
                const int8_t rc = table[c + 1];
                if (c != rc) {
                    put_pixel(rc, (cgpoint_t){ x, y});
                }
            }
        }
    });
}

void cgimage_c::fill(cgcolorindex_t ci, cgrect_t rect) {
    with_clipping(false, [this, ci, rect] {
        for (int16_t y = rect.origin.y; y < rect.origin.y + rect.size.height; y++) {
            for (int16_t x = rect.origin.x; x < rect.origin.x + rect.size.width; x++) {
                put_pixel(ci, cgpoint_t{ x, y });
            }
        }
    });
}

void cgimage_c::draw_aligned(cgimage_c *src, cgpoint_t at) {
    assert((at.x & 0xf) == 0);
    assert(src->_offset.x == 0);
    assert(src->_offset.x == 0);
    assert(src->_maskmap == nullptr);
    if (_clipping) {
        const cgrect_t rect = (cgrect_t){ at, src->get_size() };
        if (!rect.contained_by(_size)) {
            cgrect_t rect = (cgrect_t){ {0, 0}, src->get_size()};
            draw(src, rect, at);
            return;
        }
    }
    imp_draw_aligned(this, src, at);
}

void cgimage_c::draw(cgimage_c *src, cgpoint_t at) {
    const auto offset = src->get_offset();
    const cgpoint_t real_at = (cgpoint_t){static_cast<int16_t>(at.x - offset.x), static_cast<int16_t>(at.y - offset.y)};
    cgrect_t rect = (cgrect_t){ {0, 0}, src->get_size()};
    draw(src, rect, real_at);
}

void cgimage_c::draw(cgimage_c *src, cgrect_t rect, cgpoint_t at) {
    assert(rect.contained_by(get_size()));
    if (_clipping) {
        if (at.x < 0) {
            rect.size.width += at.x;
            if (rect.size.width <= 0) return;
            rect.origin.x -= at.x;
            at.x = 0;
        }
        if (at.y < 0) {
            rect.size.height += at.y;
            if (rect.size.height <= 0) return;
            rect.origin.y -= at.y;
            at.y = 0;
        }
        const auto size = this->get_size();
        const auto dx = size.width - (at.x + rect.size.width);
        if (dx < 0) {
            rect.size.width += dx;
            if (rect.size.width <= 0) return;
        }
        const auto dy = size.height - (at.y + rect.size.height);
        if (dy < 0) {
            rect.size.height += dy;
            if (rect.size.height <= 0) return;
        }
    }
    imp_draw_rect(this, src, &rect, at);
}


#ifndef __M68000__

void cgimage_c::imp_draw_aligned(cgimage_c *image, cgimage_c *srcImage, cgpoint_t point) {
    assert(image->get_size().contains(point));
    assert((point.x & 0xf) == 0);
    assert((srcImage->get_size().width & 0xf) == 0);
    const auto size = srcImage->get_size();
    const size_t word_offset = (point.x / 16) + point.y * image->_line_words;
    for (int y = 0; y < size.height; y++) {
        memcpy(image->_bitmap + (word_offset + image->_line_words * y) * 4,
               srcImage->_bitmap +(srcImage->_line_words * y * 4),
               srcImage->_line_words * 8);
    }
}

/*
void cgimage_t::imp_draw(cgimage_t *image, cgimage_t *srcImage, cgpoint_t point) {
    assert(image->get_size().contains(point));
    cgrect_t rect = { { 0, 0 }, srcImage->get_size() };
    imp_draw_rect(image, srcImage, &rect, point);
}
*/

void cgimage_c::imp_draw_rect(cgimage_c *image, cgimage_c *srcImage, cgrect_t *const rect, cgpoint_t point) {
    assert(image->get_size().contains(point));
    for (int y = 0; y < rect->size.height; y++) {
        for (int x = 0; x < rect->size.width; x++) {
            cgcolorindex_t color = srcImage->get_pixel(cgpoint_t{static_cast<int16_t>(rect->origin.x + x), static_cast<int16_t>(rect->origin.y + y)});
            if (color >= 0) {
                image->put_pixel(color, cgpoint_t{static_cast<int16_t>(point.x + x), static_cast<int16_t>(point.y + y)});
            }
        }
    }
}

#endif
