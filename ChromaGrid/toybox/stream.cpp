//
//  stream.cpp
//  toybox
//
//  Created by Fredrik on 2024-04-15.
//

#include "stream.hpp"

using namespace toystd;

stream_c::stream_c() : _assert_on_error(false), _width(0), _fill(' ') {};

void stream_c::set_assert_on_error(bool assert) {
    _assert_on_error = assert;
}

bool stream_c::good() const { return true; };
bool stream_c::flush() { return true; }

static bool read_or_write_struct(stream_c &stream, void *buf, const char *layout, const bool write) {
    while (*layout) {
        char *end = nullptr;
        int cnt = (int)strtol(layout, &end, 0);
        if (end == layout) cnt = 1;
        layout = end;
        switch (*layout++) {
            case 'b':
                if (!(write ? stream.write((uint8_t*)buf, cnt) : stream.read((uint8_t*)buf, cnt))) {
                    return false;
                }
                buf = ((uint8_t*)buf + cnt);
                break;
            case 'w':
                if (!(write ? stream.write((uint16_t*)buf, cnt) : stream.read((uint16_t*)buf, cnt))) {
                    return false;
                }
                buf = ((uint16_t*)buf + cnt);
                break;
            case 'l':
                if (!(write ? stream.write((uint32_t*)buf, cnt) : stream.read((uint32_t*)buf, cnt))) {
                    return false;
                }
                buf = ((uint32_t*)buf + cnt);
                break;
            default:
                return false;
        }
    }
    return true;
}

bool stream_c::read(uint16_t *buf, size_t count) {
    return read((uint8_t*)buf, count * 2);
};
bool stream_c::read(uint32_t *buf, size_t count) {
    return read((uint8_t*)buf, count * 4);
};
bool stream_c::read(void *buf, const char * layout) {
    bool r = read_or_write_struct(*this, buf, layout, false);
    if (_assert_on_error) {
        hard_assert(r);
    }
    return r;
}

bool stream_c::write(const uint16_t *buf, size_t count) {
    return write((uint8_t*)buf, count * 2);
};
bool stream_c::write(const uint32_t *buf, size_t count) {
    return write((uint8_t*)buf, count * 4);
};
bool stream_c::write(const void *buf, const char * layout) {
    bool r = read_or_write_struct(*this, (void *)buf, layout, true);
    if (_assert_on_error) {
        hard_assert(r);
    }
    return r;
}

stream_c &stream_c::operator<<(manipulator_f m) {
    return m(*this);
}

stream_c &stream_c::operator<<(const char *str) {
    auto len = strlen(str);
    write((const int8_t *)str, len);
    return *this;
}

stream_c &stream_c::operator<<(const char c) {
    write((uint8_t*)&c, 1);
    return *this;
}
stream_c &stream_c::operator<<(const unsigned char c) {
    write((uint8_t*)&c, 1);
    return *this;
}
stream_c &stream_c::operator<<(const int16_t i) {
    return *this << (int32_t)i;
}
stream_c &stream_c::operator<<(const uint16_t i) {
    return *this << (uint32_t)i;
}
stream_c &stream_c::operator<<(const int32_t i) {
    char buf[12];
    sprintf(buf, "%ld", i);
    if (_width > 0) {
        int len = (int)strlen(buf);
        int fillc = _width - len;
        if (fillc > 0) {
            char buf[fillc];
            for (int i = fillc; --i != -1; ) buf[i] = _fill;
            write((uint8_t*)buf, fillc);
        }
    }
    return *this << (const char *)buf;
}
stream_c &stream_c::operator<<(const uint32_t i) {
    if (i > 0x7fffffff) {
        if (_width > 6) {
            int ow = _width;
            _width -= 6;
            *this << (i / 1000000);
            _width = 0;
            *this << (i % 1000000);
            _width = ow;
        } else {
            *this << (i / 1000000);
            *this << (i % 1000000);
        }
        return *this;
    } else {
        return *this << (int32_t)i;
    }
}

static fstream_c s_tbin(stdin);
static fstream_c s_tbout(stdout);
static fstream_c s_tberr(stderr);

stream_c& toystd::tbin = s_tbin;
stream_c& toystd::tbout = s_tbout;
stream_c& toystd::tberr = s_tberr;

static stream_c &s_endl(stream_c &s) {
#ifdef __M68000__
    return s << "\n\r";
#else
    return s << '\n';
#endif
}
static stream_c &s_ends(stream_c &s) {
    s.write((uint8_t *)"", 1);
    return s;
}
static stream_c &s_flush(stream_c &s) {
    s.flush();
    return s;
}

stream_c::manipulator_f toystd::endl = &s_endl;
stream_c::manipulator_f toystd::ends = &s_ends;
stream_c::manipulator_f toystd::flush = &s_flush;

swap_stream_c::swap_stream_c(stream_c *stream) : stream_c(), _stream(stream) {}

void swap_stream_c::set_assert_on_error(bool assert) {
    stream_c::set_assert_on_error(assert);
    _stream->set_assert_on_error(assert);
}

