//
//  graphics_ilbm.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-03-07.
//

#include "graphics.hpp"
#include "iff_file.hpp"

using namespace toybox;

CGDEFINE_ID(ILBM);
CGDEFINE_ID(BMHD);
CGDEFINE_ID(CMAP);
CGDEFINE_ID(GRAB);
CGDEFINE_ID(BODY);

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

struct __packed_struct ilbm_header_t {
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
    cgiff_group_t form;
    if (!file.first(CGIFF_FORM, CGIFF_ILBM, form)) {
        hard_assert(0);
        return; // Not a ILBM
    }
    cgiff_chunk_t chunk;
    ilbm_header_t bmhd;
    while (file.next(form, "*", chunk)) {
        if (cgiff_id_match(chunk.id, CGIFF_BMHD)) {
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
        } else if (cgiff_id_match(chunk.id, CGIFF_CMAP)) {
            uint8_t cmpa[48];
            if (!file.read(cmpa, 48)) {
                return; // Could not read palette
            }
            _palette = new cgpalette_c(&cmpa[0]);
        } else if (cgiff_id_match(chunk.id, CGIFF_BODY)) {
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
#ifndef __M68000__
            printf("Skipping '%c%c%c%c'\n", (chunk.id >> 24) & 0xff, (chunk.id >> 16) & 0xff, (chunk.id >> 8) & 0xff, chunk.id & 0xff);
#endif
            file.skip(chunk);
        }
    }
}


#ifdef CGIMAGE_SUPPORT_SAVE


static int cgimage_packbits_into_body(uint8_t *body, const uint8_t *row_buffer, int row_byte_count) {
    assert(row_byte_count >= 2);
#define PACKBITS_MIN_RUN 3
#define PACKBITS_MAX_BYTES 128
    typedef enum {
        mode_dump,
        mode_run
    } state_e;
    
    const auto packRunIntoBody = [&body] (uint8_t byte, int count) {
        *body++ = -(count - 1);
        *body++ = byte;
    };
    const auto packDumpIntoBody = [&body] (uint8_t *buf, int count) {
        *body++ = count - 1;
        memcpy(body, buf, count);
        body += count;
    };
    
    uint8_t *const body_begin = body;
    uint8_t current_byte, previous_byte = '\0';
    static uint8_t buffer[256];
    short buffer_byte_count;
    short run_start_buffer_index = 0;
    state_e mode = mode_dump;
    
    buffer[0] = previous_byte = current_byte = *row_buffer++;
    buffer_byte_count = 1;
    row_byte_count--;
    
    for (; row_byte_count > 0;  --row_byte_count) {
        buffer[buffer_byte_count++] = current_byte = *row_buffer++;
        switch (mode) {
            case mode_dump:
                if (buffer_byte_count > PACKBITS_MAX_BYTES) {
                    packDumpIntoBody(buffer, buffer_byte_count - 1);
                    buffer[0] = current_byte;
                    buffer_byte_count = 1;
                    run_start_buffer_index = 0;
                    break;
                }
                if (current_byte == previous_byte) {
                    if (buffer_byte_count - run_start_buffer_index >= PACKBITS_MIN_RUN) {
                        if (run_start_buffer_index > 0) {
                            packDumpIntoBody(buffer, run_start_buffer_index);
                        }
                        mode = mode_run;
                    }  else if (run_start_buffer_index == 0) {
                        mode = mode_run;
                    }
                } else {
                    run_start_buffer_index = buffer_byte_count - 1;
                }
                break;
                
            case mode_run:
                if ((current_byte != previous_byte) || (buffer_byte_count - run_start_buffer_index > PACKBITS_MAX_BYTES)) {
                    packRunIntoBody(previous_byte, buffer_byte_count - 1 - run_start_buffer_index);
                    buffer[0] = current_byte;
                    buffer_byte_count = 1;
                    run_start_buffer_index = 0;
                    mode = mode_dump;
                }
                break;
        }
        previous_byte = current_byte;
    }
    
    switch (mode) {
        case mode_dump:
            packDumpIntoBody(buffer, buffer_byte_count);
            break;
        case mode_run:
            packRunIntoBody(previous_byte, buffer_byte_count-run_start_buffer_index);
            break;
    }
    return (int)(body - body_begin);
}

