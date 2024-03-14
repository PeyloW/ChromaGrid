//
//  iff_file.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-04.
//

#include "iff_file.hpp"


cgiff_file_c::cgiff_file_c(FILE *file) : 
    _file(file), _hard_assert(false), _owns_file(false)
{
    hard_assert(CGIFF_FORM_ID != 0);
    hard_assert(file);
    _for_writing = true;
};
cgiff_file_c::cgiff_file_c(const char *path, const char *mode) : 
    _file(fopen(path, mode)), _hard_assert(false), _owns_file(true)
{
    hard_assert(CGIFF_FORM_ID != 0);
    hard_assert(_file);
    _for_writing = strstr(mode, "w") != nullptr;
}
cgiff_file_c::~cgiff_file_c() {
    if (_owns_file) {
        fclose(_file);
    }
}

long cgiff_file_c::get_pos() const {
    long result = -1;
    if (_file) {
        result = ftell(_file);
    }
    if (_hard_assert) {
        hard_assert(result >= 0);
    }
    return result;
}

#ifndef __M68000__
bool cgiff_file_c::set_pos(long pos) const {
    bool result = fseek(_file, pos, SEEK_SET) >= 0;
    if (_hard_assert) {
        hard_assert(result >= 0);
    }
    return result;
}
#endif


bool cgiff_file_c::first(const char *const id, cgiff_chunk_t &chunk) {
    bool result = false;
    if (_file && fseek(_file, 0, SEEK_SET) == 0) {
        if (read(chunk)) {
            result = cgiff_id_match(chunk.id, id);
        }
    }
    if (_hard_assert) {
        hard_assert(result);
    }
    return result;
}

bool cgiff_file_c::first(const char *const id, const char *const subtype, cgiff_group_t &group) {
    bool result = false;
    if (_file && fseek(_file, 0, SEEK_SET) == 0) {
        if (read(group)) {
            result = cgiff_id_match(group.id, id) && cgiff_id_match(group.subtype, subtype);
        }
    }
    if (_hard_assert) {
        hard_assert(result);
    }
    return result;
}

bool cgiff_file_c::next(const cgiff_group_t &in_group, const char *const id, cgiff_chunk_t &chunk) {
    // Hard assert handled by called functions.
    const long end = in_group.offset + sizeof(uint32_t) * 2 + in_group.size;
    long pos = get_pos();
    while (get_pos() < end && read(chunk)) {
        if (cgiff_id_match(chunk.id, id)) {
            return true;
        }
        if (!skip(chunk)) {
            break;
        }
    }
    bool result = fseek(_file, pos, SEEK_SET) >= 0;
    if (_hard_assert) {
        hard_assert(result);
    }
    return false;
}

bool cgiff_file_c::expand(const cgiff_chunk_t &chunk, cgiff_group_t &group) {
    // Hard assert handled by called functions.
    group.offset = chunk.offset;
    group.id = chunk.id;
    group.size = chunk.size;
    if (reset(group)) {
        return read(group.subtype);
    }
    return false;
}

bool cgiff_file_c::reset(const cgiff_chunk_t &chunk) {
    bool result = fseek(_file, chunk.offset + sizeof(uint32_t) * 2, SEEK_SET) >= 0;
    if (_hard_assert) {
        hard_assert(result);
    }
    return result;
}

bool cgiff_file_c::skip(const cgiff_chunk_t &chunk) {
    bool result = false;
    long end = chunk.offset + sizeof(uint32_t) * 2 + chunk.size;
    if ((result = fseek(_file, end, SEEK_SET) >= 0)) {
        result = align();
    }
    if (_hard_assert) {
        hard_assert(result);
    }
    return result;
}

bool cgiff_file_c::align() {
    bool result = false;
    long pos = get_pos();
    if (pos >= 0) {
        if ((pos & 1) != 0) {
            if (_for_writing) {
                uint8_t zero = 0;
                return write(zero);
            } else {
                result = fseek(_file, 1, SEEK_CUR) >= 0;
                goto done;
            }
        }
        result = true;
    }
done:
    if (_hard_assert) {
        hard_assert(result);
    }
    return result;
}

bool cgiff_file_c::read(void *data, size_t s, size_t n) {
    size_t read = fread(data, s, n, _file);
#ifndef __M68000__
    bool result = read == n;
#else
    bool result = read == (s * n);
#endif
    if (_hard_assert) {
        hard_assert(result);
    }
    return result;
}

bool cgiff_file_c::begin(cgiff_chunk_t &chunk, const char *const id) {
    bool result = false;
    if (align()) {
        chunk.offset = get_pos();
        if (chunk.offset >= 0) {
            chunk.id = cgiff_id_make(id);
            chunk.size = -1;
            result = write(chunk.id) && write(chunk.size);
        }
    }
    if (_hard_assert) {
        hard_assert(result);
    }
    return result;
}

bool cgiff_file_c::end(cgiff_chunk_t &chunk) {
    bool result = false;
    long pos = get_pos();
    if (pos >= 0) {
        uint32_t size = (uint32_t)(pos - (chunk.offset + 8));
        chunk.size = size;
        fseek(_file, chunk.offset + 4, SEEK_SET);
        if (write(size)) {
            result = fseek(_file, pos, SEEK_SET) >= 0;
        }
    }
    if (_hard_assert) {
        hard_assert(result);
    }
    return result;
}

bool cgiff_file_c::write(void *data, size_t s, size_t n) {
    size_t read = fwrite(data, s, n, _file);
#ifndef __M68000__
    bool result = read == n;
#else
    bool result = read == (s * n);
#endif
    if (_hard_assert) {
        hard_assert(result);
    }
    return result;
}

bool cgiff_file_c::read(cgiff_group_t &group) {
    bool result = false;
    if (read(static_cast<cgiff_chunk_t&>(group))) {
        result = read(group.subtype);
    }
    if (_hard_assert) {
        hard_assert(result);
    }
    return result;
}

bool cgiff_file_c::read(cgiff_chunk_t &chunk) {
    bool result = align();
    if (result) {
        chunk.offset = get_pos();
        if (chunk.offset >= 0) {
            result = read(chunk.id) && read(chunk.size);
        }
    }
    if (_hard_assert) {
        hard_assert(result);
    }
    return result;
}
