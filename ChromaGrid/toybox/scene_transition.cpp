//
//  scene_transition.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-03-23.
//

#include <stdio.h>

#include "scene.hpp"

using namespace toybox;

namespace toybox {
    
    class dither_transition_c : public transition_c {
    public:
        dither_transition_c(image_c::stencil_type_e dither, uint8_t through) : transition_c() {
            _transition_state.full_restores_left = 2;
            _transition_state.type = dither;
            _transition_state.shade = 0;
        }
        
        virtual bool tick(image_c &phys_screen, image_c &log_screen, int ticks) {
            auto shade = MIN(image_c::STENCIL_FULLY_OPAQUE, _transition_state.shade);
            phys_screen.with_stencil(image_c::get_stencil(_transition_state.type, shade), [this, &phys_screen, &log_screen] {
                phys_screen.draw_aligned(log_screen, (point_s){0, 0});
            });
            if (shade == image_c::STENCIL_FULLY_OPAQUE) {
                _transition_state.full_restores_left--;
            }
            _transition_state.shade += 1 + MAX(1, ticks);
            return _transition_state.full_restores_left <= 0;
        }
    private:
        struct {
            int full_restores_left;
            image_c::stencil_type_e type;
            int shade;
        } _transition_state;
    };
}

transition_c *transition_c::create(image_c::stencil_type_e dither, uint8_t through) {
    return new dither_transition_c(dither, through);
}

transition_c *create(color_c through) {
    return nullptr;;
}
