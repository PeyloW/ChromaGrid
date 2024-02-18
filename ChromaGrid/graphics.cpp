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
    const cgpalette_t *pActivePalette = NULL;
    const cgimage_t *pActiveImage = NULL;
#endif
}

void cgpalette_t::set_active() const {
#ifdef __M68000__
    memcpy(reinterpret_cast<uint16_t*>(0xffff8240), colors, sizeof(colors));
#else
    pActivePalette = this;
#endif
}

cgimage_t::cgimage_t(const cgsize_t size, mask_mode_t mask_mode, cgpalette_t *palette) {
    assert(mask_mode != mask_mode_auto);
    memset(this, 0, sizeof(cgimage_t));
    bool masked = mask_mode == mask_mode_masked;
    palette = palette;
    this->size = size;
    line_words = ((size.width + 15) / 16);
    uint16_t bitmap_words = (line_words * size.height) << 2;
    uint16_t mask_bytes = masked ? (line_words * size.height) : 0;
    bitmap = reinterpret_cast<uint16_t*>(calloc(bitmap_words + mask_bytes, 2));
    if (masked) {
        maskmap = bitmap + bitmap_words;
    }
    owns_bitmap = true;
    clipping = true;
}

cgimage_t::cgimage_t(const cgimage_t *image, cgrect_t rect) {
    assert((rect.origin.x % 0xf) == 0);
    memcpy(this, image, sizeof(cgimage_t));
    size = rect.size;
    int word_offset = image->line_words * rect.origin.y + rect.origin.x / 16;
    bitmap += word_offset * 4;
    if (maskmap) {
        maskmap += word_offset;
    }
    owns_bitmap = false;
}

