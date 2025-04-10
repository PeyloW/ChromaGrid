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

ptrdiff_t substream_c::tell() const {
    auto t = _stream->tell() - _origin;
    if (t < -1 || t > _length) {
        return -1;
    }
    return t;
};

ptrdiff_t substream_c::seek(ptrdiff_t pos, seekdir_e way) {
    switch (way) {
        case stream_c::beg:
            _stream->seek(pos + _origin, way);
            break;
        case stream_c::cur:
            _stream->seek(pos, way);
            break;
        case stream_c::end:
            _stream->seek(pos + _origin + _length, way);
            break;
    }
    return tell();
}

size_t substream_c::read(uint8_t *buf, size_t count) {
    count = MIN(count, _length - tell());
    if (count > 0) {
        return _stream->read(buf, count);
    }
    return count;
}

size_t substream_c::write(const uint8_t *buf, size_t count) {
    count = MIN(count, _length - tell());
    if (count > 0) {
        return _stream->write(buf, count);
    }
    return count;
}
