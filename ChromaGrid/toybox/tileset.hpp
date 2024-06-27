//
//  tileset.hpp
//  toybox
//
//  Created by Fredrik on 2024-04-17.
//

#ifndef tileset_hpp
#define tileset_hpp

#include "image.hpp"

namespace toybox {
    
    using namespace toybox;
 
    class tileset_c : public asset_c {
    public:
        tileset_c(const shared_ptr_c<image_c> &image, size_s tile_size);
        virtual ~tileset_c() {};
        
        type_e asset_type() const { return tileset; }
        
        inline const shared_ptr_c<image_c> &image() const __pure {
            return _image;
        }
        
        size_s tile_size() const __pure { return _rects[0].size; }
        
        int16_t max_index() const __pure;
        point_s max_tile() const __pure;

        const rect_s &tile_rect(const int16_t i) const __pure;
        const rect_s &tile_rect(const point_s tile) const __pure;

    private:
        const shared_ptr_c<image_c> _image;
        const point_s _max_tile;
        unique_ptr_c<rect_s> _rects;
    };

}
#endif /* tileset_hpp */
