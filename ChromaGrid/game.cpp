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
#include "blitter.hpp"

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
    cgfont_c font(font_image, (cgsize_t){8, 8}, 4, 2, 4);

    printf("load button.\n\r");
    cgimage_c button("BUTTON.IFF", true, 6);

    printf("load music.\n\r");
    cgmusic_c music("music.snd");
    
    printf("draw initial screen.\n\r");
    pLogical.draw_aligned(background, (cgpoint_t){0, 12});
    pLogical.draw_aligned(tiles, (cgpoint_t){0 + 16, 12 + 16});
    pLogical.draw(orbs, (cgrect_t){ {0, 0}, { 16, 10} }, (cgpoint_t){0 + 16 * 2, 12 + (16 * 3) + 3}, 6);
    pLogical.draw(orbs, (cgrect_t){ {16, 0}, { 16, 10} }, (cgpoint_t){0 + 16 * 4, 12 + (16 * 4) + 3}, 0);
    pLogical.draw(orbs, (cgrect_t){ {0, 0}, { 16, 10} }, (cgpoint_t){0 + 16 * 2, 12 + 100 });
    pLogical.draw(orbs, (cgrect_t){ {16, 0}, { 16, 10} }, (cgpoint_t){0 + 16 * 4, 12 + 100 });
    
#define BUTTON_SPACING 20
#define BUTTON_BOTTOM 188
    cgrect_t button_rect = (cgrect_t){{0,14},{32,14}};
    cgrect_t in_rect = (cgrect_t){{200, BUTTON_BOTTOM - 3}, {112, 14}};
    pLogical.draw_3_patch(button, button_rect, 7, in_rect); in_rect.origin.y -= BUTTON_SPACING;
    pLogical.draw(font, "Exit", (cgpoint_t){256, BUTTON_BOTTOM - BUTTON_SPACING * 0}, cgimage_c::align_center, 7);
    button_rect.origin.y -= 14;
    pLogical.draw_3_patch(button, button_rect, 7, in_rect); in_rect.origin.y -= BUTTON_SPACING;
    pLogical.draw(font, "Credits", (cgpoint_t){256, BUTTON_BOTTOM - BUTTON_SPACING * 1}, cgimage_c::align_center);
    pLogical.draw_3_patch(button, button_rect, 7, in_rect); in_rect.origin.y -= BUTTON_SPACING;
    pLogical.draw(font, "Help", (cgpoint_t){256, BUTTON_BOTTOM - BUTTON_SPACING * 2}, cgimage_c::align_center);
    pLogical.draw_3_patch(button, button_rect, 7, in_rect); in_rect.origin.y -= BUTTON_SPACING;
    pLogical.draw(font, "Editor", (cgpoint_t){256, BUTTON_BOTTOM - BUTTON_SPACING * 3}, cgimage_c::align_center);
    pLogical.draw_3_patch(button, button_rect, 7, in_rect); in_rect.origin.y -= BUTTON_SPACING;
    pLogical.draw(font, "Hi-Scores", (cgpoint_t){256, BUTTON_BOTTOM - BUTTON_SPACING * 4}, cgimage_c::align_center);
    pLogical.draw_3_patch(button, button_rect, 7, in_rect); in_rect.origin.y -= BUTTON_SPACING;
    pLogical.draw(font, "PLAY", (cgpoint_t){256, BUTTON_BOTTOM - BUTTON_SPACING * 5}, cgimage_c::align_center);

    /*
    for (int i = 0; i < (4 * 2 * 4); i++) {
        int16_t dstX = - 4 + (i % 4) * 8;
        int16_t dstY = 16 + i * 6;
        int16_t srcX = ((i / 4) % 2) * 8;
        int16_t srcWidth = 12 + (i / 8) % 4 * 12;
        cgpoint_t at = (cgpoint_t){dstX, dstY};
        cgrect_t rect = (cgrect_t){ {srcX, 8}, {srcWidth, 6}};
        printf("  Draw: {{%d,%d}{%d,%d}} at {%d,%d}\n\r", rect.origin.x, rect.origin.y, rect.size.width, rect.size.height, at.x, at.y);
        if (srcWidth == 12 && srcX == 8) {
            (void)0;
        }
        pLogical.draw(font.get_image(), rect, at);
    }
    pLogical.draw(font.get_image(), (cgrect_t){{0,0}, {32,16}}, (cgpoint_t){-12, 160});
     */
    pLogical.draw(font, "Welcome to Chroma Grid.", (cgpoint_t){96, 140 + 12 * 0}, cgimage_c::align_center);
    pLogical.draw(font, "\x7f 2024 T.O.Y.S.", (cgpoint_t){96, 140 + 12 * 1}, cgimage_c::align_center);
    pLogical.draw(font, "Released at Sommarhack.", (cgpoint_t){96, 140 + 12 * 2 + 6}, cgimage_c::align_center);

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
#ifdef __M68000__
            pPhysical.draw(cursor, mouse.get_postion());
#else
            pBlitter->debug = true;
            pPhysical.draw(cursor, mouse.get_postion());
            pBlitter->debug = false;
#endif
        });
        
        blue.set_at(0);
        vbl.wait();
        background.get_palette()->colors[0].set_at(0);
    }
    
    return 0;
}
