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

    static_assert(TOYBOX_DIRTYMAP_TILE_SIZE.width % 16 == 0, "Tile width must be a multiple of 16");
    
    class dirtymap_c : public nocopy_c {
        friend class canvas_c;
    public:
        void mark(const rect_s &rect);
        void merge(const dirtymap_c &dirtymap);
        void restore(canvas_c &canvas, const image_c &clean_image);
        void clear();
#if TOYBOX_DEBUG_DIRTYMAP
        void debug(const char *name) const;
#endif
    private:
        static int instance_size(size_s *size);
        dirtymap_c(const size_s size) : _size(size) {}
        const size_s _size;
        uint8_t _data[];
    };
    
}

#endif /* dirtymap_h */
