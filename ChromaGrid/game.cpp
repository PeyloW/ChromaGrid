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

void cgbutton_t::draw_in(cgimage_c &image) const {
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

void cgoverlay_scene_c::will_appear(cgimage_c &screen, bool obsured) {
    rsc.music.set_active(1);
    rsc.background.get_palette()->set_active();
}

void cgoverlay_scene_c::will_disappear(bool obscured) {
    if (obscured == false) {
        rsc.music.set_active(0);
    }
}

void cgoverlay_scene_c::tick(cgimage_c &screen) {
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
