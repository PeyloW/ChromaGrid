//
//  graphics.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-04.
//

#include "palette.hpp"
#include "audio.hpp"
#include "system.hpp"

using namespace toybox;

extern "C" {
#ifndef __M68000__
    const sound_c *g_active_sound = nullptr;
#endif
    const palette_c *g_active_palette = nullptr;
    const image_c *g_active_image = nullptr;
}

color_c color_c::mix(color_c other, int shade) const {
    assert(shade >= MIX_FULLY_THIS && shade <= MIX_FULLY_OTHER);
    int r = from_ste(color, 8) * (MIX_FULLY_OTHER - shade) + from_ste(other.color, 8) * shade;
    int g = from_ste(color, 4) * (MIX_FULLY_OTHER - shade) + from_ste(other.color, 4) * shade;
    int b = from_ste(color, 0) * (MIX_FULLY_OTHER - shade) + from_ste(other.color, 0) * shade;
    color_c mixed(r / MIX_FULLY_OTHER, g / MIX_FULLY_OTHER, b / MIX_FULLY_OTHER);
    return mixed;
}

void palette_c::set_active() const {
#ifdef __M68000__
#   if TOYBOX_TARGET_ATARI
    memcpy(reinterpret_cast<uint16_t*>(0xffff8240), colors, sizeof(colors));
#   else
#       error "Unsupported target"
#   endif
#endif
    g_active_palette = this;
}
