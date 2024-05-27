//
//  game.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-03.
//

#include "game.hpp"
#include "machine.hpp"
#include "resources.hpp"
#include "blitter.hpp"
#include "audio_mixer.hpp"

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
            assets.preload(2, &update_preload);
            state = nullptr;
            printf("Used memory %ldKb.\n\r", assets.memory_cost() / 1024);
            manager.set_overlay_scene(new cgoverlay_scene_c(manager));
            _menu_buttons.buttons[0].text = "CONTINUE";
            _menu_buttons.draw_all(clear_screen.canvas());
            break;
        }
        default: {
            auto &mouse = mouse_c::shared();
            int button = _menu_buttons.update_buttons(canvas, mouse.postion(), mouse.state(mouse_c::left));
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
