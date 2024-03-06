//
//  game_level.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-03-04.
//

#include "game.hpp"

cglevel_scene_c::cglevel_scene_c(cgmanager_c &manager, int level) :
    cggame_scene_c(manager),
    _menu_buttons((cgpoint_t){200, 184}, (cgsize_t){112, 14}, -6),
    _level_num(level),
    _level(rsc.levels[level])
{
    _menu_buttons.add_button("Main Menu");
    _menu_buttons.add_button("Restart");
}

void cglevel_scene_c::will_appear(cgimage_c &screen, bool obsured) {
    screen.draw_aligned(rsc.background, (cgpoint_t){0, 0});
    _menu_buttons.draw_all(screen);
    _level.draw_all(screen);
}

void cglevel_scene_c::tick(cgimage_c &screen, int ticks) {
    int button = update_button_group(screen, _menu_buttons);
    switch (button) {
        case 0:
            manager.pop();
            return;
        case 1:
            manager.replace(new cglevel_scene_c(manager, _level_num));
            return;;
        default:
            break;
    }
    auto state = _level.update_tick(screen, manager.mouse, ticks);
    if (state != level_t::normal) {
        cglevel_ended_scene_c::level_state_t level_state = {
            _level_num, state
        };
        _level.get_remaining(&level_state.orbs_left, &level_state.time_left);
        manager.replace(new cglevel_ended_scene_c(manager, level_state));
    }
}

cglevel_ended_scene_c::cglevel_ended_scene_c(cgmanager_c &manager, level_state_t &state) :
    cggame_scene_c(manager),
    _menu_buttons((cgpoint_t){200, 184}, (cgsize_t){112, 14}, -6),
    _state(state)
{
    _menu_buttons.add_button("Main Menu");
    if (state.state == level_t::failed) {
        _menu_buttons.add_button("Retry Level");
    } else {
        _menu_buttons.add_button("Next Level");
    }
}

void cglevel_ended_scene_c::will_appear(cgimage_c &screen, bool obsured) {
    screen.draw_aligned(rsc.background, (cgpoint_t){0, 0});
    _menu_buttons.draw_all(screen);

    const char *title = _state.state == level_t::failed ? "Level Failed" : "Level Completed";
    screen.draw(rsc.font, title, (cgpoint_t){96, 32});
    
    char buf[32];
    sprintf(buf, "Time: %d x 10 = %d", _state.time_left, _state.time_left * 10);
    screen.draw(rsc.font, buf, (cgpoint_t){96, 64});
    sprintf(buf, "Orbs: %d x 100 = %d", _state.orbs_left, _state.orbs_left * 100);
    screen.draw(rsc.font, buf, (cgpoint_t){96, 84});
    sprintf(buf, "Total: %d pts", _state.time_left * 10 + _state.orbs_left * 100);
    screen.draw(rsc.font, buf, (cgpoint_t){96, 114});
}

void cglevel_ended_scene_c::tick(cgimage_c &screen, int ticks) {
    int button = update_button_group(screen, _menu_buttons);
    switch (button) {
        case 0:
            manager.pop();
            return;
        case 1: {
            auto next_level = (_state.state == level_t::failed) ? _state.level_num : (_state.level_num + 1) % rsc.levels.size();
            manager.replace(new cglevel_scene_c(manager, next_level));
            return;
        }
        default:
            break;
    }
}


