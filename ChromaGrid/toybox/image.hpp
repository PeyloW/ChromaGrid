//
//  image.hpp
//  toybox
//
//  Created by Fredrik on 2024-04-11.
//

#ifndef image_hpp
#define image_hpp

#include "palette.hpp"
#include "memory.hpp"
#include "asset.hpp"

namespace toybox {
    
    using namespace toybox;
    
    typedef enum __packed {
        compression_type_none,
        compression_type_packbits,
        compression_type_vertical,  // Not supported
        compression_type_deflate    // Non-standard, not supported as of now
    } compression_type_e;

    /**
     An `image_c` is a read only representation of any graphics.
     An image with a mask is a sprite.
     Images can be loaded from EA 85 ILBM files, or created at runtime.
     On emulated host machines only, images can also be saved.
     TODO: Support bitplane layout other than interweaved.
     */
    class image_c : public asset_c {
        friend class canvas_c;
        friend class machine_c;
    public:
        static const int MASKED_CIDX = -1;
        static constexpr bool is_masked(int i) __pure { return i < 0; }
        typedef enum __packed {
            interweaved, interleaved, continious
        } bitplane_layout_e;
        
        image_c(const size_s size, bool masked, shared_ptr_c<palette_c> palette);
        image_c(const char *path, int masked_cidx = MASKED_CIDX);
        virtual ~image_c() {};
        
        type_e asset_type() const { return image; }

#if TOYBOX_IMAGE_SUPPORTS_SAVE
        bool save(const char *path, compression_type_e compression, bool masked, int masked_cidx = MASKED_CIDX);
#endif
                
        __forceinline void set_palette(const shared_ptr_c<palette_c> &palette) {
            _palette = palette;
        }
        __forceinline shared_ptr_c<palette_c> &palette() const {
            return *(shared_ptr_c<palette_c>*)&_palette;
        }
        __forceinline size_s size() const { return _size; }
        __forceinline bool masked() const { return _maskmap != nullptr; }
        __forceinline bitplane_layout_e layout() const { return interweaved; }

        int get_pixel(point_s at) const;
        
    private:
        shared_ptr_c<palette_c> _palette;
        unique_ptr_c<uint16_t> _bitmap;
        uint16_t *_maskmap;
        size_s _size;
        uint16_t _line_words;
    };
    
}

#endif /* image_hpp */
