//
//  main.c
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-03.
//

#include "game.hpp"
#include "machine.hpp"

extern "C" {
#include <ext.h>
}

#ifdef __M68000__

int main(int argc, const char * argv[]) {
    auto &m = machine_c::shared();
    printf("Type %d.\n\r", m.type());
    printf("Max %dKb.\n\r", (uint16_t)(m.max_memory() / 1024));
    printf("User %dKb.\n\r", (uint16_t)(m.user_memory() / 1024));

    asset_manager_c::set_shared(new cgassets_c());
    scene_manager_c manager(size_s(320, 208));
    auto intro_scene = new cgmenu_scene_c(manager);
    auto overlay_scene = new cgoverlay_scene_c(manager);
    manager.run(intro_scene, overlay_scene, transition_c::create(canvas_c::noise));
}

#else
#error "For target machine only"
#endif
