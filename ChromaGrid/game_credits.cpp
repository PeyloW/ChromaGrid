//
//  game_credits.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-03-03.
//

#include "game.hpp"

cgcredits_scene_c::cgcredits_scene_c(scene_manager_c &manager, page_e page) :
    cggame_scene_c(manager),
    _page(page),
    _menu_buttons(MAIN_MENU_BUTTONS_ORIGIN, MAIN_MENU_BUTTONS_SIZE, MAIN_MENU_BUTTONS_SPACING)
{
    const char *button_titles[] = { "Back", "Greetings", "Dedications", "Recognitions", "Credits", nullptr };
    for (auto title = &button_titles[0]; *title; title++) {
        _menu_buttons.add_button(*title);
    }
    _menu_buttons.buttons[4 - (int)page].state = cgbutton_t::disabled;
}

static void draw_credits(font_c &font, font_c &small_font, canvas_c &screen) {
    screen.draw(font, "Credits", point_s(96, 16));

    struct { const char *credit; const char *person; } credits[] = {
        {"Code:", "Fredrik 'PeyloW' Olsson"},
        {"Graphics:", "Herve 'Exocet' Piton"},
        {"Music:", "Joakim 'AiO' Ekblad"},
        {"Fonts:", "Damien Guard"},
        {"Concept:", "Peter 'Eagle' Nyman"},
        {"Cover art:", "Silvana Massa"},
        {nullptr}
    };
    
    point_s atc(16, 40 +  0);
    point_s atp(40, 40 + 10);
    for (auto credit = &credits[0]; credit->credit; credit++) {
        screen.draw(font, credit->credit, atc, canvas_c::align_left);
        atc.y += 26;
        screen.draw(font, credit->person, atp, canvas_c::align_left);
        atp.y += 26;
    }
}

// , ,

static void draw_recognitions(font_c &font, font_c &small_font, canvas_c &screen) {
    screen.draw(font, "Recognitions", point_s(96, 16));
    const char *texts[] = {
        "This game uses royalty free sound effects from ZapSplat.\n(https://www.zapsplat.com/)",
        "This game uses libcmini by Thorsten Otto, Oliver and Markus, for the superiour speed and size.\n(https://github.com/freemint/libcmini)",
        "Blitter and audio setup code inspired by GODLib by Leon 'Mr. Pink' O'Reilly.\n(https://github.com/ReservoirGods/GODLIB).",
        "Original game idea conceived by Peter 'Eagle' Nyman of Friendchip, now realized 32 years later.",
        nullptr
    };
    rect_s rect(16, 40, 160, 48);
    for (auto text = &texts[0]; *text; text++) {
        auto size = screen.draw(small_font, *text, rect, 2);
        rect.origin.y += size.height + 8;
    }
}

static void draw_dedications(font_c &font, font_c &small_font, canvas_c &screen) {
    screen.draw(font, "Dedications", point_s(96, 16));
    const char *texts[] = {
        "Released at Sommarhack 2024.\n""Special thanks to Anders 'evl' Erikson and the friends who stayed Atari.",
        "Fredrik would like to thank Mia, Mondi, and Sturdy who endured the develpment.\nSpecial dedication to Marianne and Jan-Erik Peylow who gave me my nick and my life.",
        "Joakim would like to thank... people",
        nullptr
    };
    rect_s rect(16, 40, 160, 48);
    for (auto text = &texts[0]; *text; text++) {
        auto size = screen.draw(small_font, *text, rect, 2);
        rect.origin.y += size.height + 8;
    }
}

static void draw_greetings(font_c &font, font_c &small_font, canvas_c &screen) {
    screen.draw(font, "Greetings", point_s(96, 16));
    const char *texts[] = {
        "To the people we shared with; Baggio, Deez, Eagle, Evil, Fritz, Jag, OB, Tam.",
        "To the groups that keep the Atari scene alive; Aggression, Cerebral Vortex, Daeghnao, Dekadence, Desire, DHS, Effect, Ephidrena, Escape, Evolution, Extream, Ghost, IMPonance, Istari, KUA, Mystic Bytes, Nature, New Beat, Newline, Oxygene, PHF, Reservoir Gods, Sector One, Sector One, SMFX, Sultans Of Sodom.",
        "To the legends that inspired us; 2-Life Crew, ACF, Aggression, Avena, Delta Force, Equinox, Inner Circle, Omega, Overlanders, Reservoir Gods, TCB, TEX, TLB, ULM.",
        "You.",
        nullptr
    };
    rect_s rect(16, 40, 160, 48);
    for (auto text = &texts[0]; *text; text++) {
        auto size = screen.draw(small_font, *text, rect, 2);
        rect.origin.y += size.height + 8;
    }
}

void cgcredits_scene_c::will_appear(screen_c &clear_screen, bool obsured) {
    auto &canvas = clear_screen.canvas();
    canvas.draw_aligned(background, point_s());
    _menu_buttons.draw_all(canvas);
    
    switch (_page) {
        case credits:
            draw_credits(font, small_font, canvas);
            break;
        case recognitions:
            draw_recognitions(font, small_font, canvas);
            break;
        case dedications:
            draw_dedications(font, small_font, canvas);
            break;
        case greetings:
            draw_greetings(font, small_font, canvas);
            break;
    }
}


void cgcredits_scene_c::update_clear(screen_c &clear_screen, int ticks) {
    auto &canvas = clear_screen.canvas();
    int button = update_button_group(canvas, _menu_buttons);
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
