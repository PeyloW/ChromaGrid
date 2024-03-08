//
//  graphics.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-04.
//

#include "graphics.hpp"
#include "system.hpp"

extern "C" {
#ifndef __M68000__
    const cgpalette_c *cgg_active_palette = nullptr;
#endif
    const cgimage_c *cgg_active_image = nullptr;
}

void cgpalette_c::set_active() const {
#ifdef __M68000__
    memcpy(reinterpret_cast<uint16_t*>(0xffff8240), colors, sizeof(colors));
#else
    cgg_active_palette = this;
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
    cgtimer_c::with_paused_timers([this] {
        cgg_active_image = this;
    });
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
