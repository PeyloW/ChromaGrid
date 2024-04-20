//
//  graphics.hpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-04.
//

#ifndef graphics_hpp
#define graphics_hpp

#include "cincludes.hpp"
#include "types.hpp"

namespace toybox {
    
    using namespace toystd;

    class color_c {
    public:
        uint16_t color;
        color_c() = default;
        color_c(uint16_t c) : color(c) {}
        color_c(const uint8_t r, const uint8_t g, const uint8_t b) : color(to_ste(r, 8) | to_ste(g, 4) | to_ste(b, 0)) {}
        void set_at(const int i) const {
#ifdef __M68000__
#   if TOYBOX_TARGET_ATARI
            reinterpret_cast<uint16_t*>(0xffff8240)[i] = color;
#   else
#       error "Unsupported target"
#   endif
#endif
        }
        void get(uint8_t *r, uint8_t *g, uint8_t *b) const {
            *r = from_ste(color, 8);
            *g = from_ste(color, 4);
            *b = from_ste(color, 0);
        }
        color_c mix(color_c other, int shade) const;
        static const int MIX_FULLY_THIS = 0;
        static const int MIX_FULLY_OTHER = 64;
    private:
        __forceinline static uint16_t to_ste(const uint8_t c, const uint8_t shift) {
            static const uint8_t STE_TO_SEQ[16] = { 0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15 };
            return STE_TO_SEQ[c >> 4] << shift;
        }
        __forceinline static uint8_t from_ste(const uint16_t c, const uint8_t shift) {
            static const uint8_t STE_FROM_SEQ[16] = { 0x00, 0x22, 0x44, 0x66, 0x88, 0xaa, 0xcc, 0xee,
                0x11, 0x33, 0x55, 0x77, 0x99, 0xbb, 0xdd, 0xff};
            return STE_FROM_SEQ[(c >> shift) & 0x0f];
        }
    };
        
    template<int Count>
    class basic_palette_c : public nocopy_c {
    public:
        color_c colors[Count];
        basic_palette_c() { memset(colors, 0, sizeof(colors)); }
        basic_palette_c(uint16_t *cs) { memcpy(colors, cs, sizeof(colors)); }
        basic_palette_c(uint8_t *c) {
            c += 3 * Count;
            for (int i = Count; --i != -1; ) {
                c -= 3;
                colors[i] = color_c(c[0], c[1], c[2]);
            }
        }
    };
        
    class palette_c : public basic_palette_c<16> {
    public:
        palette_c() : basic_palette_c<16>() {}
        palette_c(uint16_t *cs) : basic_palette_c<16>(cs) {}
        palette_c(uint8_t *c) : basic_palette_c<16>(c) {}

        void set_active() const;
    };
    
}
#endif /* graphics_hpp */
