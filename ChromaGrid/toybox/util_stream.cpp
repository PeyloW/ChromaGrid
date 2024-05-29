//
//  util_stream.cpp
//  toybox
//
//  Created by Fredrik on 2024-05-19.
//

#include "util_stream.hpp"

using namespace toybox;


nullstream_c::nullstream_c() : stream_c() {}

ptrdiff_t nullstream_c::tell() const {
    return 0;
}

ptrdiff_t nullstream_c::seek(ptrdiff_t pos, seekdir_e way) {
    return 0;
}

size_t nullstream_c::read(uint8_t *buf, size_t count) {
    memset(buf, 0, count);
    return count;
};

size_t nullstream_c::write(const uint8_t *buf, size_t count) {
    return count;
};


swapstream_c::swapstream_c(stream_c *stream) :
    stream_c(), _stream(stream)
{
    assert(stream);
}

void swapstream_c::set_assert_on_error(bool assert) {
    stream_c::set_assert_on_error(assert);
    _stream->set_assert_on_error(assert);
}

bool swapstream_c::good() const { return _stream->good(); };
ptrdiff_t swapstream_c::tell() const { return _stream->tell(); }
ptrdiff_t swapstream_c::seek(ptrdiff_t pos, seekdir_e way) { return _stream->seek(pos, way); }
bool swapstream_c::flush() { return _stream->flush(); }

__forceinline static void swap_buffer(uint16_t *buf, size_t count) {
    while (--count != -1) {
        hton(*buf);
        buf++;
    }
}
__forceinline static void swap_buffer(uint32_t *buf, size_t count) {
    while (--count != -1) {
        hton(*buf);
        buf++;
    }
}

size_t swapstream_c::read(uint8_t *buf, size_t count) { return _stream->read(buf, count); }
size_t swapstream_c::read(uint16_t *buf, size_t count) {
    size_t r = _stream->read(buf, count);
    if (r) {
        swap_buffer(buf, count);
    }
    return r;
}
size_t swapstream_c::read(uint32_t *buf, size_t count) {
    size_t r = _stream->read(buf, count);
    if (r) {
        swap_buffer(buf, count);
    }
    return r;
}

size_t swapstream_c::write(const uint8_t *buf, size_t count) {
    return _stream->write(buf, count);
};
size_t swapstream_c::write(const uint16_t *buf, size_t count) {
    uint16_t tmpbuf[count];
    memcpy(tmpbuf, buf, count * sizeof(uint16_t));
    swap_buffer(tmpbuf, count);
    return _stream->write(tmpbuf, count);
}
size_t swapstream_c::write(const uint32_t *buf, size_t count) {
    uint32_t tmpbuf[count];
    memcpy(tmpbuf, buf, count * sizeof(uint32_t));
    swap_buffer(tmpbuf, count);
    return _stream->write(tmpbuf, count);
}


/*
bufstream_c::bufstream_c(stream_c *stream, size_t len) :
    stream_c(), _stream(stream), _len(len), _max(0), _pos(0), _buffer((uint8_t*)malloc(len)), _mutated(false)
{
    assert(stream);
}

void bufstream_c::set_assert_on_error(bool assert) {
    stream_c::set_assert_on_error(assert);
    _stream->set_assert_on_error(assert);
}

bool bufstream_c::good() const {
    return _stream->good();
}

ptrdiff_t bufstream_c::tell() const {
    if (_pos >= 0) {
        ptrdiff_t r = _stream->tell();
        if (r >= 0) {
            return r - _max + _pos;
        } else {
            return -1;
        }
    }
    return _pos;
}

ptrdiff_t bufstream_c::seek(ptrdiff_t pos, seekdir_e way) {
    if (_mutated) {
        flush();
    }
    _stream->seek(pos, way);
    _pos = 0;
    _max = 0;
    return tell();
}

bool bufstream_c::flush() {
    assert(_pos > 0);
    bool r = overflow(_buffer.get(), _pos) == _pos;
    _mutated = false;
    _pos = 0;
    _max = 0;
    return r;
}

size_t bufstream_c::read(uint8_t *buf, size_t count) {
    if (_mutated) {
        if (_assert_on_error) {
            assert(0);
        }
        return 0;
    }
    size_t read_count = 0;
    while (read_count < count) {
        if (_pos == _max) {
            _max = underflow(_buffer.get(), _len);
            _pos = 0;
            if (_max == 0) {
                break;
            }
        }
        size_t available = MIN(count - read_count, _max - _pos);
        if (available > 0) {
            memcpy(buf + read_count, _buffer.get() + _pos, available);
            _pos += available;
            read_count += available;
        }
    }
    return read_count;
}

size_t bufstream_c::write(const uint8_t *buf, size_t count) {
    size_t write_count = 0;
    while (write_count < count) {
        if (_pos == _len) {
            if (!flush()) {
                break;
            }
        }
        size_t available = MIN(count - write_count, _len - _pos);
        if (available > 0) {
            _mutated = true;
            memcpy(_buffer.get() + _pos, buf + write_count, available);
            _pos += available;
            write_count += available;
        }
    }
    return write_count;
}

size_t bufstream_c::underflow(uint8_t *buf, size_t len) {
    return _stream->read(buf, len);
}

size_t bufstream_c::overflow(const uint8_t *buf, size_t len) {
    return _stream->write(buf, len);
}
*/
