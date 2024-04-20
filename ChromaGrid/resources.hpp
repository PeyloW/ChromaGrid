//
//  resources.hpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-03-01.
//

#ifndef resources_hpp
#define resources_hpp

#include "image.hpp"
#include "audio.hpp"
#include "level.hpp"
#include "vector.hpp"

class cgresources_c : public nocopy_c {
public:
    image_c background;
    tileset_c tiles;
    tileset_c empty_tile;
    tileset_c orbs;
    image_c cursor;
    image_c button;
    image_c selection;
    tileset_c shimmer;
    font_c font;
    font_c mono_font;
    font_c small_font;
    font_c small_mono_font;
    sound_c drop_orb;
    sound_c take_orb;
    sound_c fuse_orb;
    sound_c no_drop_orb;
    sound_c break_tile;
    sound_c fuse_break_tile;
    music_c music;
    
    vector_c<level_recipe_t*, 45> levels;
    vector_c<level_result_t, 45> level_results;
    vector_c<level_recipe_t*, 10> user_levels;

    bool save_user_levels();
    bool save_level_results();
    
    static cgresources_c& shared();
private:
    void load_levels();
    bool load_level_results();
    bool load_user_levels();
    cgresources_c();
    ~cgresources_c() {};
};

#endif /* resources_hpp */
