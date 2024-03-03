//
//  game.hpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-03.
//

#ifndef game_hpp
#define game_hpp

#include "grid.hpp"
#include "scene.hpp"
#include "resources.hpp"

struct cgbutton_t : cgnocopy_c {
    typedef enum __packed {
        normal,
        pressed,
        disabled
    } state_t;
    inline cgbutton_t() : text("") {}
    inline cgbutton_t(const char *text, cgrect_t rect) : text(text), rect(rect), state(normal) {}

    void draw_in(cgimage_c &image) const;
    
    const char *text;
    cgrect_t rect;
    state_t state;
};

template<int BUTTON_COUNT>
class cggame_scene_c : public cgscene_c {
public:
    cggame_scene_c(cgmanager_c &manager) :
        cgscene_c(manager),
        rsc(cgresources_c::shared()),
        _tracked_button(-1) {};
    const cgresources_c &rsc;

protected:
    int update_buttons(cgimage_c &screen) {
        const auto pos = manager.mouse.get_postion();
        const auto state = manager.mouse.get_state(cgmouse_c::left);
        update_tracked_button(screen, pos, state);
        if (state != cgmouse_c::released) {
            int idx = 0;
            for (cgbutton_t *button = _buttons.begin(); button != _buttons.end(); button++, idx++) {
                if (button->state == cgbutton_t::disabled) {
                    continue;
                }
                if (button->rect.contains(pos)) {
                    if (state == cgmouse_c::pressed) {
                        if (button->state == cgbutton_t::normal) {
                            button->state = cgbutton_t::pressed;
                            button->draw_in(screen);
                            _tracked_button = idx;
                        }
                        break;
                    } else {
                        assert(button->state == cgbutton_t::pressed);
                        button->state = cgbutton_t::normal;
                        button->draw_in(screen);
                        return idx;
                    }
                }
            }
        }
        return -1;
    }
    cgvector_c<cgbutton_t, BUTTON_COUNT> _buttons;

private:
    inline void update_tracked_button(cgimage_c &screen, const cgpoint_t pos, cgmouse_c::state_e state) {
        if (_tracked_button >= 0) {
            auto &button = _buttons[_tracked_button];
            //assert(button.state == cgbutton_t::pressed);
            if (state == cgmouse_c::released || !button.rect.contains(pos)) {
                button.state = cgbutton_t::normal;
                button.draw_in(screen);
                _tracked_button = -1;
            }
        }
    }
    int _tracked_button;
};

class cgoverlay_scene_c : public cggame_scene_c<0> {
public:
    cgoverlay_scene_c(cgmanager_c &manager) : cggame_scene_c(manager) {}
    virtual void will_appear(cgimage_c &screen, bool obsured);
    virtual void will_disappear(bool obscured);
    virtual void tick(cgimage_c &screen);
};

class cgintro_scene_c : public cggame_scene_c<6> {
public:
    cgintro_scene_c(cgmanager_c &manager);
    virtual void will_appear(cgimage_c &screen, bool obsured);
    virtual void tick(cgimage_c &screen);
};


#endif /* game_hpp */
