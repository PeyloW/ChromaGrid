//
//  font.cpp
//  toybox
//
//  Created by Fredrik on 2024-04-11.
//

#include "font.hpp"

using namespace toybox;

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