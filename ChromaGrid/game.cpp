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

void cgintro_scene_c::will_appear(screen_c &screen, bool obsured) {
    auto &image = cgasset_manager::shared().image(INTRO);
    screen.canvas().draw_aligned(image, point_s());
    machine_c::shared().set_active_palette(image.palette().get());
}

void cgintro_scene_c::update_background(screen_c &screen, int ticks) {
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
            manager.set_overlay_scene(new cgoverlay_scene_c(manager));
        default:
            if (manager.mouse.is_pressed(mouse_c::left)) {
                manager.push(new cgmenu_scene_c(manager));
            }
            break;
    }
}


cggame_scene_c::cggame_scene_c(scene_manager_c &manager) :
    scene_c(manager),
    assets(cgasset_manager::shared()),
    background(assets.image(BACKGROUND)),
    font(assets.font(FONT)),
    small_font(assets.font(SMALL_FONT))
{};

void cgoverlay_scene_c::update_foreground(screen_c &screen, int ticks) {
    auto &canvas = screen.canvas();
    canvas.with_clipping(true, [this, &canvas] {
        point_s at = manager.mouse.postion();
        at.x -= 2;
        at.y -= 2;
        canvas.draw(assets.image(CURSOR), at);
    });
}
