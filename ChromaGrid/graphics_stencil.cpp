//
//  graphics_stencil.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-29.
//

#include "graphics.hpp"

static void make_dither_mask_orderred(cgimage_c::stencil_t stencil, int shade) {
    const static uint8_t bayer_8x8[8][8] = {
        { 0, 32,  8, 40,  2, 34, 10, 42},
        {48, 16, 56, 24, 50, 18, 58, 26},
        {12, 44,  4, 36, 14, 46,  6, 38},
        {60, 28, 52, 20, 62, 30, 54, 22},
        { 3, 35, 11, 43,  1, 33,  9, 41},
        {51, 19, 59, 27, 49, 17, 57, 25},
        {15, 47,  7, 39, 13, 45,  5, 37},
        {63, 31, 55, 23, 61, 29, 53, 21}
    };
    for (int y = 8; --y != -1; ) {
        stencil[y] = 0;
        for (int x = 8; --x != -1; ) {
            if (bayer_8x8[x][y] < shade) {
                stencil[y] |= (0x101 << x);
            }
        }
    }
    for (int y = 8; --y != -1; ) {
        stencil[y + 8] = stencil[y];
    }
}

void make_dither_mask_blue_noise(cgimage_c::stencil_t stencil, int shade) {
    const static uint8_t blue_16x16[16][16] = {
        { 10,  2, 17, 23, 10, 34,  4, 28, 37,  2, 19,  7,  3,  1,  5, 62 },
        {  0,  8, 55, 46,  1, 61, 19,  0,  1,  9, 45, 25, 34, 16,  0, 24 },
        { 13, 28,  5,  0,  3, 26, 12,  6, 42, 14, 59,  0, 11,  2, 50, 42 },
        {  1, 37, 20, 14, 32,  8,  2, 53, 22,  3,  0, 29,  4, 56, 18,  3 },
        {  0, 57,  2,  6, 50,  0, 38,  0, 31,  8, 17, 39,  1,  6, 32,  9 },
        { 47, 23, 10,  1, 42, 21, 16,  4, 58,  5, 48,  2, 13, 21,  0, 15 },
        {  5, 35,  0, 63,  4,  1, 28, 11,  0,  1, 25,  9, 62, 27, 41,  2 },
        {  8, 29, 18, 12, 26,  7, 53, 14, 43, 19, 36,  0,  4,  0, 54,  1 },
        { 51,  0,  3,  0, 39,  2,  0, 34,  6,  3, 10, 46, 16,  6, 12, 20 },
        { 40, 14, 57, 22, 47, 10,  1, 59, 24,  0, 52, 30,  2, 36, 26,  0 },
        {  9, 32,  2,  6, 16, 31, 20,  4,  1, 15,  7, 21,  1, 60,  7,  3 },
        {  0,  4, 45,  1,  0,  3, 49, 12, 35, 55,  0,  4, 13,  0, 49, 17 },
        { 58, 27, 11, 36, 61,  8, 18,  0, 40, 27,  9, 44, 33,  5, 38, 23 },
        {  0,  7, 19,  0, 24, 29,  2,  0,  6,  3,  1, 17, 25,  2, 11,  1 },
        { 43, 52,  3, 13,  5, 54, 44, 11, 22, 63, 31,  0, 56,  8,  0, 15 },
        {  4, 33,  0, 41,  1,  7,  0, 15, 51,  5,  0, 12, 48, 39, 21, 30 }
    };
    for (int y = 16; --y != -1; ) {
        stencil[y] = 0;
        for (int x = 16; --x != -1; ) {
            if (blue_16x16[x][y] < shade) {
                stencil[y] |= (0x1 << x);
            }
        }
    }
}

void cgimage_c::make_stencil(stencil_t stencil, stencil_type type, int shade) {
    assert(shade >= STENCIL_FULLY_TRANSPARENT);
    assert(shade <= STENCIL_FULLY_OPAQUE);
    switch (type) {
        case orderred:
            make_dither_mask_orderred(stencil, shade);
            break;
        case noise:
            make_dither_mask_blue_noise(stencil, shade);
            break;
        default:
            assert(0);
            break;
    }
}
