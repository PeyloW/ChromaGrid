//
//  tileset.hpp
//  toybox
//
//  Created by Fredrik on 2024-04-17.
//

#ifndef tileset_hpp
#define tileset_hpp

#include "image.hpp"
#include "memory.h"

namespace toybox {
    
    using namespace toystd;
 
    class tileset_c : public nocopy_c {
    public:
        tileset_c(const shared_ptr_c<image_c> &image, size_s tile_size);
        ~tileset_c() = default;
        
        inline const shared_ptr_c<image_c> &get_image() const __pure {
            return _image;
        }
        
        size_s get_tile_size() const __pure { return _rects[0].size; }
        
        int16_t max_index() const __pure;
        point_s max_tile() const __pure;

        const rect_s &get_rect(const int16_t i) const __pure;
        const rect_s &get_rect(const point_s tile) const __pure;

    private:
        const shared_ptr_c<image_c> _image;
        const point_s _max_tile;
        unique_ptr_c<rect_s> _rects;
    };

}
#endif /* tileset_hpp */