//
//  utility.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2025-03-16.
//

#include "utility.hpp"

using namespace toybox;

#ifndef __M68000__

void toybox::hton_struct(void *ptr, const char *layout) {
    while (*layout) {
        char *end = nullptr;
        size_t cnt = (size_t)strtol(layout, &end, 0);
        if (end == layout) cnt = 1;
        layout = end;
        switch (*layout++) {
            case 'b':
                ptr = static_cast<uint8_t*>(ptr) + cnt;
                break;
            case 'w': {
                auto *buf = static_cast<uint16_t *>(ptr);
                hton(buf, cnt);
                ptr = buf + cnt;
                break;
            }
            case 'l': {
                auto *buf = static_cast<uint32_t *>(ptr);
                hton(buf, cnt);
                ptr = buf + cnt;
                break;
            }
            default:
                assert(0);
                break;
        }
    }
}

#endif
