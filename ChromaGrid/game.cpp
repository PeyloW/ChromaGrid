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

static void remap_to(color_t col, cgcolor_remap_table_t table) {
    switch (col) {
        case color_t::gold:
            table[1] = 1;
            table[3] = 1;
            table[8] = 9;
            table[11] = 14;
            break;
        case color_t::silver:
            table[1] = 2;
            table[3] = 2;
            table[8] = 8;
            table[11] = 13;
            break;
        default:
            break;
    }
}

int32_t cggame_main(void) {
    printf("create phys.\n\r");
    cgimage_c pPhysical((cgsize_t){ 320, 208 }, false, nullptr);
    printf("create log.\n\r");
    cgimage_c pLogical((cgsize_t){ 320, 208 }, false, nullptr);

    printf("load background.\n\r");
    cgimage_c background("BACKGRND.IFF", false);
    background.set_offset((cgpoint_t){0, 0});

    printf("load tiles.\n\r");
    cgimage_c tiles("TILES.IFF", false);
    tiles.set_offset((cgpoint_t){0, 0});
    for (int x = 1; x < 3; x++) {
        cgcolor_remap_table_t table;
        cgimage_c::make_noremap_table(table);
        cgrect_t rect = {{static_cast<int16_t>(x * 48), 0}, {48, 80}};
        tiles.draw(&tiles, (cgrect_t){{0, 0}, {48, 80}}, rect.origin);
        if (x == 1) {
            remap_to(color_t::gold, table);
        } else {
            remap_to(color_t::silver, table);
        }
        tiles.remap_colors(table, rect);
    }

    printf("load orbs.\n\r");
    cgimage_c orbs("ORBS.IFF", true, 6);
    orbs.set_offset((cgpoint_t){0, 0});

    printf("load cursor.\n\r");
    cgimage_c cursor("CURSOR.IFF", true, 0);
    cursor.set_offset((cgpoint_t){1, 2});

    printf("load music.\n\r");
    cgmusic_c music("music.snd");
    
    printf("draw initial screen.\n\r");
    pLogical.draw_aligned(&background, (cgpoint_t){0, 4});
    pLogical.draw_aligned(&tiles, (cgpoint_t){0 + 16, 4 + 16});
    pLogical.draw(&orbs, (cgrect_t){ {0, 0}, { 16, 10} }, (cgpoint_t){0 + 16 * 2, 4 + (16 * 3) + 3});
    pLogical.draw(&orbs, (cgrect_t){ {16, 0}, { 16, 10} }, (cgpoint_t){0 + 16 * 4, 4 + (16 * 4) + 3});

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
