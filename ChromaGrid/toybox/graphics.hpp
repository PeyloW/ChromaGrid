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

#define DEBUG_DIRTYMAP 0

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
            reinterpret_cast<uint16_t*>(0xffff8240)[i] = color;
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
        
    class palette_c : public nocopy_c {
    public:
        color_c colors[16];
        palette_c() { memset(colors, 0, sizeof(colors)); }
        palette_c(uint16_t *cs) { memcpy(colors, cs, sizeof(colors)); }
        palette_c(uint8_t *c) {
            c += 3 * 16;
            for (int i = 16; --i != -1; ) {
                c -= 3;
                colors[i] = color_c(c[0], c[1], c[2]);
            }
        }
        void set_active() const;
    };
    
    
    class image_c : public nocopy_c {\
        friend class canvas_c;
    public:
        static const uint8_t MASKED_CIDX = 0x10;
        
        image_c(const size_s size, bool masked, palette_c *palette);
        image_c(const char *path, bool masked, uint8_t masked_cidx = MASKED_CIDX);
        ~image_c();

        void set_active() const;

#ifdef CGIMAGE_SUPPORT_SAVE
        bool save(const char *path, bool compressed, bool masked, uint8_t masked_cidx = MASKED_CIDX);
#endif
                
        __forceinline palette_c *get_palette() const { return _palette; }
        __forceinline size_s get_size() const { return _size; }
        uint8_t get_pixel(point_s at) const;
        
    private:
        palette_c *_palette;
        uint16_t *_bitmap;
        uint16_t *_maskmap;
        size_s _size;
        uint16_t _line_words;
    };
        
    class font_c : public nocopy_c {
    public:
        font_c(const image_c &image, size_s character_size);
        font_c(const image_c &image, size_s max_size, uint8_t space_width, uint8_t lead_req_space, uint8_t trail_rew_space);
        
        inline const image_c &get_image() const {
            return _image;
        }
        inline const rect_s &get_rect(const char c) const {
            if (c < 32 || c > 127) {
                return _rects[0];
            } else {
                return _rects[c - 32];
            }
        }
        
    private:
        const image_c &_image;
        rect_s _rects[96];
    };
        
}
#endif /* graphics_hpp */
