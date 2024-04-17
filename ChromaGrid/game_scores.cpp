//
//  game_scores.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-03-14.
//

#include "game.hpp"

cgscores_scene_c::cgscores_scene_c(scene_manager_c &manager, scoring_e scoring) :
    cggame_scene_c(manager),
    _scoring(scoring),
    _menu_buttons(MAIN_MENU_BUTTONS_ORIGIN, MAIN_MENU_BUTTONS_SIZE, MAIN_MENU_BUTTONS_SPACING)
{
    const char *button_titles[5] = { "Back", "Least Moves", "Best Times", "Hi-Scores", nullptr };
    for (auto title = &button_titles[0]; *title; title++) {
        _menu_buttons.add_button(*title);
    }
    _menu_buttons.buttons[3 - (int)scoring].state = cgbutton_t::disabled;
}

void cgscores_scene_c::will_appear(canvas_c &screen, bool obsured) {
    screen.draw_aligned(rsc.background, point_s());
    _menu_buttons.draw_all(screen);
    
    switch (_scoring) {
        case score:
            screen.draw(rsc.font, "Hi-Scores", point_s(96, 16));
            break;
        case time:
            screen.draw(rsc.font, "Best Times", point_s(96, 16));
            break;
        case moves:
            screen.draw(rsc.font, "Least Moves", point_s(96, 16));
            break;
    }
    
    int index = 0;
    for (auto result = rsc.level_results.begin(); result != rsc.level_results.end(); result++) {
        char buf[12];
        int col = index % 3;
        int row = index / 3;
        if (result->score == 0) {
            if (_scoring == time) {
                sprintf(buf, "%2d: -:--", index + 1);
            } else {
                sprintf(buf, "%2d:    -", index + 1);
            }
        } else {
            switch (_scoring) {
                case score:
                    sprintf(buf, "%2d:%5d", index + 1, result->score);
                    break;
                case time:
                    sprintf(buf, "%2d: %d:%0d", index + 1, result->time / 60, result->time % 60);
                    break;
                case moves:
                    sprintf(buf, "%2d: %4d", index + 1, result->moves);
                    break;
            }
        }
        point_s at(16 + col * 55, 16 + 20 + 10 * row);
        screen.draw(rsc.small_mono_font, buf, at, canvas_c::align_left);
        
        index++;
    }
}

void cgscores_scene_c::update_background(canvas_c &screen, int ticks) {
    int button = update_button_group(screen, _menu_buttons);
    switch (button) {
        case -1:
            break;
        case 0:
            manager.pop();
            break;
        default:
            manager.replace(new cgscores_scene_c(manager, (scoring_e)(3 - button)));
            break;
    }
}
