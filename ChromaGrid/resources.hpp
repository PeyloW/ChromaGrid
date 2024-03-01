//
//  resources.hpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-03-01.
//

#ifndef resources_hpp
#define resources_hpp

#include "graphics.hpp"
#include "audio.hpp"

class cgresources_c : private cgnocopy_c {
public:
    cgimage_c background;
    cgimage_c tiles;
    cgimage_c orbs;
    cgimage_c cursor;
    cgimage_c button;
private:
    cgimage_c _font_image;
    cgimage_c _small_font_image;
public:
    cgfont_c font;
    cgfont_c small_font;
    cgmusic_c music;

    cgimage_c::stencil_t stencils[2][cgimage_c::STENCIL_FULLY_OPAQUE + 1];
    
    static const cgresources_c& shared();
private:
    cgresources_c();
};

#endif /* resources_hpp */
