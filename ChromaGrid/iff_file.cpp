//
//  iff_file.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-04.
//

#include "iff_file.hpp"


cgiff_file_c::cgiff_file_c(FILE *file) : _file(file), _owns_file(false) { hard_assert(file); };
cgiff_file_c::cgiff_file_c(const char *path) : _file(fopen(path, "r")), _owns_file(true) { hard_assert(_file); }
cgiff_file_c::~cgiff_file_c() {
    if (_owns_file) {
        fclose(_file);
    }
}

bool cgiff_file_c::read(cgiff_header_t *header) {
    size_t read = fread(header, sizeof(cgiff_header_t), 1, _file);
#ifndef __M68000__
    if (read == 1) {
        header->chunk.size = cg_htons(header->chunk.size);
#else
    if (read == sizeof(cgiff_header_t)) {
#endif
        return true;
    }
    printf("read: %d\n\r", (int)read);
    return false;
}

bool cgiff_file_c::find(const cgiff_id_t id, cgiff_chunk_t *chunk) {
    if (fseek(_file, sizeof(cgiff_header_t), SEEK_SET) == 0) {
        while (read(chunk)) {
            if (chunk->id == id) {
                return true;
            }
            size_t size = (chunk->size + 1) & 0xfffffffe;
            if (fseek(_file, size, SEEK_CUR) != 0) {
                break;
            }
        }
    }
    return false;
}

bool cgiff_file_c::read(cgiff_chunk_t *chunk) {
    size_t read = fread(chunk, sizeof(cgiff_chunk_t), 1, _file);
#ifndef __M68000__
    if (read == 1) {
        chunk->size = cg_htons(chunk->size);
#else
    if (read == sizeof(cgiff_chunk_t)) {
#endif
        return true;
    }
    return false;
}

#ifndef __M68000__
template<size_t S>
__forceinline static void swap(uint8_t bytes[S]) {
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

bool cgiff_file_c::read(void *data, size_t s, size_t n) {
    size_t read = fread(data, s, n, _file);
#ifndef __M68000__
    bool r = read == n;
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
#else
    bool r = read == (s * n);
#endif
    return r;
}
