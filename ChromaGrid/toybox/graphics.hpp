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
        
    class palette_c : private nocopy_c {
    public:
        color_c colors[16];
        palette_c(uint16_t *cs) {memcpy(colors, cs, sizeof(colors)); }
        palette_c(uint8_t *c) {
            c += 3 * 16;
            for (int i = 16; --i != -1; ) {
                c -= 3;
                colors[i] = color_c(c[0], c[1], c[2]);
            }
        }
        void set_active() const;
    };
    
    
    class font_c;
    class dirtymap_c;
    
    class image_c : private nocopy_c {
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
        
        image_c(const size_s size, bool masked, palette_c *palette);
        image_c(const image_c &image, rect_s rect);
        image_c(const char *path, bool masked, uint8_t masked_cidx = MASKED_CIDX);
        ~image_c();
        
#ifdef CGIMAGE_SUPPORT_SAVE
        bool save(const char *path, bool compressed, bool masked, uint8_t masked_cidx = MASKED_CIDX);
#endif
        
        void set_active() const;
        
        __forceinline palette_c *get_palette() const { return _palette; }
        __forceinline point_s get_offset() const { return _offset; }
        __forceinline void set_offset(const point_s o) { _offset = o; }
        __forceinline size_s get_size() const { return _size; }
        
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
        static const image_c::stencil_t *const get_stencil(stencil_type_e type, int shade);
        
        size_t dirtymap_size() const {
            return _line_words * _size.height / 16;
        };
        template<class Commands>
        __forceinline void with_dirtymap(dirtymap_c *dirtymap, Commands commands) {
            dirtymap_c *old_dirtymap = _dirtymap;
            _dirtymap = dirtymap;
            commands();
            _dirtymap = old_dirtymap;
        }
        void restore(const image_c &clean_image, bool *const dirtymap) const;
        void merge_dirtymap(bool *dest, const bool *source) const;
#ifndef __M68000__
#if DEBUG_DIRTYMAP
        void debug_dirtymap(bool *const dirtymap, const char *name) const;
#else
        void debug_dirtymap(bool *const dirtymap, const char *name) const {};
#endif
#endif
        
        void put_pixel(uint8_t ci, point_s at) const;
        uint8_t get_pixel(point_s at) const;
        
        inline static void make_noremap_table(remap_table_t table) {
            for (int i = MASKED_CIDX + 1; --i != -1; ) {
                table[i] = i;
            }
        }
        void remap_colors(remap_table_t table, rect_s rect) const;
        
        static void make_stencil(stencil_t stencil, stencil_type_e type, int shade);
        
        void fill(uint8_t ci, rect_s rect) const;
        
        void draw_aligned(const image_c &src, point_s at) const;
        void draw_aligned(const image_c &src, rect_s rect, point_s at) const;
        void draw(const image_c &src, point_s at, const uint8_t color = MASKED_CIDX) const;
        void draw(const image_c &src, rect_s rect, point_s at, const uint8_t color = MASKED_CIDX) const;
        
        void draw_3_patch(const image_c &src, int16_t cap, rect_s in) const;
        void draw_3_patch(const image_c &src, rect_s rect, int16_t cap, rect_s in) const;
        
        size_s draw(const font_c &font, const char *text, point_s at, text_alignment_e alignment = align_center, const uint8_t color = MASKED_CIDX) const;
        size_s draw(const font_c &font, const char *text, rect_s in, uint16_t line_spacing = 0, text_alignment_e alignment = align_center, const uint8_t color = MASKED_CIDX) const;
    private:
        const image_c *_super_image;
        palette_c *_palette;
        uint16_t *_bitmap;
        uint16_t *_maskmap;
        dirtymap_c *_dirtymap;
        const stencil_t *_stencil;
        size_s _size;
        point_s _offset;
        uint16_t _line_words;
        bool _owns_bitmap;
        bool _clipping;
        
        void imp_fill(uint8_t ci, rect_s rect) const;
        void imp_draw_aligned(const image_c &srcImage, const rect_s &rect, point_s point) const;
        void imp_draw(const image_c &srcImage, const rect_s &rect, point_s point) const;
        void imp_draw_masked(const image_c &srcImage, const rect_s &rect, point_s point) const;
        void imp_draw_color(const image_c &srcImage, const rect_s &rect, point_s point, uint16_t color) const;
        
        void imp_draw_rect_SLOW(const image_c &srcImage, const rect_s &rect, point_s point) const;
    };
    
    class font_c : private nocopy_c {
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
        font_c() = delete;
        font_c(const font_c &) = delete;
        font_c(font_c &&) = delete;
        
        const image_c &_image;
        rect_s _rects[96];
    };
    
#ifndef CGDIRTYMAP_TILE_WIDTH
#   define CGDIRTYMAP_TILE_WIDTH (16)
#endif
#ifndef CGDIRTYMAP_TILE_HEIGHT
#   define CGDIRTYMAP_TILE_HEIGHT (16)
#endif
    static_assert(CGDIRTYMAP_TILE_WIDTH % 16 == 0, "Tile width must be a multiple of 16");
    
    class dirtymap_c : private nocopy_c {
    public:
        static dirtymap_c *create(const image_c &image);
        
        void mark(const rect_s &rect);
        void merge(const dirtymap_c &dirtymap);
        void restore(image_c &image, const image_c &clean_image);
        void clear();
#ifndef __M68000__
#if DEBUG_DIRTYMAP
        void debug(const char *name) const;
#endif
#endif
    private:
        dirtymap_c(const size_s size) : _size(size) {}
        const size_s _size;
        uint8_t _data[];
    };
    
}
#endif /* graphics_hpp */
