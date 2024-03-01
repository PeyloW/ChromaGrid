//
//  game_intro.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-03-01.
//

#include "game.hpp"

void cgintro_scene_c::will_appear(cgimage_c &screen, bool obsured) {
    printf("draw initial screen.\n\r");
    screen.draw_aligned(rsc.background, (cgpoint_t){0, 0});
    screen.draw_aligned(rsc.tiles, (cgpoint_t){0 + 16, 16});
    screen.draw(rsc.orbs, (cgrect_t){ {0, 0}, { 16, 10} }, (cgpoint_t){0 + 16 * 2, (16 * 3) + 3}, 6);
    screen.draw(rsc.orbs, (cgrect_t){ {16, 0}, { 16, 10} }, (cgpoint_t){0 + 16 * 4, (16 * 4) + 3}, 0);
    screen.draw(rsc.orbs, (cgrect_t){ {0, 0}, { 16, 10} }, (cgpoint_t){0 + 16 * 2, 100 });
    screen.draw(rsc.orbs, (cgrect_t){ {16, 0}, { 16, 10} }, (cgpoint_t){0 + 16 * 4, 100 });
    
#define BUTTON_SPACING 20
#define BUTTON_BOTTOM 176
    cgrect_t button_rect = (cgrect_t){{8,14},{32,14}};
    cgrect_t in_rect = (cgrect_t){{200, BUTTON_BOTTOM - 3}, {112, 14}};
    screen.draw_3_patch(rsc.button, button_rect, 8, in_rect); in_rect.origin.y -= BUTTON_SPACING;
    screen.draw(rsc.font, "Exit", (cgpoint_t){256, BUTTON_BOTTOM - BUTTON_SPACING * 0}, cgimage_c::align_center, 7);
    button_rect.origin.y -= 14;
    screen.draw_3_patch(rsc.button, button_rect, 8, in_rect); in_rect.origin.y -= BUTTON_SPACING;
    screen.draw(rsc.font, "Credits", (cgpoint_t){256, BUTTON_BOTTOM - BUTTON_SPACING * 1}, cgimage_c::align_center);
    screen.draw_3_patch(rsc.button, button_rect, 8, in_rect); in_rect.origin.y -= BUTTON_SPACING;
    screen.draw(rsc.font, "Help", (cgpoint_t){256, BUTTON_BOTTOM - BUTTON_SPACING * 2}, cgimage_c::align_center);
    screen.draw_3_patch(rsc.button, button_rect, 8, in_rect); in_rect.origin.y -= BUTTON_SPACING;
    screen.draw(rsc.font, "Editor", (cgpoint_t){256, BUTTON_BOTTOM - BUTTON_SPACING * 3}, cgimage_c::align_center);
    screen.draw_3_patch(rsc.button, button_rect, 8, in_rect); in_rect.origin.y -= BUTTON_SPACING;
    screen.draw(rsc.font, "Hi-Scores", (cgpoint_t){256, BUTTON_BOTTOM - BUTTON_SPACING * 4}, cgimage_c::align_center);
    screen.draw_3_patch(rsc.button, button_rect, 8, in_rect); in_rect.origin.y -= BUTTON_SPACING;
    screen.draw(rsc.font, "PLAY", (cgpoint_t){256, BUTTON_BOTTOM - BUTTON_SPACING * 5}, cgimage_c::align_center);

    screen.draw(rsc.font, "Welcome to Chroma Grid.", (cgpoint_t){96, 128 + 12 * 0}, cgimage_c::align_center);
    screen.draw(rsc.font, "\x7f 2024 T.O.Y.S.", (cgpoint_t){96, 128 + 12 * 1}, cgimage_c::align_center);
    screen.draw(rsc.small_font, "Released at Sommarhack.", (cgpoint_t){96, 128 + 12 * 2 + 6}, cgimage_c::align_center);
}

void cgintro_scene_c::tick(cgimage_c &screen) {
    
}
