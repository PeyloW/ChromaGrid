//
//  game_intro.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-03-01.
//

#include "game.hpp"
#include "utility.hpp"

cgintro_scene_c::cgintro_scene_c(scene_manager_c &manager) : 
    cggame_scene_c(manager),
    _menu_buttons(MAIN_MENU_BUTTONS_ORIGIN, MAIN_MENU_BUTTONS_SIZE, MAIN_MENU_BUTTONS_SPACING)
{
    const char *button_titles[] = { "Exit", "Credits", "Help", "Editor", "Hi-Scores", "PLAY", nullptr };
    for (auto title = &button_titles[0]; *title; title++) {
        _menu_buttons.add_button(*title);
    }
    _menu_buttons.buttons[0].state = cgbutton_t::disabled;
}

void cgintro_scene_c::will_appear(screen_c &screen, bool obsured) {
    auto &canvas = screen.canvas();
    canvas.draw_aligned(rsc.background, point_s());
        
    for (int y = 0; y < 12; y++) {
        for (int x = 0; x < 12; x++) {
            int dx = ABS(x * 2 - 11);
            int dy = ABS(y * 2 - 11);
            int dist = sqrt((dx * dx + dy * dy) * 8);
            auto r = (uint16_t)rand();
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
                int16_t idx = col + row * 9;
                point_s at(x * 16, y * 16);
                canvas.with_stencil(canvas_c::stencil(canvas_c::orderred, shade), [&] {
                    canvas.draw_aligned(rsc.tiles, idx, at);
                });
            }
        }
    }
    
    _menu_buttons.draw_all(canvas);
    
    canvas.draw(rsc.font, "Welcome to Chroma Grid.", point_s(96, 150));
    canvas.draw(rsc.font, "\x7f 2024 T.O.Y.S.", point_s(96, 170));
}

void cgintro_scene_c::update_background(screen_c &screen, int ticks) {
    auto &canvas = screen.canvas();
    int button = update_button_group(canvas, _menu_buttons);
    switch (button) {
        case 0:
            manager.pop();
            break;
        case 1:
            manager.push(new cgcredits_scene_c(manager));
            break;
        case 2:
            manager.push(new cghelp_scene_c(manager));
            break;
        case 3:
            manager.push(new cglevel_edit_scene_c(manager, nullptr));
            break;
        case 4:
            manager.push(new cgscores_scene_c(manager));
            break;
        case 5: {
#ifndef ALLOW_FULL_LEVEL_SELECT
            if (rsc.level_results.front().score == 0) {
                auto transition = transition_c::create(g_active_palette->colors[0]);
                manager.push(new cglevel_scene_c(manager, 0), transition);
            } else 
#endif
            {
                manager.push(new cglevel_select_scene_c(manager));
            }
            break;
        }
        default:
            break;
    }
}

cglevel_select_scene_c::cglevel_select_scene_c(scene_manager_c &manager) :
    cggame_scene_c(manager),
    _menu_buttons(MAIN_MENU_BUTTONS_ORIGIN, MAIN_MENU_BUTTONS_SIZE, MAIN_MENU_BUTTONS_SPACING)
{
    _menu_buttons.add_button("Back");
    
    static char title_buf[3 * 45];
    int index = 0;
    point_s origin = point_s(16, 40);
    const size_s size = size_s(26, 14);
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
#ifndef ALLOW_FULL_LEVEL_SELECT
        if (result->score == 0) {
            disable = true;
        }
#endif
        
        index++;
    }
}

void cglevel_select_scene_c::will_appear(screen_c &screen, bool obsured) {
    auto &canvas = screen.canvas();
    canvas.draw_aligned(rsc.background, point_s());
    _menu_buttons.draw_all(canvas);

    canvas.draw(rsc.font, "Choose Level", point_s(96, 16));

    for (auto group = _select_button_groups.begin(); group != _select_button_groups.end(); group++) {
        group->draw_all(canvas);
    }
}

void cglevel_select_scene_c::update_background(screen_c &screen, int ticks) {
    auto &canvas = screen.canvas();
    int button = update_button_group(canvas, _menu_buttons);
    if (button == 0) {
        manager.pop();
        return;
    }

    int row = 0;
    for (auto group = _select_button_groups.begin(); group != _select_button_groups.end(); group++) {
        button = update_button_group(canvas, *group);
        if (button >= 0) {
            int level = row * 5 + button;
            auto transition = transition_c::create(g_active_palette->colors[0]);
            manager.replace(new cglevel_scene_c(manager, level), transition);
            return;
        }
        row++;
    }
}
