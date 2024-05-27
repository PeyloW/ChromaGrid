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
    INTRO, BACKGROUND, TILES, TILES_B, TILES_C, EMPTY_TILE, ORBS, CURSOR, BUTTON, SELECTION, SHIMMER,
    FONT, MONO_FONT, SMALL_FONT, SMALL_MONO_FONT, DISK,
    DROP_ORB, TAKE_ORB, FUSE_ORB, NO_DROP_ORB, BREAK_TILE, FUSE_BREAK_TILE,
    MUSIC,
    LEVELS, LEVEL_RESULTS, USER_LEVELS,
    MENU_SCROLL
} cgassets_e;

class levels_c : public asset_c, public vector_c<level_recipe_t*, 45> {
public:
    levels_c();
    virtual size_t memory_cost() const;
};

class level_results_c : public asset_c, public vector_c<level_result_t, 45> {
public:
    level_results_c(int level_count);
    virtual size_t memory_cost() const;
    bool save() const;
};

class user_levels_c : public asset_c, public vector_c<level_recipe_t*, 10> {
public:
    user_levels_c();
    virtual size_t memory_cost() const;
    bool save() const;
};

class scroll_text_c : public asset_c {
public:
    scroll_text_c(const char *path);
    virtual size_t memory_cost() const;
    const char *text() const { return _text.get(); };
private:
    unique_ptr_c<const char> _text;
};

class cgasset_manager : public asset_manager_c {
public:
    cgasset_manager();
    virtual ~cgasset_manager() {}

    static cgasset_manager &shared() { return (cgasset_manager &)asset_manager_c::shared(); }
    
    levels_c &levels() const { return (levels_c&)(asset(LEVELS)); }
    level_results_c &level_results() const { return (level_results_c&)(asset(LEVEL_RESULTS)); }
    user_levels_c &user_levels() const { return (user_levels_c&)(asset(USER_LEVELS)); }
    scroll_text_c &menu_scroll() const { return (scroll_text_c&)(asset(MENU_SCROLL)); }
protected:
    virtual asset_c *create_asset(int id, const asset_def_s &def) const;
};

#endif /* resources_hpp */
