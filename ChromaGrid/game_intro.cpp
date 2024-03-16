//
//  game_intro.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-03-01.
//

#include "game.hpp"

cgintro_scene_c::cgintro_scene_c(cgmanager_c &manager) : 
    cggame_scene_c(manager),
    _menu_buttons(MAIN_MENU_BUTTONS_ORIGIN, MAIN_MENU_BUTTONS_SIZE, MAIN_MENU_BUTTONS_SPACING)
{
    const char *button_titles[] = { "Exit", "Credits", "Help", "Editor", "Hi-Scores", "PLAY", nullptr };
    for (auto title = &button_titles[0]; *title; title++) {
        _menu_buttons.add_button(*title);
    }
    _menu_buttons.buttons[0].state = cgbutton_t::disabled;
    _menu_buttons.buttons[2].state = cgbutton_t::disabled;
}

void cgintro_scene_c::will_appear(cgimage_c &screen, bool obsured) {
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
    
    screen.draw(rsc.font, "Welcome to Chroma Grid.", (cgpoint_t){96, 150 });
    screen.draw(rsc.font, "\x7f 2024 T.O.Y.S.", (cgpoint_t){96, 170});
}

void cgintro_scene_c::tick(cgimage_c &screen, int ticks) {
    int button = update_button_group(screen, _menu_buttons);
    switch (button) {
        case 0:
            manager.pop();
            break;
        case 1:
            manager.push(new cgcredits_scene_c(manager));
            break;
        case 2:
            // Show help
            break;
        case 3:
            manager.push(new cglevel_edit_scene_c(manager, nullptr));
            break;
        case 4:
            manager.push(new cgscores_scene_c(manager));
            break;
        case 5:
            if (rsc.level_results.front().score == 0) {
                manager.push(new cglevel_scene_c(manager, 0));
            } else {
                manager.push(new cglevel_select_scene_c(manager));
            }
            break;
        default:
            break;
    }
}

cglevel_select_scene_c::cglevel_select_scene_c(cgmanager_c &manager) :
    cggame_scene_c(manager),
    _menu_buttons(MAIN_MENU_BUTTONS_ORIGIN, MAIN_MENU_BUTTONS_SIZE, MAIN_MENU_BUTTONS_SPACING)
{
    _menu_buttons.add_button("Back");
    
    static char title_buf[3 * 45];
    int index = 0;
    cgpoint_t origin = (cgpoint_t){16, 40};
    const cgsize_t size = (cgsize_t){26, 14};
    bool disable = false;
    for (auto result = rsc.level_results.begin(); result != rsc.level_results.end(); result++) {
        int col = index % 5;
        if (col == 0) {
            _select_button_groups.emplace_back(origin, size, 8);
            origin.y += 14 + 2;
        }
        auto &button_group = _select_button_groups.back();
        auto title = title_buf + index * 3;
        sprintf(title, "%d", index + 1);
        button_group.add_button(title, true);

        button_group.buttons.back().state = disable ? cgbutton_t::disabled : cgbutton_t::normal;
        if (result->score == 0) {
            disable = true;
        }
        
        index++;
    }
}

void cglevel_select_scene_c::will_appear(cgimage_c &screen, bool obsured) {
    screen.draw_aligned(rsc.background, (cgpoint_t){0, 0});
    _menu_buttons.draw_all(screen);

    screen.draw(rsc.font, "Choose Level", (cgpoint_t){96, 16});

    for (auto group = _select_button_groups.begin(); group != _select_button_groups.end(); group++) {
        group->draw_all(screen);
    }
}

void cglevel_select_scene_c::tick(cgimage_c &screen, int ticks) {
    int button = update_button_group(screen, _menu_buttons);
    if (button == 0) {
        manager.pop();
        return;
    }

    int row = 0;
    for (auto group = _select_button_groups.begin(); group != _select_button_groups.end(); group++) {
        button = update_button_group(screen, *group);
        if (button >= 0) {
            int level = row * 5 + button;
            manager.replace(new cglevel_scene_c(manager, level));
            return;
        }
        row++;
    }
}
