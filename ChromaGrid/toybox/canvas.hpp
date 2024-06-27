//
//  canvas.hpp
//  toybox
//
//  Created by Fredrik on 2024-04-10.
//

#ifndef canvas_hpp
#define canvas_hpp

#include "image.hpp"
#include "tileset.hpp"
#include "font.hpp"
#include "dirtymap.hpp"

namespace toybox {
    
    /**
     A `canvas_c` is a wrapper for an `image_c` to provide drawing operations.
     Only the private functions prefixed with `imp_` needs to be reimplemented
     for each target, all other functions should be abstractions ontop.
     TODO: Only Atari STe with blitter and interweaved bitplanes supported.
     TODO: Remove stencil, too ChromaGrid/STe specific?
     */
    class canvas_c : public nocopy_c {
    public:
        class remap_table_c : nocopy_c {
        public:
            remap_table_c() { for (int i = -1; i < 16; i++) (*this)[i] = i; }
            int& operator[](int i) { return _table[i + 1]; }
            const int& operator[](int i) const { return _table[i + 1]; }
        private:
            int _table[17];
        };
        
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

        image_c &image() const { return _image; }
        size_s size() const { return _image.size(); }
        
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
        static const canvas_c::stencil_t *const stencil(stencil_type_e type, int shade);
        
        dirtymap_c *create_dirtymap() const __pure;
        template<class Commands>
        __forceinline void with_dirtymap(dirtymap_c *dirtymap, Commands commands) {
            dirtymap_c *old_dirtymap = _dirtymap;
            _dirtymap = dirtymap;
            commands();
            _dirtymap = old_dirtymap;
        }
        
        void put_pixel(int ci, point_s at) const;
        
        void remap_colors(const remap_table_c &table, rect_s rect) const;
        
        static void make_stencil(stencil_t stencil, stencil_type_e type, int shade);
        
        void fill(uint8_t ci, rect_s rect) const;
        
        void draw_aligned(const image_c &src, point_s at) const;
        void draw_aligned(const image_c &src, rect_s rect, point_s at) const;
        void draw_aligned(const tileset_c &src, int idx, point_s at) const;
        void draw_aligned(const tileset_c &src, point_s tile, point_s at) const;
        void draw(const image_c &src, point_s at, const int color = image_c::MASKED_CIDX) const;
        void draw(const image_c &src, rect_s rect, point_s at, const int color = image_c::MASKED_CIDX) const;
        void draw(const tileset_c &src, int idx, point_s at, const int color = image_c::MASKED_CIDX) const;
        void draw(const tileset_c &src, point_s tile, point_s at, const int color = image_c::MASKED_CIDX) const;

        
        
        void draw_3_patch(const image_c &src, int16_t cap, rect_s in) const;
        void draw_3_patch(const image_c &src, rect_s rect, int16_t cap, rect_s in) const;
        
        size_s draw(const font_c &font, const char *text, point_s at, text_alignment_e alignment = align_center, const int color = image_c::MASKED_CIDX) const;
        size_s draw(const font_c &font, const char *text, rect_s in, uint16_t line_spacing = 0, text_alignment_e alignment = align_center, const int color = image_c::MASKED_CIDX) const;
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
