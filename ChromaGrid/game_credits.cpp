//
//  game_credits.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-03-03.
//

#include "game.hpp"

cgcredits_scene_c::cgcredits_scene_c(cgmanager_c &manager, page_e page) :
    cggame_scene_c(manager),
    _page(page),
    _menu_buttons(MAIN_MENU_BUTTONS_ORIGIN, MAIN_MENU_BUTTONS_SIZE, MAIN_MENU_BUTTONS_SPACING)
{
    const char *button_titles[5] = { "Back", "Greetings", "Dedications", "Recognitions", "Credits" };
    for (int i = 0; i < 5; i++) {
        _menu_buttons.add_button(button_titles[i]);
    }
    _menu_buttons.buttons[4 - (int)page].state = cgbutton_t::disabled;
}

static void draw_credits(const cgresources_c &rsc, cgimage_c &screen) {
    struct { const char *credit; const char *person; } credits[5] = {
        {"Code:", "Fredrik 'PeyloW' Olsson"},
        {"Graphics:", "Herve 'Exocet' Piton"},
        {"Music:", "Joakim 'AiO' Ekblad"},
        {"Fonts:", "Damien Guard"},
        {"Concept:", "Peter 'Eagle' Nyman"}
    };
    
    cgpoint_t atc = (cgpoint_t){16, 32 +  0};
    cgpoint_t atp = (cgpoint_t){40, 32 + 10};
    for (int i = 0; i < 5; i++) {
        screen.draw(rsc.font, credits[i].credit, atc, cgimage_c::align_left);
        atc.y += 26;
        screen.draw(rsc.font, credits[i].person, atp, cgimage_c::align_left);
        atp.y += 26;
    }
}

// , ,

static void draw_recognitions(const cgresources_c &rsc, cgimage_c &screen) {
    screen.draw(rsc.font, "Recognitions", (cgpoint_t){96, 16});
    const char *texts[4] = {
        "This game uses royalty free sound effects from ZapSplat.\n(https://www.zapsplat.com/)",
        "This game uses libcmini by Thorsten Otto, Oliver and Markus, for the superiour speed and size.\n(https://github.com/freemint/libcmini)",
        "Blitter and audio setup code inspired by GODLib by Leon 'Mr. Pink' O'Reilly.\n(https://github.com/ReservoirGods/GODLIB).",
        "Original game idea conceived by Peter 'Eagle' Nyman of Friendchip, now realized 32 years later."
    };
    cgrect_t rect = (cgrect_t){{16, 40}, {160, 48}};
    for (int i = 0; i < 4; i++) {
        auto size = screen.draw(rsc.small_font, texts[i], rect, 2);
        rect.origin.y += size.height + 8;
    }
}

static void draw_dedications(const cgresources_c &rsc, cgimage_c &screen) {
    screen.draw(rsc.font, "Dedications", (cgpoint_t){96, 16});
    const char *texts[4] = {
        "Released at Sommarhack 2024.\n""Special thanks to Anders 'evl' Erikson and the friends who stayed Atari.",
        "Fredrik would like to thank Mia, Mondi, and Sturdy who endured the develpment.\nSpecial dedication to Marianne and Jan-Erik Peylow who I owe it all.",
        "Joakim would like to thank... people",
        "Herve would also like to thank... people"
    };
    cgrect_t rect = (cgrect_t){{16, 40}, {160, 48}};
    for (int i = 0; i < 4; i++) {
        auto size = screen.draw(rsc.small_font, texts[i], rect, 2);
        rect.origin.y += size.height + 8;
    }
}

static void draw_greetings(const cgresources_c &rsc, cgimage_c &screen) {
    screen.draw(rsc.font, "Greetings", (cgpoint_t){96, 16});
    const char *texts[3] = {
        "List of specific people.",
        "List of active Atari groups.",
        "List of the great groups that inspired us.",
    };
    cgrect_t rect = (cgrect_t){{16, 40}, {160, 48}};
    for (int i = 0; i < 3; i++) {
        auto size = screen.draw(rsc.small_font, texts[i], rect, 2);
        rect.origin.y += size.height + 8;
    }
}

void cgcredits_scene_c::will_appear(cgimage_c &screen, bool obsured) {
    screen.draw_aligned(rsc.background, (cgpoint_t){0, 0});
    _menu_buttons.draw_all(screen);
    const auto &rsc = this->rsc;
    
    switch (_page) {
        case credits:
            draw_credits(rsc, screen);
            break;
        case recognitions:
            draw_recognitions(rsc, screen);
            break;
        case dedications:
            draw_dedications(rsc, screen);
            break;
        case greetings:
            draw_greetings(rsc, screen);
            break;
    }
}


void cgcredits_scene_c::tick(cgimage_c &screen, int ticks) {
    int button = update_button_group(screen, _menu_buttons);
    switch (button) {
        case -1:
            break;
        case 0:
            manager.pop();
            break;
        default:
            manager.replace(new cgcredits_scene_c(manager, (page_e)(4 - button)));
            break;
    }
}
