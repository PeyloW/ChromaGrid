//
//  scene_transition.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-03-23.
//

#include "scene.hpp"

using namespace toybox;

namespace toybox {
    
    class dither_transition_c : public transition_c {
    public:
        dither_transition_c(canvas_c::stencil_type_e dither) : transition_c() {
            _transition_state.full_restores_left = 2;
            _transition_state.type = canvas_c::effective_type(dither);
            _transition_state.shade = 0;
        }
        
        virtual bool tick(screen_c &phys_screen, screen_c &log_screen, int ticks) {
            auto shade = MIN(canvas_c::STENCIL_FULLY_OPAQUE, _transition_state.shade);
            phys_screen.get_canvas().with_stencil(canvas_c::get_stencil(_transition_state.type, shade), [this, &phys_screen, &log_screen] {
                phys_screen.get_canvas().draw_aligned(log_screen.get_image(), point_s());
            });
            if (shade == canvas_c::STENCIL_FULLY_OPAQUE) {
                _transition_state.full_restores_left--;
            }
            _transition_state.shade += 1 + MAX(1, ticks);
            return _transition_state.full_restores_left <= 0;
        }
    protected:
        struct {
            int full_restores_left;
            canvas_c::stencil_type_e type;
            int shade;
        } _transition_state;
    };
}

class dither_through_transition_c : public dither_transition_c {
public:
    dither_through_transition_c(canvas_c::stencil_type_e dither, uint8_t through) :
        dither_transition_c(dither), _through(through) {
            _transition_state.full_restores_left = 4;
        }
    
    virtual bool tick(screen_c &phys_screen, screen_c &log_screen, int ticks) {
        if (_transition_state.full_restores_left > 2) {
            auto shade = MIN(canvas_c::STENCIL_FULLY_OPAQUE, _transition_state.shade);
            phys_screen.get_canvas().with_stencil(canvas_c::get_stencil(_transition_state.type, shade), [this, &phys_screen, &log_screen] {
                phys_screen.get_canvas().fill(_through, rect_s(point_s(), phys_screen.get_size()));
            });
            if (shade == canvas_c::STENCIL_FULLY_OPAQUE) {
                _transition_state.full_restores_left--;
            }
            if (_transition_state.full_restores_left > 2) {
                _transition_state.shade += 1 + MAX(1, ticks);
            } else {
                _transition_state.shade = 0;
            }
            return false;
        } else {
            return dither_transition_c::tick(phys_screen, log_screen, ticks);
        }
    }
protected:
    const uint8_t _through;
};

class fade_through_transition_c : public transition_c {
public:
    fade_through_transition_c(color_c through) : transition_c() {
        _count = 0;
        uint8_t r, g, b;
        through.get(&r, &g, &b);
        _old_active = g_active_palette;
        assert(_old_active);
        for (int i = 0; i <= 16; i++) {
            _palettes.emplace_back();
            auto &palette = _palettes.back();
            int shade = i * color_c::MIX_FULLY_OTHER / 16;
            for (int j = 0; j < 16; j++) {
                palette.colors[j] = _old_active->colors[j].mix(through, shade);
            }
        }
    }
    virtual ~fade_through_transition_c() {
        _old_active->set_active();
    }
    virtual bool tick(screen_c &phys_screen, screen_c &log_screen, int ticks) {
        const int count = _count / 2;
        if (count < 17) {
            _palettes[count].set_active();
        } else if (count < 18) {
            phys_screen.get_canvas().draw_aligned(log_screen.get_image(), point_s());
        } else if (count < 35) {
            _palettes[34 - count].set_active();
        } else {
            return true;
        }
        _count++;
        return false;
    }
private:
    const palette_c *_old_active;
    int _count;
    vector_c<palette_c, 17> _palettes;
};

transition_c *transition_c::create(canvas_c::stencil_type_e dither) {
    return new dither_transition_c(dither);
}

transition_c *transition_c::create(canvas_c::stencil_type_e dither, uint8_t through) {
    return new dither_through_transition_c(dither, through);
}

transition_c *transition_c::create(color_c through) {
    return new fade_through_transition_c(through);
}
