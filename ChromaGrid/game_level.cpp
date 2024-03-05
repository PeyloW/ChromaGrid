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
    _previous_tick = manager.vbl.tick();
    _menu_buttons.add_button("Abort");
    _menu_buttons.add_button("Restart");
}

void cglevel_scene_c::will_appear(cgimage_c &screen, bool obsured) {
    screen.draw_aligned(rsc.background, (cgpoint_t){0, 0});
    _menu_buttons.draw_all(screen);
    _level.draw_all(screen);
    _previous_tick = manager.vbl.tick();
}

void cglevel_scene_c::tick(cgimage_c &screen) {
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
    auto tick = manager.vbl.tick();
    auto state = _level.update_tick(screen, tick - _previous_tick);
    _previous_tick = tick;
    switch (state) {
        case level_t::failed:
            manager.pop();
            break;
        case level_t::success:
            manager.replace(new cglevel_scene_c(manager, _level_num + 1));
            break;
        default:
            break;
    }
}
