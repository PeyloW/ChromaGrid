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
    cgmanager_c manager;
    auto root_scene = new cgroot_scene_c(manager);
    manager.run(root_scene);
}

#else
#error "For target machine only"
#endif
