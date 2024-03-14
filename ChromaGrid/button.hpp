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

class cgbutton_group_base_c : public cgnocopy_c {
    protected:
    cgbutton_group_base_c(cgpoint_t origin, cgsize_t size, int16_t spacing) :
        _tracked_button(-1),
        _size(size),
        _spacing(spacing)
    {
        _group_rect = (cgrect_t){ origin, { size.width, 0 }};
    }

    cgrect_t next_button_rect(bool first);
    void next_button_pair_rects(bool first, cgrect_t &left_rect, cgrect_t &right_rect, int16_t spacing);
    int update_button_range(cgbutton_t *begin, cgbutton_t *end, const cgpoint_t &pos, cgimage_c &screen, cgmouse_c::state_e state);
    
    int _tracked_button;
    cgsize_t _size;
    int16_t _spacing;
    cgrect_t _group_rect;
};

template<int BUTTON_COUNT>
class cgbutton_group_c : private cgbutton_group_base_c {
public:
    cgbutton_group_c(cgpoint_t origin, cgsize_t size, int16_t spacing) : 
        cgbutton_group_base_c(origin, size, spacing) {}
        
    void add_button(const char *title) {
        cgrect_t rect = next_button_rect(buttons.size() == 0);
        buttons.emplace_back(title, rect);
    }
    
    void add_button_pair(const char *left_title, const char *right_title, int16_t spacing = 8) {
        cgrect_t left_rect, right_rect;
        next_button_pair_rects(buttons.size() == 0, left_rect, right_rect, spacing);
        buttons.emplace_back(left_title, left_rect);
        buttons.emplace_back(right_title, right_rect);
    }
    
    void draw_all(cgimage_c &screen) {
        for (auto button = buttons.begin(); button != buttons.end(); button++) {
            button->draw_in(screen);
        }
    }
        
    int update_buttons(cgimage_c &screen, cgpoint_t pos, cgmouse_c::state_e state) {
        return update_button_range(buttons.begin(), buttons.end(), pos, screen, state);
    }

    cgvector_c<cgbutton_t, BUTTON_COUNT> buttons;
};


#endif /* button_hpp */
