//
//  graphics.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-04.
//

#include "graphics.hpp"
#include "audio.hpp"
#include "system.hpp"

using namespace toybox;

extern "C" {
#ifndef __M68000__
    const sound_c *g_active_sound = nullptr;
#endif
    const palette_c *g_active_palette = nullptr;
    const image_c *g_active_image = nullptr;
}

color_c color_c::mix(color_c other, int shade) const {
    assert(shade >= MIX_FULLY_THIS && shade <= MIX_FULLY_OTHER);
    int r = from_ste(color, 8) * (MIX_FULLY_OTHER - shade) + from_ste(other.color, 8) * shade;
    int g = from_ste(color, 4) * (MIX_FULLY_OTHER - shade) + from_ste(other.color, 4) * shade;
    int b = from_ste(color, 0) * (MIX_FULLY_OTHER - shade) + from_ste(other.color, 0) * shade;
    color_c mixed(r / MIX_FULLY_OTHER, g / MIX_FULLY_OTHER, b / MIX_FULLY_OTHER);
    return mixed;
}

void palette_c::set_active() const {
#ifdef __M68000__
    memcpy(reinterpret_cast<uint16_t*>(0xffff8240), colors, sizeof(colors));
#endif
    g_active_palette = this;
}

image_c::image_c(const size_s size, bool masked, palette_c *palette) {
    memset(this, 0, sizeof(image_c));
    _palette = palette;
    this->_size = size;
    _line_words = ((size.width + 15) / 16);
    uint16_t bitmap_words = (_line_words * size.height) << 2;
    uint16_t mask_bytes = masked ? (_line_words * size.height) : 0;
    _bitmap = reinterpret_cast<uint16_t*>(calloc(bitmap_words + mask_bytes, 2));
    if (masked) {
        _maskmap = _bitmap + bitmap_words;
    }
    _owns_bitmap = true;
}

image_c::image_c(const image_c &image, rect_s rect) {
    assert((rect.origin.x % 0xf) == 0);
    memcpy(this, &image, sizeof(image_c));
    _size = rect.size;
    int word_offset = image._line_words * rect.origin.y + rect.origin.x / 16;
    _bitmap += word_offset * 4;
    if (_maskmap) {
        _maskmap += word_offset;
    }
    _owns_bitmap = false;
}

image_c::~image_c() {
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

void image_c::set_active() const {
    timer_c::with_paused_timers([this] {
        g_active_image = this;
    });
}

font_c::font_c(const image_c &image, size_s character_size) : _image(image) {
    const int cols = image.get_size().width / character_size.width;
    for (int i = 0; i < 96; i++) {
        const int col = i % cols;
        const int row = i / cols;
        _rects[i] = (rect_s){{(int16_t)(col * character_size.width), (int16_t)(row * character_size.height)}, character_size };
    }
}

font_c::font_c(const image_c &image, size_s max_size, uint8_t space_width, uint8_t lead_req_space, uint8_t trail_req_space) : _image(image)  {
    const int cols = image.get_size().width / max_size.width;
    for (int i = 0; i < 96; i++) {
        const int col = i % cols;
        const int row = i / cols;
        rect_s rect = (rect_s){{(int16_t)(col * max_size.width), (int16_t)(row * max_size.height)}, max_size };
        if (i == 0) {
            rect.size.width = space_width;
        } else {
            // Find first non-empty column, count spaces from top
            {
                for (int fc = 0; fc < rect.size.width; fc++) {
                    for (int fcc = 0; fcc < rect.size.height; fcc++) {
                        const point_s at = (point_s){ (int16_t)(rect.origin.x + fc), (int16_t)(rect.origin.y + fcc)};
                        if (image.get_pixel(at) != image_c::MASKED_CIDX) {
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
                const point_s max_at = (point_s){ (int16_t)(rect.max_x()), (int16_t)(rect.max_y())};
                for (int fc = 0; fc < rect.size.width; fc++) {
                    for (int fcc = 0; fcc < rect.size.height; fcc++) {
                        const point_s at = (point_s){ (int16_t)(max_at.x - fc), (int16_t)(max_at.y - fcc)};
                        if (image.get_pixel(at) != image_c::MASKED_CIDX) {
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
