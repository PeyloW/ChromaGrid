//
//  graphics_stencil.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-29.
//

#include "canvas.hpp"

using namespace toybox;

canvas_c::stencil_type_e  canvas_c::effective_type(stencil_type_e type) {
    if (type == random) {
        type = (stencil_type_e)(((uint16_t)rand() % 4) + 1);
    }
    return type;
}

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

const static uint8_t diag_16x16[16][16] = {    
    {  0,  2,  4,  6,  8, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 32 },
    {  2,  4,  6,  8, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 32, 34 },
    {  4,  6,  8, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 32, 34, 36 },
    {  6,  8, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 32, 34, 36, 38 },
    {  8, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 32, 34, 36, 38, 40 },
    { 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 32, 34, 36, 38, 40, 42 },
    { 13, 15, 17, 19, 21, 23, 25, 27, 29, 32, 34, 36, 38, 40, 42, 44 },
    { 15, 17, 19, 21, 23, 25, 27, 29, 32, 34, 36, 38, 40, 42, 44, 46 },
    { 17, 19, 21, 23, 25, 27, 29, 32, 34, 36, 38, 40, 42, 44, 46, 48 },
    { 19, 21, 23, 25, 27, 29, 32, 34, 36, 38, 40, 42, 44, 46, 48, 50 },
    { 21, 23, 25, 27, 29, 32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 53 },
    { 23, 25, 27, 29, 32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 53, 55 },
    { 25, 27, 29, 32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 53, 55, 57 },
    { 27, 29, 32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 53, 55, 57, 59 },
    { 29, 32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 53, 55, 57, 59, 61 },
    { 32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 53, 55, 57, 59, 61, 63 },
 };

static uint8_t circle_16x16[16][16] = {    
    { 59, 55, 51, 48, 46, 44, 42, 42, 42, 43, 44, 47, 50, 53, 57, 61 },
    { 55, 51, 47, 43, 40, 38, 36, 36, 36, 37, 39, 42, 45, 49, 53, 57 },
    { 51, 47, 43, 39, 35, 33, 31, 30, 30, 32, 34, 37, 41, 45, 49, 54 },
    { 48, 43, 39, 34, 31, 27, 25, 24, 25, 26, 29, 32, 36, 41, 46, 51 },
    { 46, 40, 35, 31, 26, 22, 20, 18, 19, 21, 24, 28, 33, 38, 43, 48 },
    { 44, 38, 33, 27, 22, 18, 14, 13, 13, 16, 20, 25, 30, 35, 41, 46 },
    { 42, 36, 31, 25, 20, 14, 10,  7,  8, 12, 17, 22, 28, 34, 39, 45 },
    { 42, 36, 30, 24, 18, 13,  7,  2,  4, 10, 16, 21, 27, 33, 39, 44 },
    { 42, 36, 30, 25, 19, 13,  8,  4,  6, 11, 16, 22, 27, 33, 39, 45 },
    { 43, 37, 32, 26, 21, 16, 12, 10, 11, 14, 18, 24, 29, 34, 40, 46 },
    { 44, 39, 34, 29, 24, 20, 17, 16, 16, 18, 22, 26, 31, 36, 42, 47 },
    { 47, 42, 37, 32, 28, 25, 22, 21, 22, 24, 26, 30, 35, 39, 44, 49 },
    { 50, 45, 41, 36, 33, 30, 28, 27, 27, 29, 31, 35, 38, 43, 47, 52 },
    { 53, 49, 45, 41, 38, 35, 34, 33, 33, 34, 36, 39, 43, 47, 51, 55 },
    { 57, 53, 49, 46, 43, 41, 39, 39, 39, 40, 42, 44, 47, 51, 55, 59 },
    { 61, 57, 54, 51, 48, 46, 45, 44, 45, 46, 47, 49, 52, 55, 59, 63 },
 };

static void make_dither_mask(canvas_c::stencil_t stencil, const uint8_t mask_8x8[8][8], int shade) {
    for (int y = 8; --y != -1; ) {
        uint16_t row = 0;
        for (int x = 8; --x != -1; ) {
            if (mask_8x8[x][y] < shade) {
                row |= (0x101 << x);
            }
        }
        stencil[y] = row;
        stencil[y + 8] = row;
    }
}

void make_dither_mask(canvas_c::stencil_t stencil, const uint8_t mask_16x16[16][16], int shade) {
    for (int y = 16; --y != -1; ) {
        uint16_t row = 0;
        for (int x = 16; --x != -1; ) {
            if (mask_16x16[x][y] < shade) {
                row |= (0x1 << x);
            }
        }
        stencil[y] = row;
    }
}

void make_dither_mask(canvas_c::stencil_t stencil, int (*func)(int), int shade) {
    for (int y = 16; --y != -1; ) {
        uint16_t row = 0;
        for (int x = 16; --x != -1; ) {
            if (func(x + y * 16) < shade) {
                row |= (0x1 << x);
            }
        }
        stencil[y] = row;
    }
}

void canvas_c::make_stencil(stencil_t stencil, stencil_type_e type, int shade) {
    assert(shade >= STENCIL_FULLY_TRANSPARENT);
    assert(shade <= STENCIL_FULLY_OPAQUE);
    switch (type) {
        case orderred:
            make_dither_mask(stencil, bayer_8x8, shade);
            break;
        case noise:
            make_dither_mask(stencil, &brand, shade);
            break;
        case diagonal:
            make_dither_mask(stencil, diag_16x16, shade);
            break;
        case circle:
            make_dither_mask(stencil, circle_16x16, shade);
            break;
        default:
            assert(0);
            break;
    }
}

const canvas_c::stencil_t *const canvas_c::stencil(stencil_type_e type, int shade) {
    assert(shade >= canvas_c::STENCIL_FULLY_TRANSPARENT);
    assert(shade <= canvas_c::STENCIL_FULLY_OPAQUE);
    assert(type < random);
    static canvas_c::stencil_t _none_stencil = { 0 };
    static bool _initialized = false;
    static stencil_t _stencils[4][canvas_c::STENCIL_FULLY_OPAQUE + 1];
    if (!_initialized) {
        for (int i = 4; --i != -1; ) {
            for (int j = canvas_c::STENCIL_FULLY_OPAQUE + 1; --j != -1; ) {
                canvas_c::make_stencil(_stencils[i][j], (canvas_c::stencil_type_e)(i + 1), j);
            }
        }
        _initialized = true;
    }
    if (type == none) {
        return &_none_stencil;
    } else {
        return &_stencils[type - 1][shade];
    }
}
