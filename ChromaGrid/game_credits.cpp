//
//  game_credits.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-03-03.
//

#include "game.hpp"

cgcredits_scene_c::cgcredits_scene_c(cgmanager_c &manager) :
    cggame_scene_c(manager),
    _menu_buttons(MAIN_MENU_BUTTONS_ORIGIN, MAIN_MENU_BUTTONS_SIZE, MAIN_MENU_BUTTONS_SPACING)
{
    _menu_buttons.add_button("Back");
}

void cgcredits_scene_c::will_appear(cgimage_c &screen, bool obsured) {
    screen.draw_aligned(rsc.background, (cgpoint_t){0, 0});
    _menu_buttons.draw_all(screen);
    
    /*
     * Main Code: _Fredrik 'PeyloW' Olsson_
     * Blitter Code: _Hatari 1.0 as reference_
     * Music: _Joakim 'AiO' Ekblad_
     * Graphics: ??
     * Font: _Damien Guard_
     * Concept: Peter 'Eagle' Nyman
     */
    
    struct { const char *credit; const char *person; } credits[5] = {
        {"Code:", "Fredrik 'PeyloW' Olsson"},
        {"Graphics:", "Herve 'Exocet' Piton"},
        {"Music:", "Joakim 'AiO' Ekblad"},
        {"Fonts:", "Damien Guard"},
        {"Concept:", "Peter 'Eagle' Nyman"}
    };
    
    cgpoint_t atc = (cgpoint_t){16, 16};
    cgpoint_t atp = (cgpoint_t){40, 26};
    for (int i = 0; i < 5; i++) {
        screen.draw(rsc.font, credits[i].credit, atc, cgimage_c::align_left);
        atc.y += 26;
        screen.draw(rsc.font, credits[i].person, atp, cgimage_c::align_left);
        atp.y += 26;
    }
    
    atc.y += 8;
    
    cgrect_t rect = (cgrect_t){ atc, {160, 7 * 4}};
    screen.draw(rsc.small_font,
        "Released at Sommarhack 2024.\n"
        "Special thanks to Anders 'evl' Erikson and friends who stayed Atari.\n"
        "Dedicated to our families."
        , rect, 2);
}


void cgcredits_scene_c::tick(cgimage_c &screen, int ticks) {
    int button = update_button_group(screen, _menu_buttons);
    switch (button) {
        case 0:
            manager.pop();
            break;
        default:
            break;
    }
}
