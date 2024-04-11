//
//  canvas.hpp
//  toybox
//
//  Created by Fredrik on 2024-04-10.
//

#ifndef canvas_hpp
#define canvas_hpp

#include "graphics.hpp"

namespace toybox {
    
    class canvas_c;
    
#ifndef CGDIRTYMAP_TILE_WIDTH
#   define CGDIRTYMAP_TILE_WIDTH (16)
#endif
#ifndef CGDIRTYMAP_TILE_HEIGHT
#   define CGDIRTYMAP_TILE_HEIGHT (16)
#endif
    static_assert(CGDIRTYMAP_TILE_WIDTH % 16 == 0, "Tile width must be a multiple of 16");
    
    class dirtymap_c : public nocopy_c {
    public:
        static dirtymap_c *create(const canvas_c &canvas);
        
        void mark(const rect_s &rect);
        void merge(const dirtymap_c &dirtymap);
        void restore(canvas_c &canvas, const image_c &clean_image);
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
    
    
    class canvas_c : public nocopy_c {
    public:
        typedef uint8_t remap_table_t[17];
        
        static const int STENCIL_FULLY_TRANSPARENT = 0;
        static const int STENCIL_FULLY_OPAQUE = 64;
        typedef uint16_t stencil_t[16];
        typedef enum __packed {
            none,
            orderred,
            noise,
            diagonal,
            circle,
            random
        } stencil_type_e;
        static stencil_type_e effective_type(stencil_type_e type);
        
        typedef enum __packed {
            align_left,
            align_center,
            align_right
        } text_alignment_e;
        
                
        canvas_c(image_c &image);
        ~canvas_c();

        image_c &get_image() const { return _image; }
        size_s get_size() const { return _image.get_size(); }
        
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
        static const canvas_c::stencil_t *const get_stencil(stencil_type_e type, int shade);
        
        template<class Commands>
        __forceinline void with_dirtymap(dirtymap_c *dirtymap, Commands commands) {
            dirtymap_c *old_dirtymap = _dirtymap;
            _dirtymap = dirtymap;
            commands();
            _dirtymap = old_dirtymap;
        }
        void restore(const canvas_c &clean_image, bool *const dirtymap) const;
        void merge_dirtymap(bool *dest, const bool *source) const;
#ifndef __M68000__
#if DEBUG_DIRTYMAP
        void debug_dirtymap(bool *const dirtymap, const char *name) const;
#else
        void debug_dirtymap(bool *const dirtymap, const char *name) const {};
#endif
#endif
        
        void put_pixel(uint8_t ci, point_s at) const;
        
        inline static void make_noremap_table(remap_table_t table) {
            for (int i = image_c::MASKED_CIDX + 1; --i != -1; ) {
                table[i] = i;
            }
        }
        void remap_colors(remap_table_t table, rect_s rect) const;
        
        static void make_stencil(stencil_t stencil, stencil_type_e type, int shade);
        
        void fill(uint8_t ci, rect_s rect) const;
        
        void draw_aligned(const image_c &src, point_s at) const;
        void draw_aligned(const image_c &src, rect_s rect, point_s at) const;
        void draw(const image_c &src, point_s at, const uint8_t color = image_c::MASKED_CIDX) const;
        void draw(const image_c &src, rect_s rect, point_s at, const uint8_t color = image_c::MASKED_CIDX) const;
        
        void draw_3_patch(const image_c &src, int16_t cap, rect_s in) const;
        void draw_3_patch(const image_c &src, rect_s rect, int16_t cap, rect_s in) const;
        
        size_s draw(const font_c &font, const char *text, point_s at, text_alignment_e alignment = align_center, const uint8_t color = image_c::MASKED_CIDX) const;
        size_s draw(const font_c &font, const char *text, rect_s in, uint16_t line_spacing = 0, text_alignment_e alignment = align_center, const uint8_t color = image_c::MASKED_CIDX) const;
    private:
        image_c &_image;
        dirtymap_c *_dirtymap;
        const stencil_t *_stencil;
        bool _clipping;
        
        void imp_fill(uint8_t ci, rect_s rect) const;
        void imp_draw_aligned(const image_c &srcImage, const rect_s &rect, point_s point) const;
        void imp_draw(const image_c &srcImage, const rect_s &rect, point_s point) const;
        void imp_draw_masked(const image_c &srcImage, const rect_s &rect, point_s point) const;
        void imp_draw_color(const image_c &srcImage, const rect_s &rect, point_s point, uint16_t color) const;
        
        void imp_draw_rect_SLOW(const image_c &srcImage, const rect_s &rect, point_s point) const;
    };

    
}

#endif /* canvas_hpp */
