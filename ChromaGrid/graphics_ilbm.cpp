//
//  graphics_ilbm.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-03-07.
//

#include "graphics.hpp"
#include "iff_file.hpp"

typedef enum __packed {
    mask_type_none,
    mask_type_plane,
    mask_type_color,
    mask_type_lasso,
} mask_type_e;

typedef enum __packed {
    compression_type_none,
    compression_type_packbits,
    compression_type_vertical
} compression_type_e;

struct ilbm_header_t {
    cgsize_t size;
    cgpoint_t offset;
    uint8_t plane_count;
    mask_type_e mask_type;
    compression_type_e compression_type;
    uint8_t _pad;
    uint16_t mask_color;
    uint8_t aspect[2];
    cgsize_t page_size;
};
static_assert(sizeof(ilbm_header_t) == 20, "Heade size mismatch");


#ifndef __M68000__
static void cghton(ilbm_header_t &bmhd) {
    cghton(bmhd.size);
    cghton(bmhd.offset);
    cghton(bmhd.mask_color);
    cghton(bmhd.page_size);
};
#endif

static void cgimage_read(cgiff_file_c &file, uint16_t line_words, int height, uint16_t *bitmap, uint16_t *maskmap) {
    uint16_t word_buffer[line_words];
    const int bp_count = (maskmap ? 5 : 4);
    while (--height != -1) {
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
                for (int i = line_words; --i != -1; ) {
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
    while (--height != -1) {
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
                    bitmap[bp + i * 4] = word_buffer[bp * line_words + i];
                    cghton(bitmap[bp + i * 4]);
                }
            } else {
                for (int i = 0; i < line_words; i++) {
                    maskmap[i] = word_buffer[bp * line_words + i];
                    cghton(bitmap[i]);
                }
            }
        }
        bitmap += line_words * 4;
        if (maskmap) {
            maskmap += line_words;
        }
    }
}

cgimage_c::cgimage_c(const char *path, bool masked, uint8_t masked_cidx) {
    memset(this, 0, sizeof(cgimage_c));

    cgiff_file_c file(path);
    cgiff_group_t header;
    if (!file.find(CGIFF_FORM, cgiff_id_make("ILBM"), header)) {
        hard_assert(0);
        return; // Not a ILBM
    }
    cgiff_chunk_t chunk;
    ilbm_header_t bmhd;
    while (file.read(chunk)) {
        if (cgiff_id_equals(chunk.id, "BMHD")) {
            if (!file.read(bmhd)) {
                return;
            }
            _size = bmhd.size;
            _offset = bmhd.offset;
            assert(bmhd.plane_count == 4);
            assert(bmhd.mask_type < mask_type_lasso); // Lasso not supported
            assert(bmhd.compression_type < compression_type_vertical); // DeluxePain ST format not supported
            if (masked_cidx != MASKED_CIDX && masked) {
                bmhd.mask_color = masked_cidx;
            }
        } else if (cgiff_id_equals(chunk.id, "CMAP")) {
            uint8_t cmpa[48];
            if (!file.read(cmpa, 48)) {
                return; // Could not read palette
            }
            _palette = new cgpalette_c(&cmpa[0]);
        } else if (cgiff_id_equals(chunk.id, "BODY")) {
            _line_words = ((_size.width + 15) / 16);
            const uint16_t bitmap_words = (_line_words * _size.height) << 2;
            const bool needs_mask_words = masked || (bmhd.mask_type == mask_type_plane);
            const uint16_t mask_words = needs_mask_words ? (bitmap_words >> 2) : 0;
            _bitmap = reinterpret_cast<uint16_t*>(malloc((bitmap_words + mask_words) << 1));
            assert(_bitmap);
            if (needs_mask_words) {
                _maskmap = _bitmap + bitmap_words;
            } else {
                _maskmap = nullptr;
            }
            _owns_bitmap = true;
            switch (bmhd.compression_type) {
                case compression_type_none:
                    cgimage_read(file, _line_words, _size.height, _bitmap, bmhd.mask_type == mask_type_plane ? _maskmap : nullptr);
                    break;
                case compression_type_packbits:
                    cgimage_read_packbits(file, _line_words, _size.height, _bitmap, bmhd.mask_type == mask_type_plane ? _maskmap : nullptr);
                    break;
                default:
                    break;
            }
            if (needs_mask_words) {
                if (!masked) {
                    _maskmap = nullptr;
                } else if (bmhd.mask_type == mask_type_color) {
                    memset(_maskmap, -1, mask_words << 1);
                    remap_table_t table;
                    make_noremap_table(table);
                    table[masked_cidx] = MASKED_CIDX;
                    remap_colors(table, (cgrect_t){ {0, 0}, _size});
                }
            }
        } else {
            printf("Skipping '%c%c%c%c'\n", (chunk.id >> 24) & 0xff, (chunk.id >> 16) & 0xff, (chunk.id >> 8) & 0xff, chunk.id & 0xff);
            file.skip(chunk);
        }
        file.align();
    }
}
