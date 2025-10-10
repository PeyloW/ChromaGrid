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
    _menu_buttons.buttons[3 - (int)scoring].state = cgbutton_t::state_e::disabled;
}

void cgscores_scene_c::will_appear(screen_c &clear_screen, bool obsured) {
    auto &canvas = clear_screen;
    canvas.draw_aligned(background, point_s());
    _menu_buttons.draw_all(canvas);
    
    switch (_scoring) {
        case scoring_e::score:
            canvas.draw(font, "Hi-Scores", point_s(96, 16));
            break;
        case scoring_e::time:
            canvas.draw(font, "Best Times", point_s(96, 16));
            break;
        case scoring_e::moves:
            canvas.draw(font, "Least Moves", point_s(96, 16));
            break;
    }
    
    int index = 0;
    char buf[12];
    strstream_c str(buf, 12);
    for (auto &result : assets.level_results()) {
        int col = index % 3;
        int row = index / 3;
        str.reset();
        str.fill(' ');
        str.width(2);
        str << (int16_t)(index + 1) << ':';
        if (result.score == 0) {
            if (_scoring == scoring_e::time) {
                str << " -:--" << ends;
            } else {
                str << "    -" << ends;
            }
        } else {
            switch (_scoring) {
                case scoring_e::score:
                    str << setw(5) << result.score;
                    break;
                case scoring_e::time:
                    str << (int16_t)(result.time / 60) << ':' << setfill('0') << (int16_t)(result.time % 60);
                    break;
                case scoring_e::moves:
                    str << setw(5) << result.moves;
                    break;
            }
        }
        str << ends;
        point_s at(16 + col * 55, 16 + 20 + 14 * row);
        canvas.draw(assets.font(SMALL_MONO_FONT), str.str(), at, canvas_c::alignment_e::left);
        
        index++;
    }
}

void cgscores_scene_c::update_clear(screen_c &clear_screen, int ticks) {
    auto &canvas = clear_screen;
    int button = update_button_group(canvas, _menu_buttons);
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