static void cgimage_write_packbits(cgiff_file_c &file, uint16_t line_words, uint16_t next_line_words, int height, uint16_t *bitmap, uint16_t *maskmap) {
    const int bp_count = (maskmap ? 5 : 4);
    uint16_t word_buffer[line_words * bp_count];
    while (--height != -1) {
        for (int bp = 0; bp < bp_count; bp++) {
            if (bp < 4) {
                for (int i = 0; i < line_words; i++) {
                    word_buffer[bp * line_words + i] = bitmap[bp + i * 4];
                    cghton(word_buffer[bp * line_words + i]);
                }
            } else {
                for (int i = 0; i < line_words; i++) {
                    word_buffer[bp * line_words + i] = maskmap[i];
                    cghton(word_buffer[bp * line_words + i]);
                }
            }
        }
        uint8_t body[line_words * bp_count * 2 + 32];
        int bytes = cgimage_packbits_into_body(body, (const uint8_t *)word_buffer, line_words * bp_count * 2);
        file.write(body, 1, bytes);
        
        bitmap += next_line_words * 4;
        if (maskmap) {
            maskmap += next_line_words;
        }
    }
}

static void cgimage_write(cgiff_file_c &file, uint16_t line_words, uint16_t next_line_words, int height, uint16_t *bitmap, uint16_t *maskmap) {
    const int bp_count = (maskmap ? 5 : 4);
    uint16_t word_buffer[line_words * bp_count];
    while (--height != -1) {
        for (int bp = 0; bp < bp_count; bp++) {
            if (bp < 4) {
                for (int i = 0; i < line_words; i++) {
                    word_buffer[bp * line_words + i] = bitmap[bp + i * 4];
                }
            } else {
                for (int i = 0; i < line_words; i++) {
                    word_buffer[bp * line_words + i] = maskmap[i];
                }
            }
        }
        file.write(word_buffer, line_words * bp_count);
        
        bitmap += next_line_words * 4;
        if (maskmap) {
            maskmap += next_line_words;
        }
    }
}


bool cgimage_c::save(const char *path, bool compressed, bool masked, uint8_t masked_cidx) {
    cgiff_file_c ilbm(path, "w+");
    if (ilbm.get_pos() >= 0) {
        ilbm.with_hard_asserts(true, [&] {
            cgiff_group_t form;
            cgiff_chunk_t chunk;
            ilbm_header_t header;
            ilbm.begin(form, CGIFF_FORM);
            ilbm.write(CGIFF_ILBM_ID);
            {
                memset(&header, 0, sizeof(ilbm_header_t));
                header.size = _size;
                header.plane_count = 4;
                header.mask_type = masked_cidx != MASKED_CIDX ? mask_type_color : (masked && _maskmap) ? mask_type_plane : mask_type_none;
                header.compression_type = compressed ? compression_type_packbits : compression_type_none;
                if (header.mask_type == mask_type_color) {
                    header.mask_color = masked_cidx;
                }
                header.aspect[0] = 10;
                header.aspect[0] = 11;
                header.page_size = {320, 200};
                ilbm.begin(chunk, CGIFF_BMHD);
                ilbm.write(header);
                ilbm.end(chunk);
            }
            if (_offset.x != 0 || _offset.y != 0) {
                ilbm.begin(chunk, CGIFF_GRAB);
                ilbm.write(_offset);
                ilbm.end(chunk);
            }
            if (_palette) {
                uint8_t cmap[48];
                for (int i = 0; i < 16; i++) {
                    _palette->colors[i].get(&cmap[i * 3 + 0], &cmap[i * 3 + 1], &cmap[i * 3 + 2]);
                }
                ilbm.begin(chunk, CGIFF_CMAP);
                ilbm.write(cmap, 1, 48);
                ilbm.end(chunk);
            }
            {
                ilbm.begin(chunk, CGIFF_BODY);
                if (compressed) {
                    cgimage_write_packbits(ilbm, (_size.width + 15) / 16, _line_words, _size.height, _bitmap,  header.mask_type == mask_type_plane ? _maskmap : nullptr);
                } else {
                    cgimage_write(ilbm, (_size.width + 15) / 16, _line_words, _size.height, _bitmap,  header.mask_type == mask_type_plane ? _maskmap : nullptr);
                }
                ilbm.end(chunk);
            }
            ilbm.end(form);
        });
    }
    return false;
}
#endif