//
//  graphics_dirtymap.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-03-18.
//

#include "graphics.hpp"


cgdirtymap_c *cgdirtymap_c::create(const cgimage_c &image) {
    auto size = image.get_size();
    size.width /= CGDIRTYMAP_TILE_WIDTH;
    size.height /= CGDIRTYMAP_TILE_HEIGHT;
    int bytes = sizeof(cgdirtymap_c) + size.width * size.height;
    return new (calloc(1, bytes)) cgdirtymap_c(size);
}

void cgdirtymap_c::mark(const cgrect_t &rect) {
    const int x1 = rect.origin.x / CGDIRTYMAP_TILE_WIDTH;
    const int x2 = (rect.origin.x + rect.size.width - 1) / CGDIRTYMAP_TILE_WIDTH;
    const int y1 = rect.origin.y / CGDIRTYMAP_TILE_HEIGHT;
    const int y2 = (rect.origin.y + rect.size.height - 1) / CGDIRTYMAP_TILE_HEIGHT;
    for (int y = y1; y <= y2; y++) {
        const int line_offset = y * _size.width;
        for (int x = x1; x <= x2; x++) {
            _data[x + line_offset] = true;
        }
    }
}

void cgdirtymap_c::merge(const cgdirtymap_c &dirtymap) {
    assert(_size == dirtymap._size);
    int count = _size.width * _size.height;
    int l_count = count / 4;
    if (l_count * 4 == count) {
        uint32_t *l_dest = (uint32_t*)_data;
        const uint32_t *l_source = (uint32_t*)dirtymap._data;
        while (--l_count != -1) {
            uint32_t v = *l_source++;
            if (v) {
                *l_dest++ |= v;
            } else {
                l_dest++;
            }
        }
    } else {
        // slow path
        uint8_t *dest = _data;
        const uint8_t *source = dirtymap._data;
        while (--count != -1) {
            *dest++ |= *source++;
        }
    }
}

void cgdirtymap_c::restore(cgimage_c &image, const cgimage_c &clean_image) {
    assert(image.get_size() == clean_image.get_size());
    assert(_size.width * CGDIRTYMAP_TILE_WIDTH == clean_image.get_size().width);
    assert(_size.height * CGDIRTYMAP_TILE_HEIGHT == clean_image.get_size().height);
    assert((image.get_size().width % CGDIRTYMAP_TILE_WIDTH) == 0);
    assert((image.get_size().height % CGDIRTYMAP_TILE_HEIGHT) == 0);
    const_cast<cgimage_c&>(image).with_clipping(false, [this, &image, &clean_image] {
        const auto image_size = image.get_size();
        int16_t y = image_size.height - CGDIRTYMAP_TILE_HEIGHT;
        for (int row = _size.height; --row != -1; y -= CGDIRTYMAP_TILE_HEIGHT) {
            const int row_offset = row * _size.width;
            int16_t x = image_size.width - CGDIRTYMAP_TILE_WIDTH;
            for (int col = _size.width; --col != -1; x -= CGDIRTYMAP_TILE_WIDTH) {
                if (_data[col + row_offset]) {
                    _data[col + row_offset] = false;
                    cgpoint_t at = (cgpoint_t){x, y};
                    cgrect_t rect = (cgrect_t){at, {CGDIRTYMAP_TILE_WIDTH, CGDIRTYMAP_TILE_HEIGHT}};
                    image.draw_aligned(clean_image, rect, at);
                }
            }
        }
    });
}

void cgdirtymap_c::clear() {
    memset(_data, 0, _size.width * _size.height);
}

#ifndef __M68000__
#if DEBUG_DIRTYMAP
void cgdirtymap_c::debug(const char *name) const {
    const int row_count = _size.height;
    printf("Dirtymap %d columns [%s]\n", _size.width, name);
    for (int row = 0; row < row_count; row++) {
        char buf[_size.width + 1];
        const int row_offset = row * _size.width;
        for (int col = 0; col < _size.width; col++) {
            buf[col] = _data[col + row_offset] ? 'X' : '-';
        }
        buf[_size.width] = 0;
        printf("  row %2d: %s\n", row, buf);
    }
}
#endif
#endif
