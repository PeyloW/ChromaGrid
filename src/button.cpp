//
//  button.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-03-14.
//

#include "button.hpp"

void cgbutton_t::draw_in(canvas_c &image) const {
    if (state == hidden) {
        return;
    }
    int row = state == disabled ? 2 : style == destructive ? 1 : 0;
    const rect_s button_rect(8, row * 14, 32, 14);

    const auto &assets = cgasset_manager::shared();
    image.draw_3_patch(assets.image(BUTTON), button_rect, 8, rect);
    point_s at(
        rect.origin.x + rect.size.width / 2,
        rect.origin.y + (state != pressed ? 3 : 4)
    );
    image.with_dirtymap(nullptr, [&] {
        image.draw(assets.font(FONT), text, at, canvas_c::align_center, state != disabled ? image_c::MASKED_CIDX : 2);
    });
}

rect_s cgbutton_group_base_c::next_button_rect(bool first, bool horizontal) {
    rect_s rect(_group_rect.origin, _size);
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

void cgbutton_group_base_c::next_button_pair_rects(bool first, rect_s &left_rect, rect_s &right_rect, int16_t spacing) {
    left_rect = rect_s(point_s(_group_rect.origin.x, 0), _size);
    int16_t extra = !first ? ABS(_spacing) : 0;
    int16_t step = _size.height + ABS(_spacing);
    if (_spacing < 0) {
        left_rect.origin.y = _group_rect.origin.y - (_size.height + extra);
        _group_rect.origin.y -= (_size.height + extra);
    } else {
        left_rect.origin.y = _group_rect.origin.y + _group_rect.size.height + extra;
    }
    _group_rect.size.width = _size.width;
    _group_rect.size.height += _size.height + extra;
    left_rect.size.width = left_rect.size.width / 2 - (spacing / 2);
    
    right_rect = left_rect;
    right_rect.origin.x += left_rect.size.width + spacing;
}

int cgbutton_group_base_c::update_button_range(cgbutton_t *begin, cgbutton_t *end, canvas_c &screen, mouse_c &mouse) {
    const auto pos = mouse.postion();
    const auto state = MAX(mouse.state(mouse_c::left), mouse.state(mouse_c::right));
    if (_tracked_button >= 0) {
        auto &button = begin[_tracked_button];
        //assert(button.state == cgbutton_t::pressed);
        if (state == mouse_c::released || !button.rect.contains(pos)) {
            button.state = cgbutton_t::normal;
            button.draw_in(screen);
            _tracked_button = -1;
        }
    }
    if (_group_rect.contains(pos)) {
        if (state != mouse_c::released) {
            int idx = 0;
            for (cgbutton_t *button = begin; button != end; button++, idx++) {
                if (button->state >= cgbutton_t::disabled) {
                    continue;
                }
                if (button->rect.contains(pos)) {
                    if (state == mouse_c::pressed) {
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
