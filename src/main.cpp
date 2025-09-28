//
//  main.c
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-03.
//

#include "game.hpp"
#include "machine.hpp"

int main(int argc, const char * argv[]) {
    return  machine_c::with_machine(argc, argv, [] (machine_c &m) {
        switch (m.type()) {
            case machine_c::st:
                printf("Type ST.\n\r", m.type());
                break;
            case machine_c::ste:
                printf("Type STe.\n\r", m.type());
                break;
            case machine_c::falcon:
                printf("Type Falcon.\n\r", m.type());
                break;
            default:
                break;
        }
        printf("Max %dKb.\n\r", (uint16_t)(m.max_memory() / 1024));
        printf("User %dKb.\n\r", (uint16_t)(m.user_memory() / 1024));
#ifndef TOYBOX_HOST
        printf("Avail %dKb.\n\n", (uint16_t)(Malloc(-1) / 1024));
#endif
        if ((m.user_memory() / 1024) < 512) {
            m.free_system_memory();
        }
        
        asset_manager_c::set_shared(new cgasset_manager());
        scene_manager_c manager(size_s(320, 208));
        auto intro_scene = new cgintro_scene_c(manager);
        auto color = machine_c::shared().active_palette()->colors[0];
        auto transition = transition_c::create(color);
        manager.run(intro_scene, nullptr, transition);
        return 0;
    });
}
