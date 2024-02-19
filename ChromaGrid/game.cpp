//
//  game.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-03.
//

#include "game.hpp"
#include "system.hpp"
#include "graphics.hpp"
#include "audio.hpp"

#ifdef __M68000__
extern "C" {
#include <ext.h>
}
#endif


int32_t cggame_main(void) {
    printf("create phys.\n\r");
    cgimage_c pPhysical((cgsize_t){ 320, 208 }, cgimage_c::mask_mode_none, nullptr);
    printf("create log.\n\r");
    cgimage_c pLogical((cgsize_t){ 320, 208 }, cgimage_c::mask_mode_none, nullptr);

    printf("load background.\n\r");
    cgimage_c background("BACKGRND.IFF", cgimage_c::mask_mode_none);
    background.set_offset((cgpoint_t){0, 0});

    printf("load cursor.\n\r");
    cgimage_c cursor("CURSOR.IFF");
    cursor.set_offset((cgpoint_t){1, 2});

    printf("load music.\n\r");
    cgmusic_c music("music.snd");
    
    printf("draw background.\n\r");
    pLogical.draw_aligned(&background, (cgpoint_t){0, 4});

    printf("setup vbl.\n\r");
    cgtimer_c vbl(cgtimer_c::vbl);

    
    printf("start music.\n\r");
    music.set_active(2);
    
    printf("setup mouse.\n\r");
    cgmouse_c mouse((cgrect_t){ 0, 4, 320, 200 });
    
    printf("set low rez.\n\r");
    cgset_screen(nullptr, nullptr, 0);

    printf("set palette.\n\r");
    background.get_palette()->set_active();

    printf("Activate phys.\n\r");
    pPhysical.set_offset((cgpoint_t){ 0, 4 });
    pPhysical.set_active();
        
        
    while (true) {
        if (mouse.was_clicked(cgmouse_c::left)) {
            pLogical.put_pixel(9, mouse.get_postion());
        }
        if (mouse.was_clicked(cgmouse_c::right)) {
            pLogical.put_pixel(10, mouse.get_postion());
        }
        pPhysical.draw_aligned(&pLogical, (cgpoint_t){ 0, 0 } );
        pPhysical.draw(&cursor, mouse.get_postion());
        vbl.wait();
    }
    
    return 0;
}
