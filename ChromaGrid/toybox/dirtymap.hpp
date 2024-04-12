//
//  dirtymap.hpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-04-11.
//

#ifndef dirtymap_h
#define dirtymap_h

#include "types.hpp"

namespace toybox {
    
    class image_c;
    class canvas_c;

#ifndef CGDIRTYMAP_TILE_WIDTH
#   define CGDIRTYMAP_TILE_WIDTH (16)
#endif
#ifndef CGDIRTYMAP_TILE_HEIGHT
#   define CGDIRTYMAP_TILE_HEIGHT (16)
#endif
    static_assert(CGDIRTYMAP_TILE_WIDTH % 16 == 0, "Tile width must be a multiple of 16");
    
    class dirtymap_c : public nocopy_c {
        friend class canvas_c;
    public:
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
        static int instance_size(size_s *size);
        dirtymap_c(const size_s size) : _size(size) {}
        const size_s _size;
        uint8_t _data[];
    };
    
}

#endif /* dirtymap_h */
