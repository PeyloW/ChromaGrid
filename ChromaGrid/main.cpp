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
//    int16_t res = Dsetpath("C:\\CGRID\\");
//    hard_assert(res >= 0);

    return exec_super(&game_main);
}

#else
#error "For target mchine only"
#endif
