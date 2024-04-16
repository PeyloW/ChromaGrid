//
//  iffstream.cpp
//  toybox
//
//  Created by Fredrik on 2024-04-15.
//

#include "iffstream.hpp"

using namespace toystd;

iffstream_c::iffstream_c(stream_c &stream) : stream_c(), _stream(stream) {}

iffstream_c::iffstream_c(const char *path, fstream_c::openmode_e mode) :
#ifdef __M68000__
    stream_c(), _owned_stream(new fstream_c(path, mode)), _stream(*_owned_stream)
#else
stream_c(), _owned_stream(new hton_stream_c(new fstream_c(path, mode))), _stream(*_owned_stream)
#endif
{}

iffstream_c::~iffstream_c() {
    if (_owned_stream) {
        delete _owned_stream;
    }
}

void iffstream_c::set_assert_on_error(bool assert) {
    stream_c::set_assert_on_error(assert);
    _stream.set_assert_on_error(assert);
}

bool iffstream_c::good() const { return _stream.good(); }
ptrdiff_t iffstream_c::tell() const { return _stream.tell(); }
ptrdiff_t iffstream_c::seek(ptrdiff_t pos, seekdir_e way) { return _stream.seek(pos, way); }

bool iffstream_c::first(const char *const id, iff_chunk_s &chunk) {
    bool result = false;
    if (seek(0, beg) == 0) {
        if (read(chunk)) {
            result = iff_id_match(chunk.id, id);
        }
    }
    if (_assert_on_error) {
        hard_assert(result);
    }
    return result;
}

bool iffstream_c::first(const char *const id, const char *const subtype, iff_group_s &group) {
    bool result = false;
    if (seek(0, beg) == 0) {
        if (read(group)) {
            result = iff_id_match(group.id, id) && iff_id_match(group.subtype, subtype);
        }
    }
    if (_assert_on_error) {
        hard_assert(result);
    }
    return result;
}

bool iffstream_c::next(const iff_group_s &in_group, const char *const id, iff_chunk_s &chunk) {
    // Hard assert handled by called functions.
    const long end = in_group.offset + sizeof(uint32_t) * 2 + in_group.size;
    long pos = tell();
    while (tell() < end  && read(chunk)) {
        if (iff_id_match(chunk.id, id)) {
            return true;
        }
        if (!skip(chunk)) {
            break;
        }
    }
    seek(pos, beg);
    return false;
}

bool iffstream_c::expand(const iff_chunk_s &chunk, iff_group_s &group) {
    // Hard assert handled by called functions.
    group.offset = chunk.offset;
    group.id = chunk.id;
    group.size = chunk.size;
    if (reset(group)) {
        return read(&group.subtype);
    }
    return false;
}


bool iffstream_c::reset(const iff_chunk_s &chunk) {
    return seek(chunk.offset + sizeof(uint32_t) * 2, beg) >= 0;
}

bool iffstream_c::skip(const iff_chunk_s &chunk) {
    bool result = false;
    long end = chunk.offset + sizeof(uint32_t) * 2 + chunk.size;
    if ((result = seek(end, beg) >= 0)) {
        result = align();
    }
    return result;
}

bool iffstream_c::align() {
    long pos = tell();
    bool result = pos >= 0;
    if (result) {
        if ((pos & 1) != 0) {
            result = seek(1, cur) >= 0;
            goto done;
        }
    }
done:
    return result;
}

bool iffstream_c::begin(iff_chunk_s &chunk, const char *const id) {
    bool result = false;
    if (align()) {
        chunk.offset = tell();
        if (chunk.offset >= 0) {
            chunk.id = iff_id_make(id);
            chunk.size = -1;
            result = write(&chunk.id) && write(&chunk.size);
        }
    }
    return result;
}

bool iffstream_c::end(iff_chunk_s &chunk) {
    bool result = false;
    long pos = tell();
    if (pos >= 0) {
        uint32_t size = (uint32_t)(pos - (chunk.offset + 8));
        chunk.size = size;
        if (seek(chunk.offset + 4, beg) >= 0) {
            if (write(&size)) {
                result = seek(pos, beg) >= 0;
            }
        }
    }
    return result;
}

bool iffstream_c::read(iff_group_s &group) {
    bool result = false;
    if (read(static_cast<iff_chunk_s&>(group))) {
        result = read(&group.subtype);
    }
    return result;
}

bool iffstream_c::read(iff_chunk_s &chunk) {
    bool result = align();
    if (result) {
        chunk.offset = tell();
        if (chunk.offset >= 0) {
            result = read(&chunk.id) && read(&chunk.size);
        }
    }
    return result;
}

bool iffstream_c::read(uint8_t *buf, size_t count) { return _stream.read(buf, count); }
bool iffstream_c::read(uint16_t *buf, size_t count) { return _stream.read(buf, count); }
bool iffstream_c::read(uint32_t *buf, size_t count) { return _stream.read(buf, count); }

bool iffstream_c::write(const uint8_t *buf, size_t count) { return _stream.write(buf, count); };
bool iffstream_c::write(const uint16_t *buf, size_t count) { return _stream.write(buf, count); };
bool iffstream_c::write(const uint32_t *buf, size_t count) { return _stream.write(buf, count); };
