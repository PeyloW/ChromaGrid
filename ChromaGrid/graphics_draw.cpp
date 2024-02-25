//
//  graphics_draw.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-23.
//

#include "graphics.hpp"

void cgimage_c::restore(const cgimage_c &clean_image, bool *const dirtymap) const {
    assert(_size == clean_image.get_size());
    assert((_size.width & 0xf) == 0);
    assert((_size.height & 0xf) == 0);
    const_cast<cgimage_c*>(this)->with_clipping(false, [this, &clean_image, dirtymap] {
        cgimage_c subimage(clean_image, (cgrect_t){{0, 0}, {16, 16}});
        const int row_count = _size.height / 16;
        const int row_words = _line_words * 16;
        int16_t y = _size.height - 16;
        for (int row = row_count; --row != -1; y -= 16) {
            const int row_offset = row * _line_words;
            const int row_word_offset = row * row_words;
            int16_t x = _size.width - 16;
            for (int col = _line_words; --col != -1; x -= 16) {
                if (dirtymap[col + row_offset]) {
                    dirtymap[col + row_offset] = false;
                    subimage._bitmap = clean_image._bitmap + (row_word_offset + col) * 4;
                    draw_aligned(subimage, (cgpoint_t){x, y});
                }
            }
        }
    });
}

void cgimage_c::put_pixel(uint8_t ci, cgpoint_t at) const {
    if (_clipping) {
        if (!_size.contains(at)) return;
    }
    assert(_size.contains(at));
    int word_offset = (at.x / 16) + at.y * _line_words;
    const uint16_t bit = 1 << (15 - at.x & 15);
    const uint16_t mask = ~bit;
    if (_maskmap != nullptr) {
        uint16_t *maskmap = this->_maskmap + word_offset;
        if (ci == MASKED_CIDX) {
            *maskmap &= mask;
        } else {
            *maskmap |= bit;
        }
    } else if (ci < 0) {
        return;
    }
    uint16_t *bitmap = this->_bitmap + (word_offset << 2);
    uint8_t cb = 1;
    for (int bp = 4; --bp != -1; cb <<= 1) {
        if (ci & cb) {
            *bitmap++ |= bit;
        } else {
            *bitmap++ &= mask;
        }
    }
}

uint8_t cgimage_c::get_pixel(cgpoint_t at) const {
    if (!_clipping || _size.contains(at)) {
        int word_offset = (at.x / 16) + at.y * _line_words;
        const uint16_t bit = 1 << (15 - at.x & 15);
        if (_maskmap != nullptr) {
            uint16_t *maskmap = this->_maskmap + word_offset;
            if (!(*maskmap & bit)) {
                return MASKED_CIDX;
            }
        }
        uint8_t ci = 0;
        uint8_t cb = 1;
        uint16_t *bitmap = this->_bitmap + (word_offset << 2);
        for (int bp = 4; --bp != -1; cb <<= 1) {
            if (*bitmap++ & bit) {
                ci |= cb;
            }
        }
        return ci;
    }
    return _maskmap != nullptr ? MASKED_CIDX : 0;
}

void cgimage_c::remap_colors(remap_table_t table, cgrect_t rect) const {
    const_cast<cgimage_c*>(this)->with_clipping(false, [this, table, &rect] {
        for (int16_t y = rect.origin.y; y < rect.origin.y + rect.size.height; y++) {
            for (int16_t x = rect.origin.x; x < rect.origin.x + rect.size.width; x++) {
                const uint8_t c = get_pixel((cgpoint_t){ x, y});
                const uint8_t rc = table[c];
                if (c != rc) {
                    put_pixel(rc, (cgpoint_t){ x, y});
                }
            }
        }
    });
}

void cgimage_c::fill(uint8_t ci, cgrect_t rect) const {
    const_cast<cgimage_c*>(this)->with_clipping(false, [this, ci, &rect] {
        for (int16_t y = rect.origin.y; y < rect.origin.y + rect.size.height; y++) {
            for (int16_t x = rect.origin.x; x < rect.origin.x + rect.size.width; x++) {
                put_pixel(ci, cgpoint_t{ x, y });
            }
        }
    });
}

