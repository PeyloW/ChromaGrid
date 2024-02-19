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

static const uint8_t ste_to_seq[16] = { 0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15 };
static const uint8_t ste_from_seq[16] = { 0x00, 0x22, 0x44, 0x66, 0x88, 0xaa, 0xcc, 0xee,
                                          0x11, 0x33, 0x55, 0x77, 0x99, 0xbb, 0xdd, 0xff};

class cgcolor_c {
public:
    uint16_t color;
    cgcolor_c() = default;
    cgcolor_c(const uint8_t r, const uint8_t g, const uint8_t b) : color(to_ste(r, 8) | to_ste(g, 4) | to_ste(b, 0)) {}
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
private:
    __forceinline static uint16_t to_ste(const uint8_t c, const uint8_t shift) {
        return ste_to_seq[c >> 4] << shift;
    }
    __forceinline static uint8_t from_ste(const uint16_t c, const uint8_t shift) {
        return ste_from_seq[(c >> shift) & 0x0f];
    }
};


class cgpalette_c {
public:
    cgcolor_c colors[16];
    cgpalette_c(uint16_t *cs) {memcpy(colors, cs, sizeof(colors)); }
    cgpalette_c(uint8_t *c) {
        for (int i = 0; i < 16; i++) {
            colors[i] = cgcolor_c(c[0], c[1], c[2]);
            c += 3;
        }
    }
    void set_active() const;
};

typedef int8_t cgcolorindex_t;
static const cgcolorindex_t cgtransparent_colorindex = -1;

class __packed cgimage_c {
public:
    enum mask_mode_t {
        mask_mode_auto, mask_mode_none, mask_mode_masked
    };
    
    cgimage_c(const cgsize_t size, mask_mode_t mask_mode, cgpalette_c *palette);
    cgimage_c(const cgimage_c *image, cgrect_t rect);
    cgimage_c(const char *path, mask_mode_t mask_mode = mask_mode_auto);
    ~cgimage_c();
    
    void set_active() const;
    
    __forceinline cgpalette_c *get_palette() const { return _palette; }
    __forceinline cgpoint_t get_offset() const { return _offset; }
    __forceinline void set_offset(const cgpoint_t o) { _offset = o; }
    __forceinline cgsize_t get_size() const { return _size; }
    
    template<class Commands>
    __forceinline void with_clipping(bool clip, Commands commands) {
        const bool old_clip = _clipping;
        _clipping = clip;
        commands();
        _clipping = old_clip;
    }
    
    void put_pixel(cgcolorindex_t ci, cgpoint_t at);
    cgcolorindex_t get_pixel(cgpoint_t at);

    void fill(cgcolorindex_t ci, cgrect_t rect);
    
    void draw_aligned(cgimage_c *src, cgpoint_t at);
    void draw(cgimage_c *src, cgpoint_t at);
    void draw(cgimage_c *src, cgrect_t rect, cgpoint_t at);
    
private:
    // Must be __packed and syn with graphics_m68k.s implementation
    const cgimage_c *_super_image;
    cgpalette_c *_palette;
    uint16_t *_bitmap;
    uint16_t *_maskmap;
    cgsize_t _size;
    cgpoint_t _offset;
    uint16_t _line_words;
    bool _owns_bitmap;
    bool _clipping;
    
    static void imp_draw_aligned(cgimage_c *image, cgimage_c *srcImage, cgpoint_t point) asm("_m68_cgimage_draw_aligned");
    static void imp_draw_rect(cgimage_c *image, cgimage_c *srcImage, cgrect_t *const rect, cgpoint_t point) asm("_m68_cgimage_draw_rect");

};

#endif /* graphics_hpp */
