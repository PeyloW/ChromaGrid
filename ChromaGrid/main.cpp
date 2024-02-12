//
//  main.c
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-03.
//

#include "game.hpp"
#include "system.hpp"

#ifdef __M68000__

int main(int argc, const char * argv[]) {
    return exec_super(&game_main);
}

#else
#error "For target mchine only"
#endif
