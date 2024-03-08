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

bool cgiff_file_c::find(const cgiff_id_t id, const cgiff_id_t type, cgiff_group_t &header) {
    while (read(header)) {
        if (header.chunk.id == id && header.subtype) {
            return true;
        }
        if (!skip(header)) {
            break;
        }
    }
    return false;
}

bool cgiff_file_c::find(const cgiff_id_t id, cgiff_chunk_t &chunk) {
    while (read(chunk)) {
        if (chunk.id == id) {
            return true;
        }
        if (!skip(chunk)) {
            break;
        }
    }
    return false;
}

bool cgiff_file_c::expand(const cgiff_chunk_t &chunk, cgiff_group_t &group) {
    group.chunk = chunk;
    return read(group.subtype);
}

bool cgiff_file_c::skip(const cgiff_group_t &header) {
    return skip(header.chunk);
}

bool cgiff_file_c::skip(const cgiff_chunk_t &chunk) {
    if (fseek(_file, chunk.size - sizeof(cgiff_id_t), SEEK_CUR)) {
        return align();
    }
    return false;
}

bool cgiff_file_c::align() {
    long pos = ftell(_file);
    if (pos >= 0) {
        if ((pos & 1) != 0) {
            return fseek(_file, 1, SEEK_CUR);
        }
        return true;
    }
    return false;
}

bool cgiff_file_c::read(void *data, size_t s, size_t n) {
    size_t read = fread(data, s, n, _file);
#ifndef __M68000__
    bool r = read == n;
#else
    bool r = read == (s * n);
#endif
    return r;
}
