//
//  graphics_dirtymap.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-03-18.
//

#include "graphics.hpp"

using namespace toybox;

#define CGDIRTYMAP_BITSET (1)

#if CGDIRTYMAP_BITSET
    static const int LOOKUP_SIZE = 256;
    typedef struct bitrun_list_s {
        int16_t num_runs;
        struct bitrun_s {
            int16_t start;
            int16_t length;
        } bit_runs[4];
    } bitrun_list_s;
static bitrun_list_s* lookup_table[LOOKUP_SIZE];
static void init_lookup_table_if_needed() {
    static bitrun_list_s _lookup_buffer[LOOKUP_SIZE];
    static bool is_initialized = false;
    if (is_initialized) {
        return;
    }
    is_initialized = true;
    for (int input = 0; input < LOOKUP_SIZE; input++) {
        bitrun_list_s* result = &_lookup_buffer[input];
        lookup_table[input] = result;
        result->num_runs = 0;
        // Iterate through each bit in the input
        for (int i = 0; i < 8; ) {
            // If the current bit is 1, start a new run
            if ((input >> i) & 1) {
                int16_t start = i * CGDIRTYMAP_TILE_WIDTH;
                int16_t length = CGDIRTYMAP_TILE_WIDTH;
                result->bit_runs[result->num_runs].start = start;
                while ((input >> (i + 1)) & 1) {
                    length += CGDIRTYMAP_TILE_WIDTH;
                    i++;
                }
                result->bit_runs[result->num_runs].length = length;
                result->num_runs++;
            }
            i++;
        }
    }
}
#endif

dirtymap_c *dirtymap_c::create(const image_c &image) {
    auto size = image.get_size();
#if CGDIRTYMAP_BITSET
    init_lookup_table_if_needed();
    size.width = (size.width + (8 * CGDIRTYMAP_TILE_WIDTH - 1)) / (8 * CGDIRTYMAP_TILE_WIDTH);
    size.height /= CGDIRTYMAP_TILE_HEIGHT;
    const int data_size = size.width * (size.height + 1) + 3;
#else
    size.width /= CGDIRTYMAP_TILE_WIDTH;
    size.height /= CGDIRTYMAP_TILE_HEIGHT;
    const int data_size = size.width * size.height;
#endif
    return new (calloc(1, sizeof(dirtymap_c) + data_size)) dirtymap_c(size);
}

void dirtymap_c::mark(const rect_s &rect) {
    const int x1 = rect.origin.x / CGDIRTYMAP_TILE_WIDTH;
    const int x2 = (rect.origin.x + rect.size.width - 1) / CGDIRTYMAP_TILE_WIDTH;
    const int y1 = rect.origin.y / CGDIRTYMAP_TILE_HEIGHT;
    assert(y1 < _size.height);
#if CGDIRTYMAP_BITSET
#define BITS_PER_BYTE 8
    static const uint8_t first_byte_masks[BITS_PER_BYTE] = {
        0xFF, 0xFE, 0xFC, 0xF8, 0xF0, 0xE0, 0xC0, 0x80
    };
    static const uint8_t last_byte_masks[BITS_PER_BYTE] = {
        0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF
    };

    const int extra_rows = ((rect.origin.y + rect.size.height - 1) / CGDIRTYMAP_TILE_HEIGHT - y1);
    assert(y1 + extra_rows < _size.height);
    const int start_byte = x1 / BITS_PER_BYTE;
    assert(start_byte < _size.width);
    const int end_byte = x2 / BITS_PER_BYTE;
    assert(end_byte < _size.width);
    const int start_bit = x1 % BITS_PER_BYTE;
    assert(start_bit < 8);
    const int end_bit = x2 % BITS_PER_BYTE;
    assert(end_bit < 8);
    
    uint8_t *data = _data + (start_byte + _size.width * y1);

    if (extra_rows == 0) {
        if (start_byte == end_byte) {
            *data |= (first_byte_masks[start_bit] & last_byte_masks[end_bit]);
        } else {
            *data++ |= (first_byte_masks[start_bit]);
            for (int j = end_byte - start_byte - 1; --j != -1; ) {
                *data++ = 0xFF;
            }
            *data |= (last_byte_masks[end_bit]);
        }
    } else {
        if (start_byte == end_byte) {
            auto mask = (first_byte_masks[start_bit] & last_byte_masks[end_bit]);
            for (int y = 0; y <= extra_rows; y++) {
                *data |= mask;
                data += _size.width;
            }
        } else {
            auto mask_first = (first_byte_masks[start_bit]);
            auto mask_last = (last_byte_masks[end_bit]);
            for (int y = 0; y <= extra_rows; y++) {
                auto line_data = data;
                *line_data++ |= mask_first;
                for (int j = end_byte - start_byte - 1; --j != -1; ) {
                    *line_data++ = 0xFF;
                }
                *line_data |= mask_last;
                data += _size.width;
            }
        }
    }
#else
    const int y2 = (rect.origin.y + rect.size.height - 1) / CGDIRTYMAP_TILE_HEIGHT;
    for (int y = y1; y <= y2; y++) {
        const int line_offset = y * _size.width;
        for (int x = x1; x <= x2; x++) {
            _data[x + line_offset] = true;
        }
    }
#endif
}

