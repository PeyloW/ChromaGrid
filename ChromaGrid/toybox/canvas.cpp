//
//  canvas.cpp
//  toybox
//
//  Created by Fredrik on 2024-04-10.
//

#include "canvas.hpp"
#include "vector.hpp"

using namespace toybox;

canvas_c::canvas_c(image_c &image) :
    _image(image), _dirtymap(nullptr), _stencil(nullptr), _clipping(true)
{}

canvas_c::~canvas_c() {}


dirtymap_c *canvas_c::create_dirtymap() const {
    auto size = _image.get_size();
    int bytes = dirtymap_c::instance_size(&size);
    return new (calloc(1, bytes)) dirtymap_c(size);
}

void canvas_c::put_pixel(uint8_t ci, point_s at) const {
    if (_clipping) {
        if (!_image._size.contains(at)) return;
    }
    assert(_image._size.contains(at));
    int word_offset = (at.x / 16) + at.y * _image._line_words;
    const uint16_t bit = 1 << (15 - at.x & 15);
    const uint16_t mask = ~bit;
    if (_image._maskmap != nullptr) {
        uint16_t *maskmap = _image._maskmap + word_offset;
        if (ci == image_c::MASKED_CIDX) {
            *maskmap &= mask;
        } else {
            *maskmap |= bit;
        }
    } else if (ci < 0) {
        return;
    }
    uint16_t *bitmap = _image._bitmap + (word_offset << 2);
    uint8_t cb = 1;
    for (int bp = 4; --bp != -1; cb <<= 1) {
        if (ci & cb) {
            *bitmap++ |= bit;
        } else {
            *bitmap++ &= mask;
        }
    }
}

void canvas_c::remap_colors(remap_table_t table, rect_s rect) const {
    const_cast<canvas_c*>(this)->with_clipping(false, [this, table, &rect] {
        for (int16_t y = rect.origin.y; y < rect.origin.y + rect.size.height; y++) {
            for (int16_t x = rect.origin.x; x < rect.origin.x + rect.size.width; x++) {
                const point_s at(x, y);
                const uint8_t c = _image.get_pixel(at);
                const uint8_t rc = table[c];
                if (c != rc) {
                    put_pixel(rc, at);
                }
            }
        }
    });
}

void canvas_c::fill(uint8_t ci, rect_s rect) const {
    assert(_image._maskmap == nullptr);
    assert(rect.contained_by(get_size()));
    if (_clipping) {
        if (!rect.clip_to(_image._size, rect.origin)) {
            return;
        }
    }
    if (_dirtymap) {
        _dirtymap->mark(rect);
    }
    imp_fill(ci, rect);
}

void canvas_c::draw_aligned(const image_c &src, point_s at) const {
    assert((at.x & 0xf) == 0);
    assert((src._size.width & 0xf) == 0);
    assert(_image._maskmap == nullptr);
    assert(src._maskmap == nullptr);
    rect_s rect(point_s(), src.get_size());
    draw_aligned(src, rect, at);
}

void canvas_c::draw_aligned(const image_c &src, rect_s rect, point_s at) const {
    assert((at.x & 0xf) == 0);
    assert((rect.origin.x &0xf) == 0);
    assert((rect.size.width & 0xf) == 0);
    assert(_image._maskmap == nullptr);
    assert(src._maskmap == nullptr);
    if (_clipping) {
        if (!rect.clip_to(_image._size, at)) {
            return;
        }
    }
    if (_dirtymap) {
        const rect_s dirty_rect(at, rect.size);
        _dirtymap->mark(dirty_rect);
    }
    imp_draw_aligned(src, rect, at);
}

void canvas_c::draw_aligned(const tileset_c &src, int idx, point_s at) const {
    draw_aligned(*src.get_image(), src.get_rect(idx), at);
}

void canvas_c::draw_aligned(const tileset_c &src, point_s tile, point_s at) const {
    draw_aligned(*src.get_image(), src.get_rect(tile), at);
}

void canvas_c::draw(const image_c &src, point_s at, const uint8_t color) const {
    assert(_image._maskmap == nullptr);
    rect_s rect(point_s(), src.get_size());
    draw(src, rect, at, color);
}

