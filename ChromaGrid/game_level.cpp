//
//  game_level.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-03-04.
//

#include "game.hpp"
#include "machine.hpp"


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
                if (assets.level_results()[level_num].merge_from(results)) {
                    assets.save_level_results();
                }
            }
        }
    };

    virtual void will_appear(screen_c &clear_screen, bool obsured) {
        auto &canvas = clear_screen.canvas();
        
        rect_s rect(0, 0, MAIN_MENU_ORIGIN_X, 200);
        canvas.with_stencil(canvas_c::stencil(canvas_c::orderred, 48), [this, &canvas, &rect] {
            canvas.draw_aligned(background, rect, rect.origin);
        });
        rect = rect_s(
            MAIN_MENU_ORIGIN_X, 0,
            MAIN_MENU_SIZE_WIDTH, 200
        );
        canvas.draw_aligned(background, rect, rect.origin);
        _menu_buttons.draw_all(canvas);

        if (_level_num != cglevel_scene_c::TEST_LEVEL) {
            char buf[20];
            strstream_c str(buf, 20);
            const bool failed = _results.score == level_result_t::FAILED_SCORE;
            str << "Level " << _level_num + 1 << (failed ? " Failed" : " Completed") << ends;
            canvas.draw(font, buf, point_s(96, 32));
        } else {
            const char *title = _results.score == level_result_t::FAILED_SCORE ? "Level Failed" : "Level Completed";
            canvas.draw(font, title, point_s(96, 32));
        }
        
        if (_results.score != level_result_t::FAILED_SCORE) {
            char buf[32];
            strstream_c str(buf, 32);
            uint16_t time_score, orbs_score;
            _results.subscores(orbs_score, time_score);
            str << "Time: " << _results.time << " x 10 = " << time_score << ends;
            canvas.draw(font, buf, point_s(96, 64));
            str.reset();
            str << "Orbs: " << _results.orbs[0] + _results.orbs[1] << " x 100 = " << orbs_score << ends;
            canvas.draw(font, buf, point_s(96, 84));
            str.reset();
            str << "Total: " << _results.score << " pts" << ends;
            canvas.draw(font, buf, point_s(96, 114));
            str.reset();
            str << "Moves: " << _results.moves << ends;
            canvas.draw(font, buf, point_s(96, 144));
        }
    }

    virtual void update_clear(screen_c &clear_screen, int ticks) {
        auto &canvas = clear_screen.canvas();
        int button = update_button_group(canvas, _menu_buttons);
        switch (button) {
            case 0:
                manager.pop();
                return;
            case 1: {
                auto next_level = (_results.score == level_result_t::FAILED_SCORE) ? _level_num: (_level_num + 1) % assets.levels().size();
                auto color = machine_c::shared().active_palette()->colors[0];
                auto transition = transition_c::create(color);
                manager.replace(new cglevel_scene_c(manager, next_level), transition);
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
    _level(assets.levels()[level])
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

void tick_second(cglevel_scene_c *that) {
    that->_passed_seconds++;
}

static int next_shimmer_ticks() {
    return 100 + (uint16_t)rand() % 200;
}

void cglevel_scene_c::will_appear(screen_c &clear_screen, bool obsured) {
    auto &canvas = clear_screen.canvas();
    canvas.draw_aligned(background, point_s());
    char buffer[256] = {0};
    strstream_c str(buffer, 256);
    if (_level_num == TEST_LEVEL) {
        str << "Testing level";
    } else {
        str << "Level " << _level_num + 1;
        auto text = assets.levels()[_level_num]->text;
        if (text) {
            str << ": " << text;
        }
    }
    str << ends;
    canvas.draw(small_font, buffer, rect_s(8, 193, 304, 6), 0, canvas_c::align_left);

    _menu_buttons.draw_all(canvas);
    _level.draw_all(canvas);
    _shimmer_ticks = next_shimmer_ticks();
    _shimmer_tile = -1;
    _passed_seconds = 0;
    manager.vbl.add_func((timer_c::func_a_t)&tick_second, this, 1);
}

void cglevel_scene_c::will_disappear(bool obscured) {
    manager.vbl.remove_func((timer_c::func_a_t)&tick_second, this);
};

void cglevel_scene_c::update_clear(screen_c &clear_screen, int ticks) {
    auto &canvas = clear_screen.canvas();
    int button = update_button_group(canvas, _menu_buttons);
    switch (button) {
        case 0:
            manager.pop();
            return;
        case 1: {
            auto color = machine_c::shared().active_palette()->colors[0];
            auto transition = transition_c::create(color);
            if (_level_num == -1) {
                manager.replace(new cglevel_scene_c(manager, _recipe), transition);
            } else {
                manager.replace(new cglevel_scene_c(manager, _level_num), transition);
            }
            return;
        }
        default:
            break;
    }
    auto passed = _passed_seconds;
    _passed_seconds = 0;
    auto state = _level.update_tick(canvas, mouse, passed);
    if (state != level_t::normal) {
        level_result_t results;
        _level.results(&results);
        results.calculate_score(state == level_t::success);
        manager.replace(new cglevel_ended_scene_c(manager, _level_num, results));
    }
}

void cglevel_scene_c::update_back(screen_c &back_screen, int ticks) {
    auto &canvas = back_screen.canvas();
    _shimmer_ticks -= ticks;
    if (_shimmer_tile != -1) {
        if (_shimmer_ticks < -7) {
            _shimmer_ticks = next_shimmer_ticks();
            _shimmer_tile = -1;
        } else {
            int16_t idx = ABS(_shimmer_ticks);
            point_s at(_shimmer_tile % 12 * 16, _shimmer_tile / 12 * 16);
            canvas.draw(assets.tileset(SHIMMER), idx, at);
        }
    } else if (_shimmer_ticks <= 0) {
        _shimmer_ticks = 0;
        int tile = ((uint16_t)rand()) % (12 * 12);
        if (_level.tilestate_at(tile % 12, tile / 12).type != empty) {
            _shimmer_tile = tile;
        }
    }
}
