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
    auto &m = machine_c::shared();
    if (m.active_image() == &_image) {
        m.set_active_image(nullptr);
    }
}