void canvas_c::draw(const image_c &src, rect_s rect, point_s at, const uint8_t color) const {
    assert(_image._maskmap == nullptr);
    assert(rect.contained_by(get_size()));
    if (_clipping) {
        if (!rect.clip_to(_image._size, at)) {
            return;
        }
    }
    if (_dirtymap) {
        const rect_s dirty_rect(at, rect.size);
        _dirtymap->mark(dirty_rect);
    }
    if (src._maskmap) {
        if (color == image_c::MASKED_CIDX) {
            imp_draw_masked(src, rect, at);
        } else {
            imp_draw_color(src, rect, at, color);
        }
    } else {
        assert(color == image_c::MASKED_CIDX);
        imp_draw(src, rect, at);
    }
}

void canvas_c::draw(const tileset_c &src, int idx, point_s at, const uint8_t color) const {
    draw(*src.get_image(), src.get_rect(idx), at, color);
}

void canvas_c::draw(const tileset_c &src, point_s tile, point_s at, const uint8_t color) const {
    draw(*src.get_image(), src.get_rect(tile), at, color);
}

void canvas_c::draw_3_patch(const image_c &src, int16_t cap, rect_s in) const {
    rect_s rect(point_s(), src.get_size());
    draw_3_patch(src, rect, cap, in);
}

void canvas_c::draw_3_patch(const image_c &src, rect_s rect, int16_t cap, rect_s in) const {
    assert(in.size.width >= cap * 2);
    assert(rect.size.width > cap * 2);
    assert(rect.size.height == in.size.height);
    if (_dirtymap) {
        _dirtymap->mark(in);
    }
    const_cast<canvas_c*>(this)->with_dirtymap(nullptr, [&] {
        const rect_s left_rect(rect.origin, size_s(cap, rect.size.height));
        draw(src, left_rect, in.origin);
        const rect_s right_rect(
            rect.origin.x + rect.size.width - cap, rect.origin.y,
            cap, rect.size.height
        );
        const point_s right_at(in.origin.x + in.size.width - cap, in.origin.y);
        draw(src, right_rect, right_at);
        rect_s middle_rect(
            rect.origin.x + cap, rect.origin.y,
            rect.size.width - cap * 2, rect.size.height
        );
        point_s at(in.origin.x + cap, in.origin.y);
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

size_s canvas_c::draw(const font_c &font, const char *text, point_s at, text_alignment_e alignment, const uint8_t color) const {
    int len = (int)strlen(text);
    size_s size = font.get_rect(' ').size;
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
        rect_s dirty_rect(point_s(at.x - size.width, at.y), size);
        _dirtymap->mark(dirty_rect);
    }
    const_cast<canvas_c*>(this)->with_dirtymap(nullptr, [&] {
        for (int i = len; --i != -1; ) {
            const rect_s &rect = font.get_rect(text[i]);
            at.x -= rect.size.width;
            draw(*font.get_image(), rect, at, color);
        }
    });
    return size;
}

#define MAX_LINES 8
static char draw_text_buffer[80 * MAX_LINES];

size_s canvas_c::draw(const font_c &font, const char *text, rect_s in, uint16_t line_spacing, text_alignment_e alignment, const uint8_t color) const {
    strcpy(draw_text_buffer, text);
    vector_c<const char *, 12> lines;

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
    point_s at;
    switch (alignment) {
        case align_left: at = in.origin; break;
        case align_center: at = point_s( in.origin.x + in.size.width / 2, in.origin.y); break;
        case align_right: at = point_s( in.origin.x + in.size.width / 2, in.origin.y); break;
    }
    size_s max_size = {0,0};
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

void image_c::imp_draw_aligned(const image_c &srcImage, point_s point) const {
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

void image_c::imp_draw_rect(const image_c &srcImage, rect_s *const rect, point_s point) const {
    assert(get_size().contains(point));
    for (int y = 0; y < rect->size.height; y++) {
        for (int x = 0; x < rect->size.width; x++) {
            uint8_t color = srcImage.get_pixel(point_s{(int16_t)(rect->origin.x + x), (int16_t)(rect->origin.y + y)});
            if (color != MASKED_CIDX) {
                put_pixel(color, point_s{(int16_t)(point.x + x), (int16_t)(point.y + y)});
            }
        }
    }
}

#endif

#endif
