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

int32_t cggame_main(void) {
    auto &rsc = cgresources_c::shared();
    
    printf("create phys.\n\r");
    cgimage_c pPhysical((cgsize_t){ 320, 224 }, false, nullptr);
    printf("create log.\n\r");
    cgimage_c pLogical((cgsize_t){ 320, 224 }, false, nullptr);
    bool *dirtymap = (bool *)calloc(pPhysical.dirtymap_size(), 1);
    
    printf("draw initial screen.\n\r");
    pLogical.draw_aligned(rsc.background, (cgpoint_t){0, 12});
    pLogical.draw_aligned(rsc.tiles, (cgpoint_t){0 + 16, 12 + 16});
    pLogical.draw(rsc.orbs, (cgrect_t){ {0, 0}, { 16, 10} }, (cgpoint_t){0 + 16 * 2, 12 + (16 * 3) + 3}, 6);
    pLogical.draw(rsc.orbs, (cgrect_t){ {16, 0}, { 16, 10} }, (cgpoint_t){0 + 16 * 4, 12 + (16 * 4) + 3}, 0);
    pLogical.draw(rsc.orbs, (cgrect_t){ {0, 0}, { 16, 10} }, (cgpoint_t){0 + 16 * 2, 12 + 100 });
    pLogical.draw(rsc.orbs, (cgrect_t){ {16, 0}, { 16, 10} }, (cgpoint_t){0 + 16 * 4, 12 + 100 });
    
#define BUTTON_SPACING 20
#define BUTTON_BOTTOM 188
    cgrect_t button_rect = (cgrect_t){{8,14},{32,14}};
    cgrect_t in_rect = (cgrect_t){{200, BUTTON_BOTTOM - 3}, {112, 14}};
    pLogical.draw_3_patch(rsc.button, button_rect, 8, in_rect); in_rect.origin.y -= BUTTON_SPACING;
    pLogical.draw(rsc.font, "Exit", (cgpoint_t){256, BUTTON_BOTTOM - BUTTON_SPACING * 0}, cgimage_c::align_center, 7);
    button_rect.origin.y -= 14;
    pLogical.draw_3_patch(rsc.button, button_rect, 8, in_rect); in_rect.origin.y -= BUTTON_SPACING;
    pLogical.draw(rsc.font, "Credits", (cgpoint_t){256, BUTTON_BOTTOM - BUTTON_SPACING * 1}, cgimage_c::align_center);
    pLogical.draw_3_patch(rsc.button, button_rect, 8, in_rect); in_rect.origin.y -= BUTTON_SPACING;
    pLogical.draw(rsc.font, "Help", (cgpoint_t){256, BUTTON_BOTTOM - BUTTON_SPACING * 2}, cgimage_c::align_center);
    pLogical.draw_3_patch(rsc.button, button_rect, 8, in_rect); in_rect.origin.y -= BUTTON_SPACING;
    pLogical.draw(rsc.font, "Editor", (cgpoint_t){256, BUTTON_BOTTOM - BUTTON_SPACING * 3}, cgimage_c::align_center);
    pLogical.draw_3_patch(rsc.button, button_rect, 8, in_rect); in_rect.origin.y -= BUTTON_SPACING;
    pLogical.draw(rsc.font, "Hi-Scores", (cgpoint_t){256, BUTTON_BOTTOM - BUTTON_SPACING * 4}, cgimage_c::align_center);
    pLogical.draw_3_patch(rsc.button, button_rect, 8, in_rect); in_rect.origin.y -= BUTTON_SPACING;
    pLogical.draw(rsc.font, "PLAY", (cgpoint_t){256, BUTTON_BOTTOM - BUTTON_SPACING * 5}, cgimage_c::align_center);

    pLogical.draw(rsc.font, "Welcome to Chroma Grid.", (cgpoint_t){96, 140 + 12 * 0}, cgimage_c::align_center);
    pLogical.draw(rsc.font, "\x7f 2024 T.O.Y.S.", (cgpoint_t){96, 140 + 12 * 1}, cgimage_c::align_center);
    pLogical.draw(rsc.small_font, "Released at Sommarhack.", (cgpoint_t){96, 140 + 12 * 2 + 6}, cgimage_c::align_center);

    /*
    printf("draw initial screen.\n\r");
    pPhysical.draw_aligned(pLogical, (cgpoint_t){0, 0});
    */
     
    printf("setup vbl.\n\r");
    cgtimer_c vbl(cgtimer_c::vbl);
    printf("start music.\n\r");
    rsc.music.set_active(1);
    printf("setup mouse.\n\r");
    cgmouse_c mouse((cgrect_t){ 0, 12, 320, 200 });
    
    printf("set low rez.\n\r");
    cgset_screen(nullptr, nullptr, 0);
    printf("set palette.\n\r");
    rsc.background.get_palette()->set_active();
    printf("Activate phys.\n\r");
    pPhysical.set_offset((cgpoint_t){ 0, 12 });
    pPhysical.set_active();

    cgcolor_c blue(0, 0, 95);
    vbl.reset_tick();
    while (true) {
        auto tick = vbl.tick();
        if (tick <= cgimage_c::STENCIL_FULLY_OPAQUE) {
            pPhysical.with_stencil(&rsc.stencils[1][tick], [&pPhysical, &pLogical] {
                pPhysical.draw_aligned(pLogical, (cgpoint_t){0, 0});
            });
        } else {
            if (mouse.was_clicked(cgmouse_c::left)) {
                pLogical.put_pixel(9, mouse.get_postion());
            }
            if (mouse.was_clicked(cgmouse_c::right)) {
                pLogical.put_pixel(10, mouse.get_postion());
            }
            pPhysical.restore(pLogical, dirtymap);
            pPhysical.with_dirtymap(dirtymap, [&pPhysical, &rsc, &mouse] {
#ifdef __M68000__
                pPhysical.draw(rsc.cursor, mouse.get_postion());
#else
                pBlitter->debug = true;
                pPhysical.draw(rsc.cursor, mouse.get_postion());
                pBlitter->debug = false;
#endif
            });
        }

        rsc.background.get_palette()->colors[0].set_at(0);
        vbl.wait();
        blue.set_at(0);
    }
    
    return 0;
}
