//
//  iff_file.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-04.
//

#include "iff_file.hpp"


cgiff_file::cgiff_file(FILE *file) : file(file), owns_file(false) { assert(file); };
cgiff_file::cgiff_file(const char *path) : file(fopen(path, "r")), owns_file(true) { assert(file); }
cgiff_file::~cgiff_file() {
    if (owns_file) {
        fclose(file);
    }
}

bool cgiff_file::read(cgiff_header_t *header) {
    if (fread(header, sizeof(cgiff_header_t), 1, file) == 1) {
#ifndef __M68000__
        header->chunk.size = cg_htons(header->chunk.size);
#endif
        return true;
    }
    return false;
}

bool cgiff_file::find(const char *id, cgiff_chunk_t *chunk) {
    assert(strlen(id) == 4);
    const cgiff_id_t *pid = reinterpret_cast<const cgiff_id_t*>(id);
    return find(*pid, chunk);
}

bool cgiff_file::find(const cgiff_id_t id, cgiff_chunk_t *chunk) {
    if (fseek(file, sizeof(cgiff_header_t), SEEK_SET) == 0) {
        while (read(chunk)) {
            if (chunk->id.value == id.value) {
                return true;
            }
            size_t size = (chunk->size + 1) & 0xfffffffe;
            if (fseek(file, size, SEEK_CUR) != 0) {
                break;
            }
        }
    }
    return false;
}

bool cgiff_file::read(cgiff_chunk_t *chunk) {
    if (fread(chunk, sizeof(cgiff_chunk_t), 1, file) == 1) {
#ifndef __M68000__
        chunk->size = cg_htons(chunk->size);
#endif
        return true;
    }
    return false;
}

#ifndef __M68000__
template<size_t S>
static inline void swap(uint8_t bytes[S]) {
    for (int i = 0; i < S / 2; i++) {
        uint8_t t = bytes[i];
        bytes[i] = bytes[S - (i + 1)];
        bytes[S - (i + 1)] = t;
    }
}

uint16_t cg_htons(uint16_t v) {
    union {
        uint8_t bytes[2];
        uint16_t val;
    };
    val = v;
    swap<2>(bytes);
    return val;
}
uint32_t cg_htons(uint32_t v) {
    union {
        uint8_t bytes[4];
        uint32_t val;
    };
    val = v;
    swap<4>(bytes);
    return val;
}

#endif

bool cgiff_file::read(void *data, size_t s, size_t n) {
    bool r = fread(data, s, n, file) == n;
#ifndef __M68000__
    for (int i = 0; i < n; i++) {
        uint8_t *ptr = reinterpret_cast<uint8_t*>(data) + s * i;
        switch (s) {
            case 1:
                break;
            case 2:
                swap<2>(ptr);
                break;
            case 4:
                swap<4>(ptr);
                break;
            case 8:
                swap<8>(ptr);
                break;
            default:
                assert(0);
        }
    }
#endif
    return r;
}