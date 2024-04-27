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
#include "asset.hpp"

typedef enum __packed {
    INTRO, BACKGROUND, TILES, EMPTY_TILE, ORBS, CURSOR, BUTTON, SELECTION, SHIMMER,
    FONT, MONO_FONT, SMALL_FONT, SMALL_MONO_FONT,
    DROP_ORB, TAKE_ORB, FUSE_ORB, NO_DROP_ORB, BREAK_TILE, FUSE_BREAK_TILE,
    MUSIC
} cgassets_e;

class cgasset_manager : public asset_manager_c {
public:
    cgasset_manager();
    virtual ~cgasset_manager() {}

    static cgasset_manager &shared() { return (cgasset_manager &)asset_manager_c::shared(); }
    
    typedef vector_c<level_recipe_t*, 45> levels_c;
    typedef vector_c<level_result_t, 45> level_results_c;
    typedef vector_c<level_recipe_t*, 10> user_levels_c;

    levels_c &levels() const;
    level_results_c &level_results() const;
    user_levels_c &user_levels() const;

    bool save_user_levels() const;
    bool save_level_results() const;
protected:
    virtual asset_c *create_asset(int id, const asset_def_s &def) const;
private:
    void load_levels() const;
    void load_level_results() const;
    void load_user_levels() const;
    levels_c _levels;
    level_results_c _level_results;
    user_levels_c _user_levels;
};

#endif /* resources_hpp */