bool swap_stream_c::good() const { return _stream->good(); };
ptrdiff_t swap_stream_c::tell() const { return _stream->tell(); }
ptrdiff_t swap_stream_c::seek(ptrdiff_t pos, seekdir_e way) { return _stream->seek(pos, way); }
bool swap_stream_c::flush() { return _stream->flush(); }

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

bool swap_stream_c::read(uint8_t *buf, size_t count) { return _stream->read(buf, count); }
bool swap_stream_c::read(uint16_t *buf, size_t count) {
    bool r = _stream->read(buf, count);
    if (r) {
        swap_buffer(buf, count);
    }
    return r;
}
bool swap_stream_c::read(uint32_t *buf, size_t count) {
    bool r = _stream->read(buf, count);
    if (r) {
        swap_buffer(buf, count);
    }
    return r;
}

bool swap_stream_c::write(const uint8_t *buf, size_t count) {
    return _stream->write(buf, count);
};
bool swap_stream_c::write(const uint16_t *buf, size_t count) {
    uint16_t tmpbuf[count];
    memcpy(tmpbuf, buf, count * sizeof(uint16_t));
    swap_buffer(tmpbuf, count);
    return _stream->write(tmpbuf, count);
}
bool swap_stream_c::write(const uint32_t *buf, size_t count) {
    uint32_t tmpbuf[count];
    memcpy(tmpbuf, buf, count * sizeof(uint32_t));
    swap_buffer(tmpbuf, count);
    return _stream->write(tmpbuf, count);
}

fstream_c::fstream_c(FILE *file) :
    stream_c(), _path(nullptr), _mode(input), _file(file)
{}

fstream_c::fstream_c(const char *path, openmode_e mode) :
    stream_c(), _path(path), _mode(mode), _file(nullptr)
{
    open();
}

fstream_c::~fstream_c() {
    set_assert_on_error(false);
    if (is_open() && _path) {
        close();
    }
}

static const char *mode_for_mode(fstream_c::openmode_e mode) {
    if (mode & fstream_c::append) {
        assert(mode & fstream_c::output);
        if (mode & fstream_c::input) {
            return "a+";
        } else {
            return "a";
        }
    } else if (mode & fstream_c::output) {
        if (mode & fstream_c::input) {
            return "w+";
        } else {
            return "w";
        }
    } else if (mode & fstream_c::input) {
        return "r";
    } else {
        return "";
    }
}

bool fstream_c::open() {
    bool r = false;
    if (_file == nullptr) {
        _file = fopen(_path, mode_for_mode(_mode));
        r = _file != nullptr;
    }
    if (_assert_on_error) {
        hard_assert(r);
    }
    return r;
}

bool fstream_c::close() {
    bool r = false;
    if (_file) {
        r = fclose(_file) == 0;
        _file = nullptr;
    }
    if (_assert_on_error) {
        hard_assert(r);
    }
    return r;
}

bool fstream_c::good() const { return is_open(); };

ptrdiff_t fstream_c::tell() const {
    auto r = ftell(_file);
    if (_assert_on_error) {
        hard_assert(r >= 0);
    }
    return r;
}

ptrdiff_t fstream_c::seek(ptrdiff_t pos, stream_c::seekdir_e way) {
    auto r = fseek(_file, pos, way);
    if (_assert_on_error) {
        hard_assert(r >= 0);
    }
    return r;
}
bool fstream_c::flush() { return true; }

bool fstream_c::read(uint8_t *buf, size_t count) {
    return fread(buf, 1, count, _file) == count;
}
bool fstream_c::write(const uint8_t *buf, size_t count) {
    return fwrite(buf, 1, count, _file) == count;
}

strstream_c::strstream_c(int len) :
    _owned_buf((char *)malloc(len)), _buf(_owned_buf.get()), _len(len), _pos(0), _max(0)
{}

strstream_c::strstream_c(char *buf, int len) :
    _owned_buf(), _buf(buf), _len(len), _pos(0), _max(0)
{}

ptrdiff_t strstream_c::tell() const {
    return _pos;
}
ptrdiff_t strstream_c::seek(ptrdiff_t pos, seekdir_e way) {
    assert(ABS(pos <= _len));
    switch (way) {
        case seekdir_e::beg:
            _pos = (int)pos;
            break;
        case seekdir_e::cur:
            _pos += pos;
            break;
        case seekdir_e::end:
            _pos = _max - (int)pos;
            break;
    }
    if (_pos < 0 || _pos >= _len) {
        _pos = -1;
    }
    return _pos;
}

bool strstream_c::read(uint8_t *buf, size_t count) {
    if (count <= (_max - _pos)) {
        memcpy(buf, _buf + _pos, count);
        _pos += count;
        return true;
    } else {
        return false;
    }
}

bool strstream_c::write(const uint8_t *buf, size_t count) {
    if (count <= (_len - _pos)) {
        memcpy(_buf + _pos, buf, count);
        _pos += count;
        return true;
    } else {
        return false;
    }
}
