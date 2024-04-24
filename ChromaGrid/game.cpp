//
//  game.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-03.
//

#include "game.hpp"
#include "machine.hpp"
#include "resources.hpp"
#include "blitter.hpp"
#include "audio_mixer.hpp"

#ifdef __M68000__
extern "C" {
#include <ext.h>
}
#endif

void cgoverlay_scene_c::will_appear(screen_c &screen, bool obsured) {
    audio_mixer_c::shared().play(rsc.music);
    machine_c::shared().set_active_palette(rsc.background.palette().get());
}

void cgoverlay_scene_c::will_disappear(bool obscured) {
    if (obscured == false) {
        audio_mixer_c::shared().stop(rsc.music);
    }
}

void cgoverlay_scene_c::update_foreground(screen_c &screen, int ticks) {
    auto &canvas = screen.canvas();
    canvas.with_clipping(true, [this, &canvas] {
        /*
        if (manager.mouse.is_pressed(cgmouse_c::left)) {
            auto &logical = manager.logical_screen();
            logical.put_pixel(9, manager.mouse.postion());
        }
        if (manager.mouse.is_pressed(cgmouse_c::right)) {
            auto &logical = manager.logical_screen();
            logical.put_pixel(10, manager.mouse.postion());
        }
         */
        point_s at = manager.mouse.postion();
        at.x -= 2;
        at.y -= 2;
        canvas.draw(rsc.cursor, at);
    });
}
