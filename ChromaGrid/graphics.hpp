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

class cgcolor_t {
public:
    uint16_t color;
    cgcolor_t() = default;
    cgcolor_t(const uint8_t r, const uint8_t g, const uint8_t b) : color(to_ste(r, 8) | to_ste(g, 4) | to_ste(b, 0)) {}
    void set_at(const int i) const {
#ifdef __M68000__
        reinterpret_cast<uint16_t*>(0xffff8240)[i] = color;
#endif
    }
    void get(uint8_t *r, uint8_t *g, uint8_t *b) const asm("_cgcolor_get") {
        *r = from_ste(color, 8);
        *g = from_ste(color, 4);
        *b = from_ste(color, 0);
    }
private:
    static uint16_t to_ste(const uint8_t c, const uint8_t shift)  asm("_cgcolor_to_ste"){
        return ste_to_seq[c >> 4] << shift;
    }
    static uint8_t from_ste(const uint16_t c, const uint8_t shift)  asm("_cgcolor_from_ste") {
        return ste_from_seq[(c >> shift) & 0x0f];
    }
};


class cgpalette_t {
public:
    cgcolor_t colors[16];
    cgpalette_t(uint16_t *cs) {memcpy(colors, cs, sizeof(colors)); }
    cgpalette_t(uint8_t *c) asm("_cgpalette_init") {
        for (int i = 0; i < 16; i++) {
            colors[i] = cgcolor_t(c[0], c[1], c[2]);
            c += 3;
        }
    }
    void set_active() const asm("_cgpalette_set_active");
};

typedef int8_t colorindex_t;
static const colorindex_t transparent_colorindex = -1;

class __packed cgimage_t {
public:
    enum mask_mode_t {
        mask_mode_auto, mask_mode_none, mask_mode_masked
    };
    
    cgimage_t(const cgsize_t size, mask_mode_t mask_mode, cgpalette_t *palette) asm("_cgimage_init");
    cgimage_t(const cgimage_t *image, cgrect_t rect) asm("_cgimage_init_subimage");
    cgimage_t(const char *path, mask_mode_t mask_mode = mask_mode_auto) asm("_cgimage_init_path");
    ~cgimage_t() asm("_cgimage_deinit");
    
    void set_active() const asm("_cgimage_set_active");
    
    inline cgpalette_t *get_palette() const { return palette; }
    inline cgpoint_t get_offset() const { return offset; }
    inline void set_offset(const cgpoint_t o) { offset = o; }
    inline cgsize_t get_size() const { return size; }
    
    template<class Commands>
    inline void with_clipping(bool clip, Commands commands) {
        const bool old_clip = clipping;
        clipping = clip;
        commands();
        clipping = old_clip;
    }
    
    void put_pixel(colorindex_t ci, cgpoint_t at) asm("_cgimage_put_pixel");
    colorindex_t get_pixel(cgpoint_t at) asm("_cgimage_get_pixel");

    void fill(colorindex_t ci, cgrect_t rect) asm("_cgimage_fill");
    
    void draw_aligned(cgimage_t *src, cgpoint_t at) asm("_cgimage_draw_aligned");
    void draw(cgimage_t *src, cgpoint_t at) asm("_cgimage_draw");
    void draw(cgimage_t *src, cgrect_t rect, cgpoint_t at) asm("_cgimage_draw_rect");
    
private:
    const cgimage_t *super_image;
    cgpalette_t *palette;
    uint16_t *bitmap;
    uint16_t *maskmap;
    cgsize_t size;
    cgpoint_t offset;
    uint16_t line_words;
    bool owns_bitmap;
    bool clipping;
    
    static void imp_draw_aligned(cgimage_t *image, cgimage_t *srcImage, cgpoint_t point) asm("_m68_cgimage_draw_aligned");
    static void imp_draw(cgimage_t *image, cgimage_t *srcImage, cgpoint_t point) asm("_m68_cgimage_draw");
    static void imp_draw_rect(cgimage_t *image, cgimage_t *srcImage, cgrect_t *const rect, cgpoint_t point) asm("_m68_cgimage_draw_rect");

};

#endif /* graphics_hpp */
