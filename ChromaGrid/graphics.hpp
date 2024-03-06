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

class cgcolor_c {
public:
    uint16_t color;
    cgcolor_c() = default;
    cgcolor_c(uint16_t c) : color(c) {}
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
        static const uint8_t STE_TO_SEQ[16] = { 0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15 };
        return STE_TO_SEQ[c >> 4] << shift;
    }
    __forceinline static uint8_t from_ste(const uint16_t c, const uint8_t shift) {
        static const uint8_t STE_FROM_SEQ[16] = { 0x00, 0x22, 0x44, 0x66, 0x88, 0xaa, 0xcc, 0xee,
                                                  0x11, 0x33, 0x55, 0x77, 0x99, 0xbb, 0xdd, 0xff};
        return STE_FROM_SEQ[(c >> shift) & 0x0f];
    }
};


class cgpalette_c : private cgnocopy_c {
public:
    cgcolor_c colors[16];
    cgpalette_c(uint16_t *cs) {memcpy(colors, cs, sizeof(colors)); }
    cgpalette_c(uint8_t *c) {
        c += 3 * 16;
        for (int i = 16; --i != -1; ) {
            c -= 3;
            colors[i] = cgcolor_c(c[0], c[1], c[2]);
        }
    }
    void set_active() const;
};


class cgfont_c;

class cgimage_c : private cgnocopy_c {
public:
    static const uint8_t MASKED_CIDX = 0x10;
    typedef uint8_t remap_table_t[17];
    
    static const int STENCIL_FULLY_TRANSPARENT = 0;
    static const int STENCIL_FULLY_OPAQUE = 64;
    typedef uint16_t stencil_t[16];
    typedef enum __packed {
        none,
        orderred,
        noise
    } stencil_type_e;
    
    typedef enum __packed {
        align_left,
        align_center,
        align_right
    } text_alignment_e;
    
    cgimage_c(const cgsize_t size, bool masked, cgpalette_c *palette);
    cgimage_c(const cgimage_c &image, cgrect_t rect);
    cgimage_c(const char *path, bool masked, uint8_t masked_cidx = MASKED_CIDX);
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

    template<class Commands>
    __forceinline void with_stencil(const stencil_t *const stencil, Commands commands) {
        const auto old_stencil = _stencil;
        _stencil = stencil;
        commands();
        _stencil = old_stencil;
    }
    static const cgimage_c::stencil_t *const get_stencil(stencil_type_e type, int shade);
    
    size_t dirtymap_size() const {
        return _line_words * _size.height / 16;
    };
    template<class Commands>
    __forceinline void with_dirtymap(bool *const dirtymap, Commands commands) {
        bool *const old_dirtymap = _dirtymap;
        _dirtymap = dirtymap;
        commands();
        _dirtymap = old_dirtymap;
    }
    void restore(const cgimage_c &clean_image, bool *const dirtymap) const;
    void merge_dirtymap(bool *dest, const bool *source) const;
#ifndef __M68000__
#if DEBUG_DIRTYMAP
    void debug_dirtymap(bool *const dirtymap, const char *name) const;
#else
    void debug_dirtymap(bool *const dirtymap, const char *name) const {};
#endif
#endif
    
    void put_pixel(uint8_t ci, cgpoint_t at) const;
    uint8_t get_pixel(cgpoint_t at) const;

    inline static void make_noremap_table(remap_table_t table) {
        for (int i = MASKED_CIDX + 1; --i != -1; ) {
            table[i] = i;
        }
    }
    void remap_colors(remap_table_t table, cgrect_t rect) const;
    
    static void make_stencil(stencil_t stencil, stencil_type_e type, int shade);
    
    void fill(uint8_t ci, cgrect_t rect) const;
    
    void draw_aligned(const cgimage_c &src, cgpoint_t at) const;
    void draw_aligned(const cgimage_c &src, cgrect_t rect, cgpoint_t at) const;
    void draw(const cgimage_c &src, cgpoint_t at, const uint8_t color = MASKED_CIDX) const;
    void draw(const cgimage_c &src, cgrect_t rect, cgpoint_t at, const uint8_t color = MASKED_CIDX) const;
    
    void draw_3_patch(const cgimage_c &src, int16_t cap, cgrect_t in) const;
    void draw_3_patch(const cgimage_c &src, cgrect_t rect, int16_t cap, cgrect_t in) const;
    
    void draw(const cgfont_c &font, const char *text, cgpoint_t at, text_alignment_e alignment = align_center, const uint8_t color = MASKED_CIDX) const;
    void draw(const cgfont_c &font, const char *text, cgrect_t in, uint16_t line_spacing = 0, text_alignment_e alignment = align_center, const uint8_t color = MASKED_CIDX) const;
private:
    const cgimage_c *_super_image;
    cgpalette_c *_palette;
    uint16_t *_bitmap;
    uint16_t *_maskmap;
    bool *_dirtymap;
    const stencil_t *_stencil;
    cgsize_t _size;
    cgpoint_t _offset;
    uint16_t _line_words;
    bool _owns_bitmap;
    bool _clipping;

    inline void imp_update_dirtymap(cgrect_t rect) const;
        
    void imp_draw_aligned(const cgimage_c &srcImage, const cgrect_t &rect, cgpoint_t point) const;
    void imp_draw(const cgimage_c &srcImage, const cgrect_t &rect, cgpoint_t point) const;
    void imp_draw_masked(const cgimage_c &srcImage, const cgrect_t &rect, cgpoint_t point) const;
    void imp_draw_color(const cgimage_c &srcImage, const cgrect_t &rect, cgpoint_t point, uint16_t color) const;
    
    void imp_draw_rect_SLOW(const cgimage_c &srcImage, const cgrect_t &rect, cgpoint_t point) const;
};

class cgfont_c {
public:
    cgfont_c(const cgimage_c &image, cgsize_t character_size);
    cgfont_c(const cgimage_c &image, cgsize_t max_size, uint8_t space_width, uint8_t lead_req_space, uint8_t trail_rew_space);

    inline const cgimage_c &get_image() const {
        return _image;
    }
    inline const cgrect_t &get_rect(const char c) const {
        if (c < 32 || c > 127) {
            return _rects[0];
        } else {
            return _rects[c - 32];
        }
    }
    
private:
    cgfont_c() = delete;
    cgfont_c(const cgfont_c &) = delete;
    cgfont_c(cgfont_c &&) = delete;
    
    const cgimage_c &_image;
    cgrect_t _rects[96];
};

#endif /* graphics_hpp */
