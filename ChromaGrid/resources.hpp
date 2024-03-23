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
#include "vector.hpp"

class cgresources_c : private nocopy_c {
public:
    image_c background;
    image_c tiles;
    image_c orbs;
    image_c cursor;
    image_c button;
    image_c selection;
private:
    image_c _font_image;
    image_c _small_font_image;
public:
    font_c font;
    font_c mono_font;
    font_c small_font;
    font_c small_mono_font;
    sount_c drop_orb;
    sount_c take_orb;
    sount_c fuse_orb;
    sount_c no_drop_orb;
    sount_c break_tile;
    sount_c fuse_break_tile;
    music_c music;
    
    vector_c<level_recipe_t*, 45> levels;
    vector_c<level_result_t, 45> level_results;
    vector_c<level_recipe_t*, 10> user_levels;

    bool save_user_levels() const;
    bool save_level_results() const;
    
    static const cgresources_c& shared();
private:
    void load_levels();
    bool load_level_results();
    bool load_user_levels();
    cgresources_c();
};

#endif /* resources_hpp */
