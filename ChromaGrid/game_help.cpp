//
//  game_help.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-03-25.
//

#include "game.hpp"

cghelp_scene_c::cghelp_scene_c(scene_manager_c &manager, page_e page) :
    cggame_scene_c(manager),
    _page(page),
    _menu_buttons(MAIN_MENU_BUTTONS_ORIGIN, MAIN_MENU_BUTTONS_SIZE, MAIN_MENU_BUTTONS_SPACING)
{
    const char *button_titles[] = { "Back", "Level Editor", "Scoring", "Special Tiles", "Basics", nullptr };
    for (auto title = &button_titles[0]; *title; title++) {
        _menu_buttons.add_button(*title);
    }
    _menu_buttons.buttons[4 - (int)page].state = cgbutton_t::disabled;
}


static const char *basics_texts[] = {
    "The goal of each level is to color all the marked tiles on the board according to their target color within the time limit with the available orbs.",
    "Orbs are placed on tiles using the left button for gold, and right button for silver. Clicking an orb on the board picks it up again. Placing or pickup up an orb counts as one move.",
    "Placing an orb on a tile next to an empty space on the board creates a new tile in the empty space of the same type. Diagonal placement does not count.",
    "Any orb adjacent to three or more orbs of the same color is fused. A fused orb is destroyed from play and the tile it occupies takes its color. Many orbs can be fused in a single move.",
    nullptr
};

static const char *special_tiles_texts[] = {
    "Orbs cannot be placed on blocked tiles or broken glass tiles.",
    "A glass tile breaks when an orb is removed from it by picking it up or by fusing it.",
    "Orbs can only be removed from magnetic tiles by fusing them, not by picking them up.",
    nullptr
};

static const char *scoring_texts[] = {
    "Each level is scored by remaining Time, number of Moves used and Points.",
    "Each remaining second is worth 10pts, and each remaining orb not on the board is worth 100pts.",
    "Optimizing your strategy for Time, Moves, or Points may penalize the other scores.  Experiment and replay levels to get high scores in all three.",
    nullptr
};

static const char *level_editor_texts[] = {
    "Clicking on the grid updates tiles based on the currently selected tool.",
    "With a tile type tool selected, left-clicking cycles through the target color and right-clicking cycles through the current color.",
    "With the orbs tool selected a left click places a gold orb and a right click places a silver orb. Clicking a tile with an existing orb removes the orb.",
    "Use the + and - buttons to increment and decrement the available orbs and time. Time is incremented and decremented in five second steps.",
    "The editor cannot verify that it is possible to complete a level. Use Try Level to test the level and adjust for reasonable time and available orbs.",
    "Save and Load allows you to store up to 10 custom made levels.",
    nullptr
};

static void draw_help(const cgresources_c &rsc, canvas_c &screen, const char *title, const char *texts[]) {
    screen.draw(rsc.font, title, point_s(96, 12));
    rect_s rect(7, 28, 176, 48);
    for (auto text = &texts[0]; *text; text++) {
        auto size = screen.draw(rsc.small_font, *text, rect, 2, canvas_c::align_left);
        rect.origin.y += size.height + 4;
    }
}


void cghelp_scene_c::will_appear(canvas_c &screen, bool obsured) {
    screen.draw_aligned(rsc.background, point_s());
    _menu_buttons.draw_all(screen);
    const auto &rsc = this->rsc;
    
    switch (_page) {
        case basics:
            draw_help(rsc, screen, "Basics", basics_texts);
            break;
        case special_tiles:
            draw_help(rsc, screen, "Special Tiles", special_tiles_texts);
            break;
        case scoring:
            draw_help(rsc, screen, "Scoring", scoring_texts);
            break;
        case level_editor:
            draw_help(rsc, screen, "Level Editor", level_editor_texts);
            break;
    }
}

void cghelp_scene_c::update_background(canvas_c &screen, int ticks) {
    int button = update_button_group(screen, _menu_buttons);
    switch (button) {
        case -1:
            break;
        case 0:
            manager.pop();
            break;
        default:
            manager.replace(new cghelp_scene_c(manager, (page_e)(4 - button)));
            break;
    }
}
