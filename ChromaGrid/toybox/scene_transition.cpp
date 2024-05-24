//
//  scene_transition.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-03-23.
//

#include "scene.hpp"
#include "machine.hpp"

using namespace toybox;

namespace toybox {
    
    class dither_transition_c : public transition_c {
    public:
        dither_transition_c(canvas_c::stencil_type_e dither) : transition_c(), _palette(nullptr) {
            _transition_state.full_restores_left = 2;
            _transition_state.type = canvas_c::effective_type(dither);
            _transition_state.shade = 0;
        }

        virtual void will_begin(const scene_c *from, const scene_c *to) {
            if (to) {
                _palette = &to->configuration().palette;
                if (from) {
                    assert(_palette == &from->configuration().palette);
                }
            }
        }

        virtual bool tick(screen_c &phys_screen, screen_c &log_screen, int ticks) {
            if (_transition_state.shade == 0 && _palette) {
                machine_c::shared().set_active_palette(_palette);
                _palette = nullptr;
            }
            auto shade = MIN(canvas_c::STENCIL_FULLY_OPAQUE, _transition_state.shade);
            phys_screen.canvas().with_stencil(canvas_c::stencil(_transition_state.type, shade), [this, &phys_screen, &log_screen] {
                phys_screen.canvas().draw_aligned(log_screen.image(), point_s());
            });
            if (shade == canvas_c::STENCIL_FULLY_OPAQUE) {
                _transition_state.full_restores_left--;
            }
            _transition_state.shade += 1 + MAX(1, ticks);
            return _transition_state.full_restores_left <= 0;
        }
    protected:
        const palette_c *_palette;
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
        dither_transition_c(dither), _palette(nullptr), _through(through) {
            _transition_state.full_restores_left = 4;
        }
    
    virtual bool tick(screen_c &phys_screen, screen_c &log_screen, int ticks) {
        if (_transition_state.full_restores_left > 2) {
            auto shade = MIN(canvas_c::STENCIL_FULLY_OPAQUE, _transition_state.shade);
            phys_screen.canvas().with_stencil(canvas_c::stencil(_transition_state.type, shade), [this, &phys_screen, &log_screen] {
                phys_screen.canvas().fill(_through, rect_s(point_s(), phys_screen.size()));
            });
            if (shade == canvas_c::STENCIL_FULLY_OPAQUE) {
                if (_palette) {
                    machine_c::shared().set_active_palette(_palette);
                    _palette = nullptr;
                }
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
    const palette_c *_palette;
    const uint8_t _through;
};

class fade_through_transition_c : public transition_c {
public:
    fade_through_transition_c(color_c through) :
        transition_c(), _to_palette(nullptr), _through(through), _count(0)
    {}
    virtual ~fade_through_transition_c() {
        if (_to_palette) {
            machine_c::shared().set_active_palette(_to_palette);
        }
    }
    virtual void will_begin(const scene_c *from, const scene_c *to) {
        assert(from && to);
        uint8_t r, g, b;
        _through.get(&r, &g, &b);
        const palette_c &from_palette = from->configuration().palette;
        const palette_c &to_palette = to->configuration().palette;
        _to_palette = &to_palette;
        for (int i = 0; i <= 16; i++) {
            _palettes.emplace_back();
            auto &palette = _palettes.back();
            int shade = i * color_c::MIX_FULLY_OTHER / 16;
            for (int j = 0; j < 16; j++) {
                palette.colors[j] = from_palette.colors[j].mix(_through, shade);
            }
        }
        for (int i = 15; i >= 0; i--) {
            _palettes.emplace_back();
            auto &palette = _palettes.back();
            int shade = i * color_c::MIX_FULLY_OTHER / 16;
            for (int j = 0; j < 16; j++) {
                palette.colors[j] = to_palette.colors[j].mix(_through, shade);
            }
        }
    }
    virtual bool tick(screen_c &phys_screen, screen_c &log_screen, int ticks) {
        const int count = _count / 2;
        auto &m = machine_c::shared();
        if (count < 17) {
            m.set_active_palette(&_palettes[count]);
        } else if (count < 18) {
            phys_screen.canvas().draw_aligned(log_screen.image(), point_s());
        } else if (count < 34) {
            m.set_active_palette(&_palettes[count - 1]);
        } else {
            return true;
        }
        _count++;
        return false;
    }
private:
    const palette_c *_to_palette;
    const color_c _through;
    int _count;
    vector_c<palette_c, 33> _palettes;
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
