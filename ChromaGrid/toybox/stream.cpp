//
//  stream.cpp
//  toybox
//
//  Created by Fredrik on 2024-04-15.
//

#include "stream.hpp"

using namespace toystd;

stream_c::stream_c() : _assert_on_error(false) {};

void stream_c::set_assert_on_error(bool assert) {
    _assert_on_error = assert;
}

bool stream_c::good() const { return true; };

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


hton_stream_c::hton_stream_c(stream_c *stream) : stream_c(), _owned_stream(stream), _stream(*_owned_stream) {}
hton_stream_c::hton_stream_c(stream_c &stream) : stream_c(), _owned_stream(nullptr), _stream(stream) {}
hton_stream_c::~hton_stream_c() {
    if (_owned_stream) {
        delete _owned_stream;
    }
}

void hton_stream_c::set_assert_on_error(bool assert) {
    stream_c::set_assert_on_error(assert);
    _stream.set_assert_on_error(assert);
}

bool hton_stream_c::good() const { return _stream.good(); };
ptrdiff_t hton_stream_c::tell() const { return _stream.tell(); }
ptrdiff_t hton_stream_c::seek(ptrdiff_t pos, seekdir_e way) { return _stream.seek(pos, way); }

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

bool hton_stream_c::read(uint8_t *buf, size_t count) { return _stream.read(buf, count); }
bool hton_stream_c::read(uint16_t *buf, size_t count) {
    bool r = _stream.read(buf, count);
    if (r) {
        swap_buffer(buf, count);
    }
    return r;
}
bool hton_stream_c::read(uint32_t *buf, size_t count) {
    bool r = _stream.read(buf, count);
    if (r) {
        swap_buffer(buf, count);
    }
    return r;
}

bool hton_stream_c::write(const uint8_t *buf, size_t count) {
    return _stream.write(buf, count);
};
bool hton_stream_c::write(const uint16_t *buf, size_t count) {
    uint16_t tmpbuf[count];
    memcpy(tmpbuf, buf, count * sizeof(uint16_t));
    swap_buffer(tmpbuf, count);
    return _stream.write(tmpbuf, count);
}
bool hton_stream_c::write(const uint32_t *buf, size_t count) {
    uint32_t tmpbuf[count];
    memcpy(tmpbuf, buf, count * sizeof(uint32_t));
    swap_buffer(tmpbuf, count);
    return _stream.write(tmpbuf, count);
}


fstream_c::fstream_c(const char *path, openmode_e mode) :
    stream_c(), _path(path), _mode(mode), _file(nullptr)
{
    open();
}

fstream_c::~fstream_c() {
    set_assert_on_error(false);
    if (is_open()) {
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

bool fstream_c::read(uint8_t *buf, size_t count) {
    return fread(buf, 1, count, _file) == count;
}
bool fstream_c::write(const uint8_t *buf, size_t count) {
    return fwrite(buf, 1, count, _file) == count;
}

