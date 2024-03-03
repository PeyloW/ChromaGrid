//
//  button.hpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-03-03.
//

#ifndef button_hpp
#define button_hpp

#include "types.hpp"
#include "resources.hpp"
#include "system.hpp"

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
class cgbutton_group_c : public cgnocopy_c {
public:
    cgbutton_group_c(cgpoint_t origin, cgsize_t size, int16_t spacing) :
        _tracked_button(-1),
        _size(size),
        _spacing(spacing)
    {
        _group_rect = (cgrect_t){ origin, { size.width, 0 }};
    }
    
    void add_button(const char *title) {
        cgrect_t rect = (cgrect_t){{_group_rect.origin.x, 0}, _size};
        int16_t expand = _size.height + (buttons.size() ? ABS(_spacing) : 0);
        if (_spacing < 0) {
            rect.origin.y = _group_rect.origin.y - expand;
            _group_rect.origin.y -= expand;
        } else {
            rect.origin.y = _group_rect.origin.y + _group_rect.size.height + expand - _spacing;
        }
        _group_rect.size.height += expand;
        buttons.emplace_back(title, rect);
    }
    
    void draw_all(cgimage_c &screen) {
        for (auto button = buttons.begin(); button != buttons.end(); button++) {
            button->draw_in(screen);
        }
    }
    
    int update_buttons(cgimage_c &screen, cgpoint_t pos, cgmouse_c::state_e state) {
        update_tracked_button(screen, pos, state);
        if (_group_rect.contains(pos)) {
            if (state != cgmouse_c::released) {
                int idx = 0;
                for (cgbutton_t *button = buttons.begin(); button != buttons.end(); button++, idx++) {
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
        }
        return -1;
    }

    cgvector_c<cgbutton_t, BUTTON_COUNT> buttons;

private:
    inline void update_tracked_button(cgimage_c &screen, const cgpoint_t pos, cgmouse_c::state_e state) {
        if (_tracked_button >= 0) {
            auto &button = buttons[_tracked_button];
            //assert(button.state == cgbutton_t::pressed);
            if (state == cgmouse_c::released || !button.rect.contains(pos)) {
                button.state = cgbutton_t::normal;
                button.draw_in(screen);
                _tracked_button = -1;
            }
        }
    }
    int _tracked_button;
    cgsize_t _size;
    int16_t _spacing;
    cgrect_t _group_rect;
};


#endif /* button_hpp */
