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

static void remap_to(color_t col, cgimage_c::remap_table_t table, uint8_t masked_idx = cgimage_c::MASKED_CIDX) {
    switch (col) {
        case color_t::gold:
            table[0] = 1;
            table[2] = 1;
            table[7] = 9;
            table[10] = 14;
            break;
        case color_t::silver:
            table[0] = 2;
            table[2] = 2;
            table[7] = 8;
            table[10] = 13;
            break;
        default:
            break;
    }
    table[masked_idx] = cgimage_c::MASKED_CIDX;
}

int32_t cggame_main(void) {
    printf("create phys.\n\r");
    cgimage_c pPhysical((cgsize_t){ 320, 224 }, false, nullptr);
    printf("create log.\n\r");
    cgimage_c pLogical((cgsize_t){ 320, 224 }, false, nullptr);
    bool *dirtymap = (bool *)calloc(pPhysical.dirtymap_size(), 1);

    printf("load background.\n\r");
    cgimage_c background("BACKGRND.IFF", false);
    background.set_offset((cgpoint_t){0, 0});

    printf("load tiles.\n\r");
    cgimage_c tiles("TILES.IFF", false);
    tiles.set_offset((cgpoint_t){0, 0});
    for (int x = 1; x < 3; x++) {
        printf("  copy tiles %d.\n\r", x);
        cgrect_t rect = {{static_cast<int16_t>(x * 48), 0}, {48, 80}};
        tiles.draw(tiles, (cgrect_t){{0, 0}, {48, 80}}, rect.origin);
        printf("  remap tiles %d.\n\r", x);
        cgimage_c::remap_table_t table;
        cgimage_c::make_noremap_table(table);
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

    printf("load font.\n\r");
    cgimage_c font_image("FONT.IFF", true, 0);
    cgfont_c font(font_image, (cgsize_t){8, 8});
    
    printf("load music.\n\r");
    cgmusic_c music("music.snd");
    
    printf("draw initial screen.\n\r");
    pLogical.draw_aligned(background, (cgpoint_t){0, 12});
    pLogical.draw_aligned(tiles, (cgpoint_t){0 + 16, 12 + 16});
    pLogical.draw(orbs, (cgrect_t){ {0, 0}, { 16, 10} }, (cgpoint_t){0 + 16 * 2, 12 + (16 * 3) + 3});
    pLogical.draw(orbs, (cgrect_t){ {16, 0}, { 16, 10} }, (cgpoint_t){0 + 16 * 4, 12 + (16 * 4) + 3});
    pLogical.draw(orbs, (cgrect_t){ {0, 0}, { 16, 10} }, (cgpoint_t){0 + 16 * 2, 12 + 100 });
    pLogical.draw(orbs, (cgrect_t){ {16, 0}, { 16, 10} }, (cgpoint_t){0 + 16 * 4, 12 + 100 });
    pLogical.draw(font, "Hello World!", (cgpoint_t){256, 192}, cgimage_c::align_center);
     
    for (int i = 0; i < 5; i++) {
        pLogical.draw(cursor, (cgpoint_t){ (int16_t)(i * 5 - 4), int16_t(50 + i * 16) });
    }
    
    printf("draw initial screen.\n\r");
    pPhysical.draw_aligned(pLogical, (cgpoint_t){0, 0});
    
    printf("setup vbl.\n\r");
    cgtimer_c vbl(cgtimer_c::vbl);
    printf("start music.\n\r");
    music.set_active(2);
    printf("setup mouse.\n\r");
    cgmouse_c mouse((cgrect_t){ 0, 12, 320, 200 });
    
    printf("set low rez.\n\r");
    cgset_screen(nullptr, nullptr, 0);
    printf("set palette.\n\r");
    background.get_palette()->set_active();
    printf("Activate phys.\n\r");
    pPhysical.set_offset((cgpoint_t){ 0, 12 });
    pPhysical.set_active();

    cgcolor_c blue(0, 0, 95);
    while (true) {
        if (mouse.was_clicked(cgmouse_c::left)) {
            pLogical.put_pixel(9, mouse.get_postion());
        }
        if (mouse.was_clicked(cgmouse_c::right)) {
            pLogical.put_pixel(10, mouse.get_postion());
        }
        pPhysical.restore(pLogical, dirtymap);
        pPhysical.with_dirtymap(dirtymap, [&pPhysical, &cursor, &mouse] {
            pPhysical.draw(cursor, mouse.get_postion());
        });
        
        blue.set_at(0);
        vbl.wait();
        background.get_palette()->colors[0].set_at(0);
    }
    
    return 0;
}
