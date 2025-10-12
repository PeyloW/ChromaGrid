//
//  scroller.hpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-05-23.
//

#pragma once

#include "types.hpp"
#include "resources.hpp"
#include "screen.hpp"

class scroller_c : color_c {
public:
    scroller_c();
    
    void reset(const char *text, int space_count = 0);
    
    void store();
    void restore();
    
    void update(screen_c &screen);
    
private:
    const font_c &_font;
    const char *_text;
    int _next_pos;
    int _space_count;
    int _column;
    rect_s _char_rect;
    int _paragraph_pos;
};
