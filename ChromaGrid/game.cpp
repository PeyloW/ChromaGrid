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
