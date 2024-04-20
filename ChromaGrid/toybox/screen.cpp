//
//  screen.cpp
//  toybox
//
//  Created by Fredrik on 2024-04-17.
//

#include "screen.hpp"
#include "system.hpp"

using namespace toybox;

screen_c::screen_c(size_s screen_size) :
    _image(screen_size, false, nullptr), _canvas(_image), _offset()
{
    assert(screen_size.width >= 320 && screen_size.height >= 200);
    _dirtymap = _canvas.create_dirtymap();
}

screen_c::~screen_c() {
    if (g_active_image == &_image) {
        g_active_image = nullptr;
    }
}

void screen_c::set_active() const {
    timer_c::with_paused_timers([this] {
        g_active_image = &_image;
    });
}
