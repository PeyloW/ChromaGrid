//
//  game_intro.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-03-01.
//

#include "game.hpp"

cgintro_scene_c::cgintro_scene_c(cgmanager_c &manager) : 
    cggame_scene_c(manager),
    _menu_buttons((cgpoint_t){200, 184}, (cgsize_t){112, 14}, -6)
{
    const char *button_titles[6] = { "Exit", "Credits", "Help", "Editor", "Hi-Scores", "PLAY" };
    for (int i = 0; i < 6; i++) {
        _menu_buttons.add_button(button_titles[i]);
    }
    _menu_buttons.buttons[0].state = cgbutton_t::disabled;
}

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
    
    _menu_buttons.draw_all(screen);
    
    screen.draw(rsc.font, "Welcome to Chroma Grid.", (cgpoint_t){96, 64 + 12 * 0});
    screen.draw(rsc.font, "\x7f 2024 T.O.Y.S.", (cgpoint_t){96, 64 + 20 * 1});
//    screen.draw(rsc.small_font, "Released at Sommarhack.", (cgpoint_t){96, 64 + 20 * 3});

    screen.draw(rsc.small_font, "Released at Sommarhack.\nA game concep by Peter 'Eagle' Nyman of Friendchip, realized 30 years later.", (cgrect_t){{16, 64 + 20 * 3}, {160, 7 * 4}}, 3);
}

void cgintro_scene_c::tick(cgimage_c &screen) {
    int button = _menu_buttons.update_buttons(screen, manager.mouse.get_postion(), manager.mouse.get_state(cgmouse_c::left));
    switch (button) {
        case 0:
            manager.pop();
            break;
        case 1:
            // Show credits
            break;
        case 2:
            // Show help
            break;
        case 3:
            // Start editor
            break;
        case 4:
            // Show Hi-Scores
            break;
        case 5:
            manager.pop();
            break;
        default:
            break;
    }
}
