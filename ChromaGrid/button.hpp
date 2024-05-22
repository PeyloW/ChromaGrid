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
#include "system_helpers.hpp"

struct cgbutton_t : public nocopy_c {
    typedef enum __packed {
        regular,
        destructive
    } style_e;
    typedef enum __packed {
        normal,
        pressed,
        disabled,
        hidden
    } state_t;
    inline cgbutton_t() : text("") {}
    inline cgbutton_t(const char *text, rect_s rect) : text(text), rect(rect), style(regular), state(normal) {}

    void draw_in(canvas_c &image) const;
    
    const char *text;
    rect_s rect;
    style_e style;
    state_t state;
};

class cgbutton_group_base_c : public nocopy_c {
    protected:
    cgbutton_group_base_c(point_s origin, size_s size, int16_t spacing) :
        _tracked_button(-1),
        _size(size),
        _spacing(spacing)
    {
        _group_rect = rect_s(origin, size_s());
    }

    rect_s next_button_rect(bool first, bool horizontal = false);
    void next_button_pair_rects(bool first, rect_s &left_rect, rect_s &right_rect, int16_t spacing);
    int update_button_range(cgbutton_t *begin, cgbutton_t *end, const point_s &pos, canvas_c &screen, mouse_c::state_e state);
    
    int _tracked_button;
    size_s _size;
    int16_t _spacing;
    rect_s _group_rect;
};

template<int BUTTON_COUNT>
class cgbutton_group_c : private cgbutton_group_base_c {
public:
    cgbutton_group_c(point_s origin, size_s size, int16_t spacing) : 
        cgbutton_group_base_c(origin, size, spacing) {}
        
    void add_button(const char *title, bool horizontal = false) {
        rect_s rect = next_button_rect(buttons.size() == 0, horizontal);
        buttons.emplace_back(title, rect);
    }
    
    void add_button_pair(const char *left_title, const char *right_title, int16_t spacing = 8) {
        rect_s left_rect, right_rect;
        next_button_pair_rects(buttons.size() == 0, left_rect, right_rect, spacing);
        buttons.emplace_back(left_title, left_rect);
        buttons.emplace_back(right_title, right_rect);
    }
    
    void draw_all(canvas_c &screen) const {
        for (const auto &button : buttons) {
            button.draw_in(screen);
        }
    }
        
    int update_buttons(canvas_c &screen, point_s pos, mouse_c::state_e state) {
        return update_button_range(buttons.begin(), buttons.end(), pos, screen, state);
    }

    vector_c<cgbutton_t, BUTTON_COUNT> buttons;
};


#endif /* button_hpp */
