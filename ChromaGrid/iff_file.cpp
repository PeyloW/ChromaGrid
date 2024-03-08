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

bool cgiff_file_c::first(const char *const id, const char *const subtype, cgiff_group_t &group) {
    if (fseek(_file, 0, SEEK_SET) == 0) {
        if (read(group)) {
            return cgiff_id_match(group.id, id) && cgiff_id_match(group.subtype, subtype);
        }
    }
    return false;
}

bool cgiff_file_c::next(const cgiff_group_t &in_group, const char *const id, cgiff_chunk_t &chunk) {
    const long end = in_group.offset + sizeof(uint32_t) * 2 + in_group.size;
    while (ftell(_file) < end && read(chunk)) {
        if (cgiff_id_match(chunk.id, id)) {
            return true;
        }
        if (!skip(chunk)) {
            break;
        }
    }
    return false;
}

bool cgiff_file_c::expand(const cgiff_chunk_t &chunk, cgiff_group_t &group) {
    group.offset = chunk.offset;
    group.id = chunk.id;
    group.size = chunk.size;
    return read(group.subtype);
}

bool cgiff_file_c::skip(const cgiff_chunk_t &chunk) {
    long end = chunk.offset + sizeof(uint32_t) * 2 + chunk.size;
    if (fseek(_file, end, SEEK_SET)) {
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

bool cgiff_file_c::read(cgiff_group_t &group) {
    if (read(static_cast<cgiff_chunk_t&>(group))) {
        return read(group.subtype);
    }
    return false;
}

bool cgiff_file_c::read(cgiff_chunk_t &chunk) {
    align();
    chunk.offset = ftell(_file);
    if (chunk.offset >= 0) {
        return read(chunk.id) && read(chunk.size);
    }
    return false;
}
