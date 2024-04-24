//
//  machine.hpp
//  toybox
//
//  Created by Fredrik on 2024-04-24.
//

#ifndef machine_hpp
#define machine_hpp

#include "cincludes.hpp"
#include "types.hpp"

namespace toybox {
    
    
    class palette_c;
    class image_c;
    
    class machine_c : public nocopy_c {
    public:
        typedef enum _packed {
            unknown,
#if TOYBOX_TARGET_ATARI
            st, ste, tt, falcon
#elif TOYBOY_TARGET_AMIGA
            osc, ecs, aga
#else
#   error "Unsupported target"
#endif
        } type_e;
        
        static machine_c &shared();
        
        type_e type() const __pure;
        size_s screen_size() const __pure;
        size_t max_memory() const __pure;
        size_t user_memory() const __pure;
        
        const image_c *active_image() const;
        void set_active_image(const image_c *image, point_s offset = point_s());
        const palette_c *active_palette() const;
        void set_active_palette(const palette_c *palette);
        
    private:
        machine_c();
        virtual ~machine_c();
#if TOYBOX_TARGET_ATARI
        uint32_t _old_super;
        uint16_t _old_modes[3];
#endif
    };
    
}

#endif /* machine_hpp */
