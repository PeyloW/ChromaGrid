//
//  graphics_draw.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-23.
//

#include "graphics.hpp"
#include "vector.hpp"

using namespace toybox;

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
    assert(_maskmap == nullptr);
    assert(rect.contained_by(get_size()));
    if (_clipping) {
        if (!rect.clip_to(_size, rect.origin)) {
            return;
        }
    }
    if (_dirtymap) {
        _dirtymap->mark(rect);
    }
    imp_fill(ci, rect);
}

void cgimage_c::draw_aligned(const cgimage_c &src, cgpoint_t at) const {
    assert((at.x & 0xf) == 0);
    assert(src._offset.x == 0);
    assert(src._offset.y == 0);
    assert((src._size.width & 0xf) == 0);
    assert(_maskmap == nullptr);
    assert(src._maskmap == nullptr);
    cgrect_t rect = (cgrect_t){ {0, 0}, src.get_size()};
    draw_aligned(src, rect, at);
}

void cgimage_c::draw_aligned(const cgimage_c &src, cgrect_t rect, cgpoint_t at) const {
    assert((at.x & 0xf) == 0);
    assert((rect.origin.x &0xf) == 0);
    assert((rect.size.width & 0xf) == 0);
    assert(_maskmap == nullptr);
    assert(src._maskmap == nullptr);
    if (_clipping) {
        if (!rect.clip_to(_size, at)) {
            return;
        }
    }
    if (_dirtymap) {
        const cgrect_t dirty_rect = (cgrect_t){ at, rect.size };
        _dirtymap->mark(dirty_rect);
    }
    imp_draw_aligned(src, rect, at);
}

void cgimage_c::draw(const cgimage_c &src, cgpoint_t at, const uint8_t color) const {
    assert(_maskmap == nullptr);
    const auto offset = src.get_offset();
    const cgpoint_t real_at = (cgpoint_t){ (int16_t)(at.x - offset.x), (int16_t)(at.y - offset.y)};
    cgrect_t rect = (cgrect_t){ {0, 0}, src.get_size()};
    draw(src, rect, real_at, color);
}

void cgimage_c::draw(const cgimage_c &src, cgrect_t rect, cgpoint_t at, const uint8_t color) const {
    assert(_maskmap == nullptr);
    assert(rect.contained_by(get_size()));
    if (_clipping) {
        if (!rect.clip_to(_size, at)) {
            return;
        }
    }
    if (_dirtymap) {
        const cgrect_t dirty_rect = (cgrect_t){at, rect.size};
        _dirtymap->mark(dirty_rect);
    }
    if (src._maskmap) {
        if (color == MASKED_CIDX) {
            imp_draw_masked(src, rect, at);
        } else {
            imp_draw_color(src, rect, at, color);
        }
    } else {
        assert(color == MASKED_CIDX);
        imp_draw(src, rect, at);
    }
}

void cgimage_c::draw_3_patch(const cgimage_c &src, int16_t cap, cgrect_t in) const {
    cgrect_t rect = (cgrect_t){{0, 0}, src.get_size()};
    draw_3_patch(src, rect, cap, in);
}

void cgimage_c::draw_3_patch(const cgimage_c &src, cgrect_t rect, int16_t cap, cgrect_t in) const {
    assert(in.size.width >= cap * 2);
    assert(rect.size.width > cap * 2);
    assert(rect.size.height == in.size.height);
    if (_dirtymap) {
        _dirtymap->mark(in);
    }
    const_cast<cgimage_c*>(this)->with_dirtymap(nullptr, [&] {
        const cgrect_t left_rect = (cgrect_t){ rect.origin, {cap, rect.size.height}};
        draw(src, left_rect, in.origin);
        const cgrect_t right_rect = (cgrect_t){{(int16_t)(rect.origin.x + rect.size.width - cap), rect.origin.y}, {cap, rect.size.height}};
        const cgpoint_t right_at = (cgpoint_t){(int16_t)(in.origin.x + in.size.width - cap), in.origin.y};
        draw(src, right_rect, right_at);
        cgrect_t middle_rect = (cgrect_t){{(int16_t)(rect.origin.x + cap), rect.origin.y}, {(int16_t)(rect.size.width - cap * 2), rect.size.height} };
        cgpoint_t at = (cgpoint_t){(int16_t)(in.origin.x + cap), in.origin.y };
        int16_t to_draw = in.size.width - cap * 2;
        while (to_draw > 0) {
            const int16_t width = MIN(to_draw, middle_rect.size.width);
            middle_rect.size.width = width;
            draw(src, middle_rect, at);
            to_draw -= width;
            at.x += width;
        }
    });
}

cgsize_t cgimage_c::draw(const cgfont_c &font, const char *text, cgpoint_t at, text_alignment_e alignment, const uint8_t color) const {
    int len = (int)strlen(text);
    cgsize_t size = font.get_rect(' ').size;
    size.width = 0;
    for (int i = len; --i != -1; ) {
        size.width += font.get_rect(text[i]).size.width;
    }
    switch (alignment) {
        case align_left:
            at.x += size.width;
            break;
        case align_center:
            at.x += size.width / 2;
            break;
        default:
            break;
    }
    if (_dirtymap) {
        cgrect_t dirty_rect = (cgrect_t){{(int16_t)(at.x - size.width), at.y}, size};
        _dirtymap->mark(dirty_rect);
    }
    const_cast<cgimage_c*>(this)->with_dirtymap(nullptr, [&] {
        for (int i = len; --i != -1; ) {
            const cgrect_t &rect = font.get_rect(text[i]);
            at.x -= rect.size.width;
            draw(font.get_image(), rect, at, color);
        }
    });
    return size;
}

#define MAX_LINES 8
static char draw_text_buffer[80 * MAX_LINES];

cgsize_t cgimage_c::draw(const cgfont_c &font, const char *text, cgrect_t in, uint16_t line_spacing, text_alignment_e alignment, const uint8_t color) const {
    strcpy(draw_text_buffer, text);
    cgvector_c<const char *, 12> lines;

    uint16_t line_width = 0;
    int start = 0;
    int last_good_pos = 0;
    bool done = false;
    for (int i = 0; !done; i++) {
        bool emit = false;
        const char c = text[i];
        if (c == 0) {
            last_good_pos = i;
            emit = true;
            done = true;
        } else if (c == ' ') {
            last_good_pos = i;
        } else if (c == '\n') {
            last_good_pos = i;
            emit = true;
        }
        if (!emit) {
            line_width += font.get_rect(text[i]).size.width;
            if (line_width  > in.size.width) {
                emit = true;
            }
        }
        
        if (emit) {
            draw_text_buffer[last_good_pos] = 0;
            lines.push_back(draw_text_buffer + start);
            line_width = 0;
            start = last_good_pos + 1;
            i = start;
        }
    }
    cgpoint_t at;
    switch (alignment) {
        case align_left: at = in.origin; break;
        case align_center: at = (cgpoint_t){(int16_t)( in.origin.x + in.size.width / 2), in.origin.y}; break;
        case align_right: at = (cgpoint_t){(int16_t)( in.origin.x + in.size.width / 2), in.origin.y}; break;
    }
    cgsize_t max_size = {0,0};
    bool first = true;
    for (auto line = lines.begin(); line != lines.end(); line++) {
        const auto size = draw(font, *line, at, alignment, color);
        at.y += size.height + line_spacing;
        max_size.width = MAX(max_size.width, size.width);
        max_size.height += size.height + (!first ? line_spacing : 0);
        first = false;
    }
    return  max_size;
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