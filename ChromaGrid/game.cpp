//
//  game.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-03.
//

#include "game.hpp"
#include "system.hpp"
#include "graphics.hpp"

static cgimage_t pPhysical((cgsize_t){ 320, 208 }, cgimage_t::mask_mode_none, nullptr);
static cgimage_t pLogical((cgsize_t){ 320, 208 }, cgimage_t::mask_mode_none, nullptr);

int32_t game_main(void) {
    
    cgtimer_t vbl(cgtimer_t::vbl, nullptr);
    cgmouse_t mouse((cgrect_t){ 0, 4, 320, 200 });
    
    pPhysical.set_offset((cgpoint_t){ 0, 4 });
    pPhysical.set_active();
    cgimage_t background("backgrnd.iff", cgimage_t::mask_mode_none);
    background.set_offset((cgpoint_t){0, 0});
    cgimage_t cursor("cursor.iff");
    cursor.set_offset((cgpoint_t){1, 2});
    
    background.get_palette()->set_active();
    pLogical.draw_aligned(&background, (cgpoint_t){0, 4});
    
    while (true) {
        if (mouse.was_clicked(cgmouse_t::left)) {
            pLogical.put_pixel(9, mouse.get_postion());
        }
        if (mouse.was_clicked(cgmouse_t::right)) {
            pLogical.put_pixel(10, mouse.get_postion());
        }
        pPhysical.draw_aligned(&pLogical, (cgpoint_t){ 0, 0 } );
        pPhysical.draw(&cursor, mouse.get_postion());
        vbl.wait();
    }
    
    return 0;
}
