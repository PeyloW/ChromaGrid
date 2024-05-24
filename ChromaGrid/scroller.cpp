//
//  scroller.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-05-23.
//

#include "scroller.hpp"


scroller_c::scroller_c() :
    _font(cgasset_manager::shared().font(SMALL_FONT)),
    _text()
{
    reset(cgasset_manager::shared().menu_scroll().text(), 160);
}

void scroller_c::reset(const char *text, int space_count) {
    _text = text;
    _paragraph_pos = 0;
    restore();
    _space_count = space_count;
}


void scroller_c::store() {
}

void scroller_c::restore() {
    _next_pos = _paragraph_pos;
    _space_count = 0;
    _column = 0;
    _char_rect = rect_s();
}

void scroller_c::update(screen_c &screen) {
    auto &canvas = screen.canvas();
    canvas.draw(canvas.image(), rect_s(0, 192, 320, 8), point_s(-1, 200));
    canvas.draw_aligned(canvas.image(), rect_s(0, 200, 320, 8), point_s(0, 192));
    
    if (_space_count > 0) {
        _space_count--;
    } else {
        if (_column == _char_rect.size.width) {
            _column = 0;
            char c = _text[_next_pos++];
            if (c == 0) {
                _next_pos = _paragraph_pos = 0;
                _space_count = 160;
            } else if (c < 32) {
                _paragraph_pos = _next_pos;
                _space_count = 4 * 16;
            } else if (c == 32) {
                _space_count = 3;
            }
            if (_space_count > 0) {
                _char_rect = rect_s();
                return;
            }
            _char_rect = _font.char_rect(c);
        }
        const rect_s col_rect = rect_s(_char_rect.origin.x + _column, _char_rect.origin.y, 1, _char_rect.size.height);
        _column++;
        canvas.draw(*_font.image(), col_rect, point_s(318, 194));
    }
    
}
