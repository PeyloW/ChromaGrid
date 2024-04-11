//
//  main.c
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-03.
//

#include "game.hpp"
#include "system.hpp"

extern "C" {
#include <ext.h>
}

#ifdef __M68000__

int main(int argc, const char * argv[]) {
    scene_manager_c manager;
    auto intro_scene = new cgintro_scene_c(manager);
    auto overlay_scene = new cgoverlay_scene_c(manager);
    manager.run(intro_scene, overlay_scene, transition_c::create(canvas_c::noise));
}

#else
#error "For target machine only"
#endif
