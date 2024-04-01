//
//  game.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-03.
//

#include "game.hpp"
#include "system.hpp"
#include "resources.hpp"
#include "blitter.hpp"

#ifdef __M68000__
extern "C" {
#include <ext.h>
}
#endif

void cgoverlay_scene_c::will_appear(image_c &screen, bool obsured) {
    rsc.music.set_active(1);
    rsc.background.get_palette()->set_active();
}

void cgoverlay_scene_c::will_disappear(bool obscured) {
    if (obscured == false) {
        rsc.music.set_active(0);
    }
}

void cgoverlay_scene_c::update_foreground(image_c &screen, int ticks) {
    screen.with_clipping(true, [this, &screen] {
        /*
        if (manager.mouse.is_pressed(cgmouse_c::left)) {
            auto &logical = manager.get_logical_screen();
            logical.put_pixel(9, manager.mouse.get_postion());
        }
        if (manager.mouse.is_pressed(cgmouse_c::right)) {
            auto &logical = manager.get_logical_screen();
            logical.put_pixel(10, manager.mouse.get_postion());
        }
         */
        screen.draw(rsc.cursor, manager.mouse.get_postion());
    });
}
