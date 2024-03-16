//
//  button.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-03-14.
//

#include "button.hpp"

void cgbutton_t::draw_in(cgimage_c &image) const {
    if (state == hidden) {
        return;
    }
    static const cgrect_t button_rect_normal = (cgrect_t){{8,0},{32,14}};
    static const cgrect_t button_rect_disabled = (cgrect_t){{8,14},{32,14}};

    const auto &rsc = cgresources_c::shared();
    image.draw_3_patch(rsc.button, state != disabled ? button_rect_normal : button_rect_disabled, 8, rect);
    cgpoint_t at = (cgpoint_t){
        (int16_t)(rect.origin.x + rect.size.width / 2),
        (int16_t)(rect.origin.y + (state != pressed ? 3 : 4))
    };
    image.with_dirtymap(nullptr, [&] {
        image.draw(rsc.font, text, at, cgimage_c::align_center, state != disabled ? cgimage_c::MASKED_CIDX : 7);
    });
}

cgrect_t cgbutton_group_base_c::next_button_rect(bool first, bool horizontal) {
    cgrect_t rect = (cgrect_t){_group_rect.origin, _size};
    if (horizontal) {
        int16_t expand = _size.width + (!first ? ABS(_spacing) : 0);
        if (_spacing < 0) {
            rect.origin.x = _group_rect.origin.x - expand;
            _group_rect.origin.x -= expand;
        } else {
            rect.origin.x = _group_rect.origin.x + _group_rect.size.width + (expand - _size.width);
        }
        _group_rect.size.width += expand;
        _group_rect.size.height = _size.height;
    } else {
        int16_t expand = _size.height + (!first ? ABS(_spacing) : 0);
        if (_spacing < 0) {
            rect.origin.y = _group_rect.origin.y - expand;
            _group_rect.origin.y -= expand;
        } else {
            rect.origin.y = _group_rect.origin.y + _group_rect.size.height + (expand - _size.height);
        }
        _group_rect.size.width = _size.width;
        _group_rect.size.height += expand;
    }
    return rect;
}

void cgbutton_group_base_c::next_button_pair_rects(bool first, cgrect_t &left_rect, cgrect_t &right_rect, int16_t spacing) {
    left_rect = (cgrect_t){{_group_rect.origin.x, 0}, _size};
    int16_t extra = !first ? ABS(_spacing) : 0;
    int16_t step = _size.height + ABS(_spacing);
    if (_spacing < 0) {
        left_rect.origin.y = _group_rect.origin.y - (_size.height + extra);
        _group_rect.origin.y -= (_size.height + extra);
    } else {
        left_rect.origin.y = _group_rect.origin.y + _group_rect.size.height + extra;
    }
    _group_rect.size.height += _size.height + extra;
    left_rect.size.width = left_rect.size.width / 2 - (spacing / 2);
    
    right_rect = left_rect;
    right_rect.origin.x += left_rect.size.width + spacing;
}

int cgbutton_group_base_c::update_button_range(cgbutton_t *begin, cgbutton_t *end, const cgpoint_t &pos, cgimage_c &screen, cgmouse_c::state_e state) {
    if (_tracked_button >= 0) {
        auto &button = begin[_tracked_button];
        //assert(button.state == cgbutton_t::pressed);
        if (state == cgmouse_c::released || !button.rect.contains(pos)) {
            button.state = cgbutton_t::normal;
            button.draw_in(screen);
            _tracked_button = -1;
        }
    }
    if (_group_rect.contains(pos)) {
        if (state != cgmouse_c::released) {
            int idx = 0;
            for (cgbutton_t *button = begin; button != end; button++, idx++) {
                if (button->state >= cgbutton_t::disabled) {
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
