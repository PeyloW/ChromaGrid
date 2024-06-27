//
//  game.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-03.
//

#include "game.hpp"
#include "machine.hpp"
#include "resources.hpp"
#include "audio_mixer.hpp"

#ifndef __M68000__
#include <unistd.h>
#endif

#define LOADING_BUTTON_ORIGIN point_s((320-128)/2, 200-28)
#define LOADING_BUTTON_SIZE size_s(128, 14)

cgintro_scene_c::cgintro_scene_c(scene_manager_c &manager) : scene_c(manager),
    _update_count(0),
    _menu_buttons(LOADING_BUTTON_ORIGIN, LOADING_BUTTON_SIZE, 0)
{
    _menu_buttons.add_button("LOADING");
    _menu_buttons.buttons[0].state = cgbutton_t::disabled;
}

cggame_scene_c::configuration_s &cgintro_scene_c::configuration() const {
    static cggame_scene_c::configuration_s config(*cgasset_manager::shared().image(INTRO).palette());
    return config;
}

void cgintro_scene_c::will_appear(screen_c &clear_screen, bool obsured) {
    auto &image = cgasset_manager::shared().image(INTRO);
    clear_screen.canvas().draw_aligned(image, point_s());
    _menu_buttons.draw_all(clear_screen.canvas());
}

static cgintro_scene_c *state = nullptr;
void cgintro_scene_c::update_preload(int loaded, int count) {
    auto &clear_image = state->manager.screen(scene_manager_c::screen_e::clear).image();
    auto &front_canvas = state->manager.screen(scene_manager_c::screen_e::front).canvas();
    rect_s rect(LOADING_BUTTON_ORIGIN, LOADING_BUTTON_SIZE);
    rect.size.width = rect.size.width * loaded / count;
    front_canvas.draw(clear_image, rect, LOADING_BUTTON_ORIGIN);
#ifndef __M68000__
    usleep(50000);
#endif
}

#define CG_MONTH (\
  __DATE__ [2] == 'n' ? (__DATE__ [1] == 'a' ? 1 : 6) \
: __DATE__ [2] == 'b' ? 2 \
: __DATE__ [2] == 'r' ? (__DATE__ [0] == 'M' ? 3 : 4) \
: __DATE__ [2] == 'y' ? 5 \
: __DATE__ [2] == 'l' ? 7 \
: __DATE__ [2] == 'g' ? 8 \
: __DATE__ [2] == 'p' ? 9 \
: __DATE__ [2] == 't' ? 10 \
: __DATE__ [2] == 'v' ? 11 \
: 12)

#define CG_DAY ( (__DATE__[4] - '0') * 10 + __DATE__[5] - '0' )

void cgintro_scene_c::update_clear(screen_c &clear_screen, int ticks) {
    auto &canvas = clear_screen.canvas();
    auto &assets = cgasset_manager::shared();
    switch (_update_count++) {
        case 0 ... 1:
            break;
        case 2: {
            _menu_buttons.buttons[0].state = cgbutton_t::normal;
            _menu_buttons.draw_all(canvas);
            state = this;
            audio_mixer_c::shared().play(assets.music(MUSIC));
            assets.unload(1);
            const int sets = assets.support_audio() ? 4 + 2 : 2;
            assets.preload(sets, &update_preload);
            state = nullptr;
            manager.set_overlay_scene(new cgoverlay_scene_c(manager));
            _menu_buttons.buttons[0].text = "CONTINUE";
            _menu_buttons.draw_all(clear_screen.canvas());
            strstream_c version(10);
            version.width(2); version.fill('0');
            version << "v1." << CG_MONTH << '.' << CG_DAY << ends;
            clear_screen.canvas().draw(assets.font(SMALL_FONT), version.str(), point_s(318, 193), canvas_c::align_right, 9);
            break;
        }
        default: {
            auto &mouse = mouse_c::shared();
            int button = _menu_buttons.update_buttons(canvas, mouse);
            if (button == 0) {
                auto color = machine_c::shared().active_palette()->colors[0];
                auto transition = transition_c::create(color);
                manager.replace(new cgmenu_scene_c(manager), transition);
            }
            break;
        }
    }
}


cggame_scene_c::cggame_scene_c(scene_manager_c &manager) :
    scene_c(manager),
    mouse(mouse_c::shared()),
    assets(cgasset_manager::shared()),
    background(assets.image(BACKGROUND)),
    font(assets.font(FONT)),
    small_font(assets.font(SMALL_FONT))
{};

cggame_scene_c::configuration_s &cggame_scene_c::configuration() const {
    static cggame_scene_c::configuration_s config(*cgasset_manager::shared().image(BACKGROUND).palette());
    return config;
}

cggame_scene_c::configuration_s &cgoverlay_scene_c::configuration() const {
    static cggame_scene_c::configuration_s config(*cgasset_manager::shared().image(BACKGROUND).palette(), 2, false);
    return config;
}

void cgoverlay_scene_c::update_back(screen_c &back_screen, int ticks) {
    auto &canvas = back_screen.canvas();
    canvas.with_clipping(true, [this, &canvas] {
        point_s at = mouse.postion();
        at.x -= 2;
        at.y -= 2;
        canvas.draw(assets.image(CURSOR), at);
    });
}

cgerror_scene_c::cgerror_scene_c(scene_manager_c &manager, const char *title, const char *text, choice_f callback, scene_c &target) :
    cggame_scene_c(manager),
    _title(title), _text(text),
    _callback(callback), _target(target),
    _buttons(point_s(96, 146), size_s(128, 14), 0)
{
    _buttons.add_button_pair("Retry", "Cancel");
}

void cgerror_scene_c::will_appear(screen_c &clear_screen, bool obsured) {
    auto &canvas = clear_screen.canvas();
    auto &assets = cgasset_manager::shared();
    canvas.with_stencil(canvas_c::stencil(canvas_c::orderred, 48), [this, &canvas] {
        canvas.fill(7, rect_s(0, 0, 320, 200));
    });
    canvas.fill(14, rect_s(64, 32, 192, 136));
    canvas.fill(7, rect_s(65, 33, 190, 134));
    _buttons.draw_all(canvas);
    canvas.draw(assets.font(FONT), _title, point_s(160, 40));
    canvas.draw(assets.font(SMALL_FONT), _text, rect_s(80, 56, 160, 82), 2);
}

void cgerror_scene_c::update_back(screen_c &back_screen, int ticks) {
    int button = update_button_group(back_screen.canvas(), _buttons);
    if (button >= 0) {
        (_target.*_callback)((choice_e)button);
    }
}
