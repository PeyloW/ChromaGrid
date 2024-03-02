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
    for (int y = 0; y < 12; y++) {
        for (int x = 0; x < 12; x++) {
            int dx = ABS(x * 2 - 11);
            int dy = ABS(y * 2 - 11);
            int dist = sqrt((dx * dx + dy * dy) * 8);
            int32_t r = cgrand();
            if ((r % 32) > dist) {
                int shade = 44 - dist;
                int row;
                switch ((r >> 8) & 0xf) {
                    case 0 ... 3: row = 0; break;
                    case 4 ... 5: row = 1; break;
                    case 6 ... 7: row = 2; break;
                    case 8 ... 9: row = 4; break;
                    default: row = 3; break;
                }
                int col = 0;
                if (row > 0) {
                    switch ((r >> 12) &03) {
                        case 0: col = 1; break;
                        case 1: col = 2; break;
                    }
                }
                cgrect_t rect = (cgrect_t){{(int16_t)(col * 16), (int16_t)(row * 16)}, {16, 16}};
                cgpoint_t at = (cgpoint_t){(int16_t)(x * 16), (int16_t)(y * 16)};
                screen.with_stencil(cgimage_c::get_stencil(cgimage_c::orderred, shade), [&] {
                    screen.draw_aligned(rsc.tiles, rect, at);
                });
            }
        }
    }
    
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

    screen.draw(rsc.font, "Welcome to Chroma Grid.", (cgpoint_t){96, 64 + 12 * 0}, cgimage_c::align_center);
    screen.draw(rsc.font, "\x7f 2024 T.O.Y.S.", (cgpoint_t){96, 64 + 20 * 1}, cgimage_c::align_center);
    screen.draw(rsc.small_font, "Released at Sommarhack.", (cgpoint_t){96, 64 + 20 * 3}, cgimage_c::align_center);
}

void cgintro_scene_c::tick(cgimage_c &screen) {
    
}
