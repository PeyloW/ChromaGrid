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
    
    // TODO: Draw scores here.
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
