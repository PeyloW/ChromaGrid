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

#ifdef __M68000__
extern "C" {
#include <ext.h>
}
#endif

cgintro_scene_c::cgintro_scene_c(scene_manager_c &manager) : scene_c(manager), _update_count(0) {
}

void cgintro_scene_c::will_appear(screen_c &clear_screen, bool obsured) {
    auto &image = cgasset_manager::shared().image(INTRO);
    clear_screen.canvas().draw_aligned(image, point_s());
    machine_c::shared().set_active_palette(image.palette().get());
}

void cgintro_scene_c::update_clear(screen_c &clear_screen, int ticks) {
    auto &assets = cgasset_manager::shared();
    switch (_update_count++) {
        case 0 ... 1:
            break;
        case 2:
            audio_mixer_c::shared().play(assets.music(MUSIC));
            assets.unload(1);
            assets.preload(2);
            assets.levels();
            assets.level_results();
            assets.user_levels();
            printf("Used memory %ldKb.\n\r", assets.memory_cost() / 1024);
            manager.set_overlay_scene(new cgoverlay_scene_c(manager));
        default:
            if (mouse_c::shared().is_pressed(mouse_c::left)) {
                auto color = machine_c::shared().active_palette()->colors[0];
                auto transition = transition_c::create(color);
                manager.replace(new cgmenu_scene_c(manager), transition);
            }
            break;
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

void cgoverlay_scene_c::update_back(screen_c &back_screen, int ticks) {
    auto &canvas = back_screen.canvas();
    canvas.with_clipping(true, [this, &canvas] {
        point_s at = mouse.postion();
        at.x -= 2;
        at.y -= 2;
        canvas.draw(assets.image(CURSOR), at);
    });
}