cglevel_edit_scene_c::cglevel_edit_scene_c(cgmanager_c &manager, level_t::recipe_t *recipe) :
    cggame_scene_c(manager),
    _menu_buttons((cgpoint_t){200, 184}, (cgsize_t){112, 14}, -6),
    _selected_template(0)
{
    memset(_level_grid, 0, sizeof(_level_grid));
    _menu_buttons.add_button("Main Menu");
    _menu_buttons.add_buttons("Load", "Save");
    _menu_buttons.add_button("Try Level");
    for (int i = 1; i < 4; i++) {
        _menu_buttons.buttons[i].state = cgbutton_t::disabled;
    }
    
    _tile_templates.push_back((tilestate_t){ tiletype_e::regular, color_e::none, color_e::none, color_e::none});
    _tile_templates.push_back((tilestate_t){ tiletype_e::regular, color_e::gold, color_e::none, color_e::none});
    _tile_templates.push_back((tilestate_t){ tiletype_e::regular, color_e::silver, color_e::none, color_e::none});
    _tile_templates.push_back((tilestate_t){ tiletype_e::magnetic, color_e::none, color_e::none, color_e::none});
    _tile_templates.push_back((tilestate_t){ tiletype_e::magnetic, color_e::gold, color_e::none, color_e::none});
    _tile_templates.push_back((tilestate_t){ tiletype_e::magnetic, color_e::silver, color_e::none, color_e::none});
    _tile_templates.push_back((tilestate_t){ tiletype_e::glass, color_e::none, color_e::none, color_e::none});
    _tile_templates.push_back((tilestate_t){ tiletype_e::glass, color_e::gold, color_e::none, color_e::none});
    _tile_templates.push_back((tilestate_t){ tiletype_e::glass, color_e::silver, color_e::none, color_e::none});
    _tile_templates.push_back((tilestate_t){ tiletype_e::broken, color_e::none, color_e::none, color_e::none});
    _tile_templates.push_back((tilestate_t){ tiletype_e::broken, color_e::gold, color_e::none, color_e::none});
    _tile_templates.push_back((tilestate_t){ tiletype_e::broken, color_e::silver, color_e::none, color_e::none});
    _tile_templates.push_back((tilestate_t){ tiletype_e::blocked, color_e::none, color_e::none, color_e::none});
    _tile_templates.push_back((tilestate_t){ tiletype_e::empty, color_e::none, color_e::none, color_e::gold});
    _tile_templates.push_back((tilestate_t){ tiletype_e::empty, color_e::none, color_e::none, color_e::silver});
}

void cglevel_edit_scene_c::will_appear(cgimage_c &screen, bool obsured) {
    screen.draw_aligned(rsc.background, (cgpoint_t){0, 0});
    _menu_buttons.draw_all(screen);
    draw_tile_templates(screen);
    for (int y = 0; y < 12; y++) {
        for (int x = 0; x < 12; x++) {
            draw_level_grid(screen, x, y);
        }
    }
}

void cglevel_edit_scene_c::tick(cgimage_c &screen, int ticks) {
    int button = update_button_group(screen, _menu_buttons);
    switch (button) {
        case 0:
            manager.pop();
            break;
        case 1: // Load level
            break;
        case 2: // Save level
            break;
        case 3: // Try level
            break;
        default:
            break;
    }
    
    auto &mouse = manager.mouse;
    bool lb = mouse.get_state(cgmouse_c::left) == cgmouse_c::clicked;
    bool rb = mouse.get_state(cgmouse_c::right) == cgmouse_c::clicked;
    const auto pos = mouse.get_postion();
    if (lb || rb) {
        if (pos.x < 192 && pos.y < 192) {
            int tx = pos.x / 16;
            int ty = pos.y / 16;
            if (lb) {
                _level_grid[tx][ty] = _tile_templates[_selected_template];
            } else if (rb) {
                _level_grid[tx][ty] = (tilestate_t) { empty, none, none, none};
            }
            draw_level_grid(screen, tx, ty);
        }
    }
}

void cglevel_edit_scene_c::draw_tile_templates(cgimage_c &screen) const {
    for (int i = 0; i < 15; i++) {
        int row = i / 6;
        int col = i % 6;
        int16_t x = 192 + 16 + col * 16;
        int16_t y = 80 + row * 16;
        cgpoint_t at = (cgpoint_t){x, y};
        draw_tilestate(screen, _tile_templates[i], at, _selected_template == i);
    }
}

void cglevel_edit_scene_c::draw_level_grid(cgimage_c &screen, int x, int y) const {
    cgpoint_t at = (cgpoint_t){(int16_t)(x * 16), (int16_t)(y * 16)};
    draw_tilestate(screen, _level_grid[x][y], at);
}