void cgimage_c::draw_aligned(const cgimage_c &src, cgpoint_t at) const {
    assert((at.x & 0xf) == 0);
    assert(src._offset.x == 0);
    assert(src._offset.x == 0);
    assert(src._maskmap == nullptr);
    if (_clipping) {
        const cgrect_t rect = (cgrect_t){ at, src.get_size() };
        if (!rect.contained_by(_size)) {
            cgrect_t rect = (cgrect_t){ {0, 0}, src.get_size()};
            draw(src, rect, at);
            return;
        }
    }
    if (_dirtymap) {
        const cgrect_t dirty_rect = (cgrect_t){ at, src.get_size() };
        imp_update_dirtymap(dirty_rect);
    }
    imp_draw_aligned(src, at);
}

void cgimage_c::draw(const cgimage_c &src, cgpoint_t at) const {
    const auto offset = src.get_offset();
    const cgpoint_t real_at = (cgpoint_t){ (int16_t)(at.x - offset.x), (int16_t)(at.y - offset.y)};
    cgrect_t rect = (cgrect_t){ {0, 0}, src.get_size()};
    draw(src, rect, real_at);
}

void cgimage_c::draw(const cgimage_c &src, cgrect_t rect, cgpoint_t at) const {
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
    if (_dirtymap) {
        const cgrect_t dirty_rect = (cgrect_t){at, rect.size};
        imp_update_dirtymap(dirty_rect);
    }
    imp_draw_rect(src, &rect, at);
}

void cgimage_c::draw(const cgfont_c &font, const char *text, cgpoint_t at, text_alignment alignment) const {
    int len = (int)strlen(text);
    cgsize_t size = font.get_rect(text[1]).size;
    for (int i = 1; i < len; i++) {
        size.width += font.get_rect(text[i]).size.width;
    }
    switch (alignment) {
        case align_right:
            at.x -= size.width;
            break;
        case align_center:
            at.x -= size.width / 2;
            break;
        default:
            break;
    }
    for (int i = 0; i < len; i++) {
        const cgrect_t &rect = font.get_rect(text[i]);
        draw(font.get_image(), rect, at);
        at.x += rect.size.width;
    }
}

void cgimage_c::imp_update_dirtymap(cgrect_t rect) const {
    assert(_dirtymap);
    assert((_size.width & 0xf) == 0);
    assert((_size.height & 0xf) == 0);
    const int x1 = rect.origin.x / 16;
    const int x2 = (rect.origin.x + rect.size.width - 1) / 16;
    const int y1 = rect.origin.y / 16;
    const int y2 = (rect.origin.y + rect.size.height - 1) / 16;
    for (int y = y1; y <= y2; y++) {
        const int line_offset = y * _line_words;
        for (int x = x1; x <= x2; x++) {
            _dirtymap[x + line_offset] = true;
        }
    }
}

#if 0
#ifndef __M68000__

void cgimage_c::imp_draw_aligned(const cgimage_c &srcImage, cgpoint_t point) const {
    assert(get_size().contains(point));
    assert((point.x & 0xf) == 0);
    assert((srcImage.get_size().width & 0xf) == 0);
    const auto size = srcImage.get_size();
    const int word_offset = (point.x / 16) + point.y * _line_words;
    for (int y = 0; y < size.height; y++) {
        memcpy(_bitmap + (word_offset + _line_words * y) * 4,
               srcImage._bitmap + (srcImage._line_words * y * 4),
               size.width / 2);
    }
}

/*
void cgimage_t::imp_draw(cgimage_t *image, cgimage_t *srcImage, cgpoint_t point) {
    assert(image->get_size().contains(point));
    cgrect_t rect = { { 0, 0 }, srcImage->get_size() };
    imp_draw_rect(image, srcImage, &rect, point);
}
*/

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

#endif

#endif
