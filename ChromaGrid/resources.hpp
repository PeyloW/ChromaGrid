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
    cgsount_c drop_orb;
    cgsount_c take_orb;
    cgsount_c fuse_orb;
    cgsount_c no_drop_orb;
    cgsount_c break_tile;
    cgmusic_c music;
    
    cgvector_c<level_recipe_t*, 100> levels;
    cgvector_c<level_result_t, 100> level_results;
    cgvector_c<level_recipe_t*, 10> user_levels;

    bool save_user_levels() const;
    bool save_level_results() const;
    
    static const cgresources_c& shared();
private:
    bool load_levels() const;
    bool load_level_results();
    bool load_user_levels() const;
    cgresources_c();
};

#endif /* resources_hpp */
