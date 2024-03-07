//
//  game_level.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-03-04.
//

#include "game.hpp"


class cglevel_ended_scene_c : public cggame_scene_c {
public:
    typedef struct {
        int level_num;
        level_t::state_e state;
        uint8_t orbs_left;
        uint8_t time_left;
    } level_state_t;
    
    cglevel_ended_scene_c(cgmanager_c &manager, level_state_t &state) :
        cggame_scene_c(manager),
        _menu_buttons(MAIN_MENU_BUTTONS_ORIGIN, MAIN_MENU_BUTTONS_SIZE, MAIN_MENU_BUTTONS_SPACING),
        _state(state)
    {
        if (state.level_num == cglevel_scene_c::TEST_LEVEL) {
            _menu_buttons.add_button("Back");
        } else {
            _menu_buttons.add_button("Main Menu");
            if (state.state == level_t::failed) {
                _menu_buttons.add_button("Retry Level");
            } else {
                _menu_buttons.add_button("Next Level");
            }
        }
    };

    virtual void will_appear(cgimage_c &screen, bool obsured) {
        cgrect_t rect = (cgrect_t) {
            {0,0},
            {MAIN_MENU_ORIGIN_X, 200}
        };
        screen.with_stencil(cgimage_c::get_stencil(cgimage_c::orderred, 32), [this, &screen, &rect] {
            screen.draw_aligned(rsc.background, rect, rect.origin);
        });
        rect = (cgrect_t){
            (cgpoint_t){MAIN_MENU_ORIGIN_X, 0},
            (cgsize_t){MAIN_MENU_SIZE_WIDTH, 200}
        };
        screen.draw_aligned(rsc.background, rect, rect.origin);
        _menu_buttons.draw_all(screen);

        const char *title = _state.state == level_t::failed ? "Level Failed" : "Level Completed";
        screen.draw(rsc.font, title, (cgpoint_t){96, 32});
        
        if (_state.state == level_t::success) {
            char buf[32];
            sprintf(buf, "Time: %d x 10 = %d", _state.time_left, _state.time_left * 10);
            screen.draw(rsc.font, buf, (cgpoint_t){96, 64});
            sprintf(buf, "Orbs: %d x 100 = %d", _state.orbs_left, _state.orbs_left * 100);
            screen.draw(rsc.font, buf, (cgpoint_t){96, 84});
            sprintf(buf, "Total: %d pts", _state.time_left * 10 + _state.orbs_left * 100);
            screen.draw(rsc.font, buf, (cgpoint_t){96, 114});
        }
    }

    virtual void tick(cgimage_c &screen, int ticks) {
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
private:
    cgbutton_group_c<2> _menu_buttons;
    level_state_t _state;
};


cglevel_scene_c::cglevel_scene_c(cgmanager_c &manager, int level) :
    cggame_scene_c(manager),
    _menu_buttons(MAIN_MENU_BUTTONS_ORIGIN, MAIN_MENU_BUTTONS_SIZE, MAIN_MENU_BUTTONS_SPACING),
    _level_num(level),
    _level(rsc.levels[level])
{
    _menu_buttons.add_button("Main Menu");
    _menu_buttons.add_button("Restart");
}

cglevel_scene_c::cglevel_scene_c(cgmanager_c &manager, level_t::recipe_t *recipe) :
    cggame_scene_c(manager),
    _menu_buttons(MAIN_MENU_BUTTONS_ORIGIN, MAIN_MENU_BUTTONS_SIZE, MAIN_MENU_BUTTONS_SPACING),
    _level_num(TEST_LEVEL),
    _level(recipe)
{
    _menu_buttons.add_button("Back");
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
