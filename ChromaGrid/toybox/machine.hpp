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
    
    /**
     A `machine_c` is an abstraction for the target machine and OS.
     An emulation host such as macOS is **not** a machine of its own.
     */
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
        void free_system_memory();

        uint32_t get_cookie(uint32_t cookie, uint32_t def_value = 0) const __pure;

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