static void cgimage_read(cgiff_file &file, uint16_t line_words, int height, uint16_t *bitmap, uint16_t *maskmap) {
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

static void cgimage_read_packbits(cgiff_file &file, uint16_t line_words, int height, uint16_t *bitmap, uint16_t *maskmap) {
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

cgimage_t::cgimage_t(const char *path, mask_mode_t mask_mode) {
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
    
    memset(this, 0, sizeof(cgimage_t));

    cgiff_file file(path);
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
    if (!file.read(&size.width, 4)) {
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
    uint16_t mask_color;
    if (!file.read(&mask_color, 1)) {
        return; // Failed to read color.
    }
    
    
    if (file.find(cgiff_id_make("CMAP"), &chunk)) {
        int cnt = chunk.size / 3;
        assert(cnt == 16);
        uint8_t cmpa[48];
        if (!file.read(cmpa, 48)) {
            return; // Could not read palette
        }
        palette = new cgpalette_t(&cmpa[0]);
    }
    if (file.find(cgiff_id_make("GRAB"), &chunk)) {
        if (!file.read(&offset.x, 2)) {
            return; // Failed ot read grab point
        }
    }
    if (!file.find(cgiff_id_make("BODY"), &chunk)) {
        return; // Could not find body
    }
    line_words = ((size.width + 15) / 16);
    const uint16_t bitmap_words = (line_words * size.height) << 2;
    const bool needs_mask_words = (mask_type == mask_type_color && mask_mode != mask_mode_none) || (mask_type == mask_type_plane);
    const uint16_t mask_words = needs_mask_words ? (bitmap_words >> 2) : 0;
    bitmap = reinterpret_cast<uint16_t*>(malloc((bitmap_words + mask_words) << 1));
    if (needs_mask_words) {
        maskmap = bitmap + bitmap_words;
    } else {
        maskmap = nullptr;
    }
    owns_bitmap = true;
    switch (compression_type) {
        case compression_type_none:
            cgimage_read(file, line_words, size.height, bitmap, mask_type == mask_type_plane ? maskmap : nullptr);
            break;
        case compression_type_packbits:
            cgimage_read_packbits(file, line_words, size.height, bitmap, mask_type == mask_type_plane ? maskmap : nullptr);
            break;
        default:
            break;
    }
    if (needs_mask_words) {
        if (mask_mode == mask_mode_none) {
            maskmap = nullptr;
        } else if (mask_type == mask_type_color) {
            memset(maskmap, -1, mask_words << 1);
            with_clipping(false, [this, mask_color]() {
                cgpoint_t at;
                for (at.y = 0; at.y < size.height; at.y++) {
                    for (at.x = 0; at.x < size.width; at.x++) {
                        const colorindex_t c = get_pixel(at);
                        if (c == mask_color) {
                            put_pixel(transparent_colorindex, at);
                        }
                    }
                }
            });
        }
    }
}

cgimage_t::~cgimage_t() {
    if (owns_bitmap) {
        free(bitmap);
    }
    if (super_image == nullptr && palette) {
        delete palette;
    }
}

void cgimage_t::set_active() const {
#ifdef __M68000__
    uint16_t word_offset = offset.y * line_words + (offset.x >> 4);
    uint32_t high_bytes =  (uint32_t)(bitmap + (word_offset << 4));
    set_screen(nullptr, high_bytes, 0);
    uint8_t low_byte = (uint8_t)high_bytes;
    uint8_t bit_shift = offset.x & 0x0f;
    uint8_t word_skip = line_words - 20;
    if (bit_shift != 0) {
        word_skip--;
    }
    word_skip <<= 2;
    // high_bytes = (high_bytes & 0xffff0000) | ((high_bytes & 0xff00) >> 8);
    __asm__ __volatile__ (
             "lsr.w #8,%[hb] \n\t"
             "move.l %[hb],0xffff8204.w \n\t"
             "move.b %[lb],0xffff8209.w \n\t"
             "move.b %[ws],0xffff820f.w \n\t"
             "move.b %[bs],0xffff8265.w \n\t"
             : [hb] "+d" (high_bytes), [lb] "+d" (low_byte), [ws] "+d" (word_skip), [bs] "+d" (bit_shift) : :);
#else
    pActiveImage = this;
#endif
}

void cgimage_t::put_pixel(colorindex_t ci, cgpoint_t at) {
    if (clipping) {
        if (!size.contains(at)) return;
    }
    assert(size.contains(at));
    int word_offset = (at.x / 16) + at.y * line_words;
    const uint16_t bit = 1 << (15 - at.x & 15);
    const uint16_t mask = ~bit;
    if (maskmap != nullptr) {
        uint16_t *maskmap = this->maskmap + word_offset;
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
    uint16_t *bitmap = this->bitmap + (word_offset << 2);
    for (int bp = 0; bp < 4; bp++) {
        if (ci & (1 << bp)) {
            *bitmap++ |= bit;
        } else {
            *bitmap++ &= mask;
        }
    }
}

colorindex_t cgimage_t::get_pixel(cgpoint_t at) {
    if (!clipping || size.contains(at)) {
        int word_offset = (at.x / 16) + at.y * line_words;
        const uint16_t bit = 1 << (15 - at.x & 15);
        if (maskmap != nullptr) {
            uint16_t *maskmap = this->maskmap + word_offset;
            if (!(*maskmap & bit)) {
                return transparent_colorindex;
            }
        }
        colorindex_t ci = 0;
        colorindex_t cb = 1;
        uint16_t *bitmap = this->bitmap + (word_offset << 2);
        for (int bp = 0; bp < 4; bp++) {
            if (*bitmap++ & bit) {
                ci |= cb;
            }
            cb <<= 1;
        }
        return ci;
    }
    return maskmap != nullptr ? transparent_colorindex : 0;
}

void cgimage_t::fill(colorindex_t ci, cgrect_t rect) {
    for (int16_t y = 0; y < rect.size.height; y++) {
        for (int16_t x = 0; x < rect.size.height; x++) {
            put_pixel(ci, cgpoint_t{ static_cast<int16_t>(rect.origin.x + x), static_cast<int16_t>(rect.origin.y + y) });
        }
    }
}

void cgimage_t::draw_aligned(cgimage_t *src, cgpoint_t at) {
    assert((at.x & 0xf) == 0);
    assert(src->offset.x == 0);
    assert(src->offset.x == 0);
    if (clipping) {
        const cgrect_t rect = (cgrect_t){ at, src->get_size() };
        if (!rect.contained_by(size)) {
            cgrect_t rect = (cgrect_t){ {0, 0}, src->get_size()};
            draw(src, rect, at);
            return;
        }
    }
    imp_draw_aligned(this, src, at);
}

void cgimage_t::draw(cgimage_t *src, cgpoint_t at) {
    const auto offset = src->get_offset();
    const cgpoint_t real_at = (cgpoint_t){static_cast<int16_t>(at.x - offset.x), static_cast<int16_t>(at.y - offset.y)};
    if (clipping) {
        const cgrect_t rect = (cgrect_t){ real_at, src->get_size() };
        if (!rect.contained_by(size)) {
            cgrect_t rect = (cgrect_t){ {0, 0}, src->get_size()};
            draw(src, rect, real_at);
            return;
        }
    }
    assert(size.contains(real_at));
    imp_draw(this, src, real_at);
}

void cgimage_t::draw(cgimage_t *src, cgrect_t rect, cgpoint_t at) {
    assert(rect.contained_by(get_size()));
    if (clipping) {
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

void cgimage_t::imp_draw_aligned(cgimage_t *image, cgimage_t *srcImage, cgpoint_t point) {
    assert(image->get_size().contains(point));
    assert((point.x & 0xf) == 0);
    assert((srcImage->get_size().width & 0xf) == 0);
    const auto size = srcImage->get_size();
    const size_t word_offset = (point.x / 16) + point.y * image->line_words;
    for (int y = 0; y < size.height; y++) {
        memcpy(image->bitmap + (word_offset + image->line_words * y) * 4,
               srcImage->bitmap +(srcImage->line_words * y * 4),
               srcImage->line_words * 8);
    }
    imp_draw(image, srcImage, point);
}

void cgimage_t::imp_draw(cgimage_t *image, cgimage_t *srcImage, cgpoint_t point) {
    assert(image->get_size().contains(point));
    cgrect_t rect = { { 0, 0 }, srcImage->get_size() };
    imp_draw_rect(image, srcImage, &rect, point);
}

void cgimage_t::imp_draw_rect(cgimage_t *image, cgimage_t *srcImage, cgrect_t *const rect, cgpoint_t point) {
    assert(image->get_size().contains(point));
    for (int y = 0; y < rect->size.height; y++) {
        for (int x = 0; x < rect->size.width; x++) {
            colorindex_t color = srcImage->get_pixel(cgpoint_t{static_cast<int16_t>(rect->origin.x + x), static_cast<int16_t>(rect->origin.y + y)});
            if (color >= 0) {
                image->put_pixel(color, cgpoint_t{static_cast<int16_t>(point.x + x), static_cast<int16_t>(point.y + y)});
            }
        }
    }
}

#endif
