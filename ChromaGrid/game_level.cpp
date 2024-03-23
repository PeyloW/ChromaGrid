//
//  game_level.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-03-04.
//

#include "game.hpp"


class cglevel_ended_scene_c : public cggame_scene_c {
public:
    
    cglevel_ended_scene_c(scene_manager_c &manager, int level_num, level_result_t &results) :
        cggame_scene_c(manager),
        _menu_buttons(MAIN_MENU_BUTTONS_ORIGIN, MAIN_MENU_BUTTONS_SIZE, MAIN_MENU_BUTTONS_SPACING),
        _level_num(level_num),
        _results(results)
    {
        if (_level_num == cglevel_scene_c::TEST_LEVEL) {
            _menu_buttons.add_button("Back");
        } else {
            _menu_buttons.add_button("Main Menu");
            if (_results.score == level_result_t::FAILED_SCORE) {
                _menu_buttons.add_button("Retry Level");
            } else {
                _menu_buttons.add_button("Next Level");
                if (rsc.level_results[level_num].merge_from(results)) {
                    rsc.save_level_results();
                }
            }
        }
    };

    virtual void will_appear(image_c &screen, bool obsured) {
        rect_s rect = (rect_s) {
            {0,0},
            {MAIN_MENU_ORIGIN_X, 200}
        };
        screen.with_stencil(image_c::get_stencil(image_c::orderred, 32), [this, &screen, &rect] {
            screen.draw_aligned(rsc.background, rect, rect.origin);
        });
        rect = (rect_s){
            (point_s){MAIN_MENU_ORIGIN_X, 0},
            (size_s){MAIN_MENU_SIZE_WIDTH, 200}
        };
        screen.draw_aligned(rsc.background, rect, rect.origin);
        _menu_buttons.draw_all(screen);

        const char *title = _results.score == level_result_t::FAILED_SCORE ? "Level Failed" : "Level Completed";
        screen.draw(rsc.font, title, (point_s){96, 32});
        
        if (_results.score != level_result_t::FAILED_SCORE) {
            char buf[32];
            uint16_t time_score, orbs_score;
            _results.get_subscores(orbs_score, time_score);
            sprintf(buf, "Time: %d x 10 = %d", _results.time, time_score);
            screen.draw(rsc.font, buf, (point_s){96, 64});
            sprintf(buf, "Orbs: %d x 100 = %d", _results.orbs[0] + _results.orbs[1], orbs_score);
            screen.draw(rsc.font, buf, (point_s){96, 84});
            sprintf(buf, "Total: %d pts", _results.score);
            screen.draw(rsc.font, buf, (point_s){96, 114});
            sprintf(buf, "Moves: %d", _results.moves);
            screen.draw(rsc.font, buf, (point_s){96, 144});
        }
    }

    virtual void tick(image_c &screen, int ticks) {
        int button = update_button_group(screen, _menu_buttons);
        switch (button) {
            case 0:
                manager.pop();
                return;
            case 1: {
                auto next_level = (_results.score == level_result_t::FAILED_SCORE) ? _level_num: (_level_num + 1) % rsc.levels.size();
                manager.replace(new cglevel_scene_c(manager, next_level));
                return;
            }
            default:
                break;
        }
    }
private:
    cgbutton_group_c<2> _menu_buttons;
    int _level_num;
    level_result_t _results;
};


cglevel_scene_c::cglevel_scene_c(scene_manager_c &manager, int level) :
    cggame_scene_c(manager),
    _menu_buttons(MAIN_MENU_BUTTONS_ORIGIN, MAIN_MENU_BUTTONS_SIZE, MAIN_MENU_BUTTONS_SPACING),
    _level_num(level),
    _recipe(nullptr),
    _level(rsc.levels[level])
{
    _menu_buttons.add_button("Main Menu");
    _menu_buttons.add_button("Restart");
}

cglevel_scene_c::cglevel_scene_c(scene_manager_c &manager, level_recipe_t *recipe) :
    cggame_scene_c(manager),
    _menu_buttons(MAIN_MENU_BUTTONS_ORIGIN, MAIN_MENU_BUTTONS_SIZE, MAIN_MENU_BUTTONS_SPACING),
    _level_num(TEST_LEVEL),
    _recipe(recipe),
    _level(recipe)
{
    _menu_buttons.add_button("Back");
    _menu_buttons.add_button("Restart");
}

void cglevel_scene_c::will_appear(image_c &screen, bool obsured) {
    screen.draw_aligned(rsc.background, (point_s){0, 0});
    _menu_buttons.draw_all(screen);
    _level.draw_all(screen);
}

void cglevel_scene_c::tick(image_c &screen, int ticks) {
    int button = update_button_group(screen, _menu_buttons);
    switch (button) {
        case 0:
            manager.pop();
            return;
        case 1:
            if (_level_num == -1) {
                manager.replace(new cglevel_scene_c(manager, _recipe));
            } else {
                manager.replace(new cglevel_scene_c(manager, _level_num));
            }
            return;
        default:
            break;
    }
    auto state = _level.update_tick(screen, manager.mouse, ticks);
    if (state != level_t::normal) {
        level_result_t results;
        _level.get_results(&results);
        results.calculate_score(state == level_t::success);
        manager.replace(new cglevel_ended_scene_c(manager, _level_num, results));
    }
}
