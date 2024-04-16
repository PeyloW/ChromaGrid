//
//  image.hpp
//  toybox
//
//  Created by Fredrik on 2024-04-11.
//

#ifndef image_hpp
#define image_hpp

#include "palette.hpp"

#ifndef CGIMAGE_SUPPORT_SAVE
#   ifndef __M68000__
#       define CGIMAGE_SUPPORT_SAVE
#   endif
#endif

namespace toybox {
    
    using namespace toystd;
    
    class image_c : public nocopy_c {\
        friend class canvas_c;
    public:
        static const uint8_t MASKED_CIDX = 0x10;
        
        image_c(const size_s size, bool masked, palette_c *palette);
        image_c(const char *path, bool masked, uint8_t masked_cidx = MASKED_CIDX);
        ~image_c();

        void set_active() const;

#ifdef CGIMAGE_SUPPORT_SAVE
        bool save(const char *path, bool compressed, bool masked, uint8_t masked_cidx = MASKED_CIDX);
#endif
                
        __forceinline palette_c *get_palette() const { return _palette; }
        __forceinline size_s get_size() const { return _size; }
        uint8_t get_pixel(point_s at) const;
        
    private:
        palette_c *_palette;
        uint16_t *_bitmap;
        uint16_t *_maskmap;
        size_s _size;
        uint16_t _line_words;
    };
        
    
}

#endif /* image_hpp */
