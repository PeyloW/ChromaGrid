//
//  graphics.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-04.
//

#include "palette.hpp"
#include "audio.hpp"
#include "system_helpers.hpp"

using namespace toybox;

#ifndef __M68000__
extern "C" {
    const sound_c *g_active_sound = nullptr;
}
#endif

color_c color_c::mix(color_c other, int shade) const {
    assert(shade >= MIX_FULLY_THIS && shade <= MIX_FULLY_OTHER);
    int r = from_ste(color, 8) * (MIX_FULLY_OTHER - shade) + from_ste(other.color, 8) * shade;
    int g = from_ste(color, 4) * (MIX_FULLY_OTHER - shade) + from_ste(other.color, 4) * shade;
    int b = from_ste(color, 0) * (MIX_FULLY_OTHER - shade) + from_ste(other.color, 0) * shade;
    color_c mixed(r / MIX_FULLY_OTHER, g / MIX_FULLY_OTHER, b / MIX_FULLY_OTHER);
    return mixed;
}