void dirtymap_c::merge(const dirtymap_c &dirtymap) {
#if CGDIRTYMAP_BITSET
    uint32_t *l_dest = (uint32_t*)_data;
    const uint32_t *l_source = (uint32_t*)dirtymap._data;
    int long_count = (_size.width * _size.height + 3) / 4;
    do {
        const uint32_t v = *l_source++;
        if (v) {
            *l_dest++ |= v;
        } else {
            l_dest++;
        }
    } while (--long_count != -1);
#else
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
#endif
}

void dirtymap_c::restore(image_c &image, const image_c &clean_image) {
    assert(image.get_size() == clean_image.get_size());
#if CGDIRTYMAP_BITSET
    assert(_size.width * CGDIRTYMAP_TILE_WIDTH * 8 >= clean_image.get_size().width);
#else
    assert(_size.width * CGDIRTYMAP_TILE_WIDTH == clean_image.get_size().width);
#endif
    assert(_size.height * CGDIRTYMAP_TILE_HEIGHT == clean_image.get_size().height);
    assert((image.get_size().width % CGDIRTYMAP_TILE_WIDTH) == 0);
    assert((image.get_size().height % CGDIRTYMAP_TILE_HEIGHT) == 0);
    const_cast<image_c&>(image).with_clipping(false, [this, &image, &clean_image] {
#if CGDIRTYMAP_BITSET
        auto data = _data;
        point_s at = {0, 0};
        for (int row = _size.height; --row != -1; ) {
            for (int col = _size.width; --col != -1; ) {
                const auto byte = *data;
                if (byte) {
                    const int16_t height = [&] {
                        int height = 0;
                        while (data[height * _size.width] == byte) {
                            data[height * _size.width] = 0;
                            height++;
                        }
                        return height * CGDIRTYMAP_TILE_HEIGHT;
                    }();
                    auto bitrunlist = lookup_table[byte];
                    int16_t *bitrun = (int16_t*)bitrunlist->bit_runs;
                    for (int r = bitrunlist->num_runs; --r != -1; ) {
                        rect_s rect = (rect_s){
                            (point_s){ (int16_t)(at.x + *bitrun++), at.y},
                            (size_s){ *bitrun++, height}
                        };
                        bitrun++;
                        #if DEBUG_DIRTYMAP
                        printf("Restore {{%d, %d}, {%d, %d}}", rect.origin.x, rect.origin.y, rect.size.width, rect.size.height);
                        #endif
                        image.draw_aligned(clean_image, rect, rect.origin);
                    }
                }
                data++;
                at.x += CGDIRTYMAP_TILE_WIDTH * 8;
            }
            at.x = 0;
            at.y += CGDIRTYMAP_TILE_HEIGHT;
        }
#else
        const auto image_size = image.get_size();
        int16_t y = image_size.height - CGDIRTYMAP_TILE_HEIGHT;
        for (int row = _size.height; --row != -1; y -= CGDIRTYMAP_TILE_HEIGHT) {
            const int row_offset = row * _size.width;
            int16_t x = image_size.width - CGDIRTYMAP_TILE_WIDTH;
            for (int col = _size.width; --col != -1; x -= CGDIRTYMAP_TILE_WIDTH) {
                if (_data[col + row_offset]) {
                    _data[col + row_offset] = false;
                    point_s at = (point_s){x, y};
                    rect_s rect = (rect_s){at, {CGDIRTYMAP_TILE_WIDTH, CGDIRTYMAP_TILE_HEIGHT}};
                    image.draw_aligned(clean_image, rect, at);
                }
            }
        }
#endif
    });
}

void dirtymap_c::clear() {
    memset(_data, 0, _size.width * _size.height);
}

#ifndef __M68000__
#if DEBUG_DIRTYMAP
void cgdirtymap_c::debug(const char *name) const {
    printf("Dirtymap %d columns [%s]\n", _size.width, name);
#if CGDIRTYMAP_BITSET
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
    ((byte) & 0x01 ? '1' : '0'), \
    ((byte) & 0x02 ? '1' : '0'), \
    ((byte) & 0x04 ? '1' : '0'), \
    ((byte) & 0x08 ? '1' : '0'), \
    ((byte) & 0x10 ? '1' : '0'), \
    ((byte) & 0x20 ? '1' : '0'), \
    ((byte) & 0x40 ? '1' : '0'), \
    ((byte) & 0x80 ? '1' : '0')
    auto data = _data;
    for (int row = _size.height; --row != -1; ) {
        for (int col = _size.width; --col != -1; ) {
            const auto byte = *data++;
            printf(BYTE_TO_BINARY_PATTERN,  BYTE_TO_BINARY(byte));
        }
        printf("\n");
    }
#else
    for (int row = 0; row < _size.height; row++) {
        char buf[_size.width + 1];
        const int row_offset = row * _size.width;
        for (int col = 0; col < _size.width; col++) {
            buf[col] = _data[col + row_offset] ? 'X' : '-';
        }
        buf[_size.width] = 0;
        printf("  row %2d: %s\n", row, buf);
    }
#endif
}
#endif
#endif
