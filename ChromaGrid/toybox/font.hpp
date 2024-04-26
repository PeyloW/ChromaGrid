//
//  font.hpp
//  toybox
//
//  Created by Fredrik on 2024-04-11.
//

#ifndef font_hpp
#define font_hpp

#include "image.hpp"
#include "memory.h"

namespace toybox {
    
    using namespace toystd;
    
    
    class font_c : public asset_c {
    public:
        font_c(const shared_ptr_c<image_c> &image, size_s character_size);
        font_c(const shared_ptr_c<image_c> &image, size_s max_size, uint8_t space_width, uint8_t lead_req_space, uint8_t trail_rew_space);
        virtual ~font_c() {};
        
        type_e asset_type() const { return font; }
        size_t memory_cost() const { return _image->memory_cost(); }
        
        inline const shared_ptr_c<image_c> &image() const {
            return _image;
        }
        inline const rect_s &char_rect(const char c) const {
            if (c < 32 || c > 127) {
                return _rects[0];
            } else {
                return _rects[c - 32];
            }
        }
        
    private:
        const shared_ptr_c<image_c> _image;
        rect_s _rects[96];
    };
       
}

#endif /* font_hpp */
