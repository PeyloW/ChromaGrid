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

