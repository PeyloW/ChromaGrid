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
#include "level.hpp"

class cgresources_c : private cgnocopy_c {
public:
    cgimage_c background;
    cgimage_c tiles;
    cgimage_c orbs;
    cgimage_c cursor;
    cgimage_c button;
    cgimage_c selection;
private:
    cgimage_c _font_image;
    cgimage_c _small_font_image;
public:
    cgfont_c font;
    cgfont_c mono_font;
    cgfont_c small_font;
    cgmusic_c music;
    
    cgvector_c<level_t::recipe_t*, 100> levels;
    
    static const cgresources_c& shared();
private:
    cgresources_c();
};

#endif /* resources_hpp */
