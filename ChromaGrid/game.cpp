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

void cgroot_scene_c::will_appear(cgimage_c &screen, bool obsured) {
    rsc.music.set_active(1);
    printf("set palette.\n\r");
    rsc.background.get_palette()->set_active();

    manager.push(new cgintro_scene_c(manager));
}

void cgroot_scene_c::will_disappear(bool obscured) {
    if (obscured == false) {
        rsc.music.set_active(0);
    }
}

void cgroot_scene_c::tick(cgimage_c &screen) {
    screen.with_clipping(true, [this, &screen] {
        if (manager.mouse.was_clicked(cgmouse_c::left)) {
            auto &logical = manager.get_logical_screen();
            logical.put_pixel(9, manager.mouse.get_postion());
        }
        if (manager.mouse.was_clicked(cgmouse_c::right)) {
            auto &logical = manager.get_logical_screen();
            logical.put_pixel(10, manager.mouse.get_postion());
        }
        screen.draw(rsc.cursor, manager.mouse.get_postion());
    });
}
