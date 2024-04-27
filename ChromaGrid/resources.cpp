//
//  resources.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-03-01.
//

#include "resources.hpp"
#include "game.hpp"
#include "iffstream.hpp"

static void remap_to(color_e col, canvas_c::remap_table_c &table, int masked_idx = image_c::MASKED_CIDX) {
    switch (col) {
        case color_e::gold:
            table[0] = 1;
            table[2] = 1;
            table[7] = 9;
            table[10] = 14;
            break;
        case color_e::silver:
            table[0] = 2;
            table[2] = 2;
            table[7] = 8;
            table[10] = 13;
            break;
        default:
            break;
    }
    table[masked_idx] = image_c::MASKED_CIDX;
}

cgasset_manager::cgasset_manager() : 
    asset_manager_c()
{
    /*
    FONT, MONO_FONT, SMALL_FONT, SMALL_MONO_FONT,
    DROP_ORB, TAKE_ORB, FUSE_ORB, NO_DROP_ORB, BREAK_TILE, FUSE_BREAK_TILE,
    MUSIC,
    LEVELS, LEVEL_RESULTS, USER_LEVELS,
     */

    add_asset_def(INTRO, asset_def_s(asset_c::image, 1, "backgrnd.iff"));
    add_asset_def(BACKGROUND, asset_def_s(asset_c::image, 2, "backgrnd.iff"));
    add_asset_def(TILES, asset_def_s(asset_c::tileset, 2, "tiles.iff"));
    add_asset_def(EMPTY_TILE, asset_def_s(asset_c::tileset, 2, "emptyt.iff"));
    add_asset_def(ORBS, asset_def_s(asset_c::tileset, 2, "orbs.iff", [](const asset_manager_c &manager, const char *path) -> asset_c* {
        return new tileset_c(new image_c(path), size_s(16, 10));
    }));
    add_asset_def(CURSOR, asset_def_s(asset_c::image, 2, "cursor.iff"));
    add_asset_def(BUTTON, asset_def_s(asset_c::image, 2, "button.iff"));
    add_asset_def(SELECTION, asset_def_s(asset_c::image, 2, "select.iff"));
    add_asset_def(SHIMMER, asset_def_s(asset_c::tileset, 2, "shimmer.iff"));
 
    add_asset_def(FONT, asset_def_s(asset_c::font, 2, "font.iff", [](const asset_manager_c &manager, const char *path) -> asset_c* {
        return new font_c(new image_c(path), size_s(8, 8), 4, 2, 4);
    }));
    add_asset_def(MONO_FONT, asset_def_s(asset_c::font, 2, nullptr, [](const asset_manager_c &manager, const char *path) -> asset_c* {
        return new font_c(manager.font(FONT).image(), size_s(8, 8), 4, 2, 4);
    }));
    add_asset_def(SMALL_FONT, asset_def_s(asset_c::font, 2, "font6.iff", [](const asset_manager_c &manager, const char *path) -> asset_c* {
        return new font_c(new image_c(path), size_s(6, 6), 3, 0, 6);
    }));
    add_asset_def(SMALL_MONO_FONT, asset_def_s(asset_c::font, 2, nullptr, [](const asset_manager_c &manager, const char *path) -> asset_c* {
        return new font_c(manager.font(SMALL_FONT).image(), size_s(6, 6));
    }));

    add_asset_def(DROP_ORB, asset_def_s(asset_c::sound, 2, "drop.aif"));
    add_asset_def(TAKE_ORB, asset_def_s(asset_c::sound, 2, "take.aif"));
    add_asset_def(FUSE_ORB, asset_def_s(asset_c::sound, 2, "fuse.aif"));
    add_asset_def(NO_DROP_ORB, asset_def_s(asset_c::sound, 2, "tock.aif"));
    add_asset_def(BREAK_TILE, asset_def_s(asset_c::sound, 2, "break.aif"));
    add_asset_def(FUSE_BREAK_TILE, asset_def_s(asset_c::sound, 2, "fusebrk.aif"));

    add_asset_def(MUSIC, asset_def_s(asset_c::music, 2, "music.snd"));
}

/*
cgassets_c::cgassets_c() :

 drop_orb(data_path("drop.aif", "Load audio")),
    take_orb(data_path("take.aif")),
    fuse_orb(data_path("fuse.aif")),
    no_drop_orb(data_path("tock.aif")),
    break_tile(data_path("break.aif")),
    fuse_break_tile(data_path("fusebrk.aif")),
    music(data_path("music.snd", "Load music"))
{
    for (int x = 1; x < 3; x++) {
        canvas_c tiles_cnv(*tiles.image());
        printf("Initialize tiles %d.\n\r", x);
        rect_s rect(x * 48, 0, 48, 80);
        tiles_cnv.draw(*tiles.image(), rect_s(0, 0, 48, 80), rect.origin);
        canvas_c::remap_table_c table;
        if (x == 1) {
            remap_to(color_e::gold, table);
        } else {
            remap_to(color_e::silver, table);
        }
        tiles_cnv.remap_colors(table, rect);
    }
    
    printf("Loading user levels.\n\r");
    uint8_t *recipes = (uint8_t *)calloc(10, level_recipe_t::MAX_SIZE);
    for (int i = 0; i < 10; i++) {
        user_levels.push_back((level_recipe_t *)(recipes + i * level_recipe_t::MAX_SIZE));
    }
    load_user_levels();
    
    printf("Loading levels.\n\r");
    load_levels();
    load_level_results();
}
*/

asset_c *cgasset_manager::create_asset(int id, const asset_def_s &def) const {
    auto asset = asset_manager_c::create_asset(id, def);
    if (id == TILES) {
        tileset_c &tiles = *(tileset_c*)asset;
        for (int x = 1; x < 3; x++) {
            canvas_c tiles_cnv(*tiles.image());
            rect_s rect(x * 48, 0, 48, 80);
            tiles_cnv.draw(*tiles.image(), rect_s(0, 0, 48, 80), rect.origin);
            canvas_c::remap_table_c table;
            if (x == 1) {
                remap_to(color_e::gold, table);
            } else {
                remap_to(color_e::silver, table);
            }
            tiles_cnv.remap_colors(table, rect);
        }
    }
    return asset;
}

cgasset_manager::levels_c &cgasset_manager::levels() const {
    if (_levels.size() == 0) {
        load_levels();
    }
    return (levels_c&)_levels;
}

cgasset_manager::level_results_c &cgasset_manager::level_results() const {
    if (_level_results.size() == 0) {
        load_level_results();
    }
    return (level_results_c&)_level_results;
}

cgasset_manager::user_levels_c &cgasset_manager::user_levels() const {
    if (_user_levels.size() == 0) {
        load_user_levels();
    }
    return (user_levels_c&)_user_levels;
}

void cgasset_manager::load_levels() const {
    auto &levels = (levels_c&)_levels;
    int i = 1;
    char buf[14];
    strstream_c str(buf, 14);
    while (levels.size() < 45) {
        str.reset();
        str << "levels" << i++ << ".dat" << ends;
        iffstream_c iff(data_path(str.str()).get(), fstream_c::input);
        if (!iff.good()) {
            break;
        }
        iff.set_assert_on_error(true);
        iff_group_s list;
        iff.first(IFF_LIST, IFF_CGLV, list);
        uint8_t *data = (uint8_t *)calloc(1, list.size);
        iff_group_s level_group;
        while (levels.size() < 45 && iff.next(list, IFF_FORM, level_group)) {
            level_recipe_t *recipe = (level_recipe_t *)(data + iff.tell());
            recipe->load(iff, level_group);
            levels.emplace_back(recipe);
        }
    }
    assert(levels.size() > 0);
}

void cgasset_manager::load_user_levels() const {
    auto &user_levels = (user_levels_c&)_user_levels;
    uint8_t *recipes = (uint8_t *)calloc(10, level_recipe_t::MAX_SIZE);
    for (int i = 0; i < 10; i++) {
        user_levels.push_back((level_recipe_t *)(recipes + i * level_recipe_t::MAX_SIZE));
    }
    iffstream_c iff(user_path("levels.dat").get(), fstream_c::input);
    if (iff.good()) {
        iff.set_assert_on_error(true);
        iff_group_s list;
        iff.first(IFF_LIST, IFF_CGLV, list);
        int index = 0;
        iff_group_s level_group;
        while (iff.next(list, IFF_FORM, level_group)) {
            auto recipe = user_levels[index++];
            recipe->load(iff, level_group);
        }
    }
}

bool cgasset_manager::save_user_levels() const {
    iffstream_c iff(user_path("levels.dat").get(), fstream_c::input | fstream_c::output);
    if (!iff.good()) {
        return false;
    }
    auto &levels = user_levels();
    iff.set_assert_on_error(true);
    iff_group_s list;
    iff.begin(list, IFF_LIST);
    iff.write(&IFF_CGLV_ID);
    for (int index = 0; index < levels.size(); index++) {
        auto &recipe = levels[index];
        if (!recipe->empty()) {
            recipe->save(iff);
        }
    }
    iff.end(list);
    return true;
}

void cgasset_manager::load_level_results() const {
    auto &level_results = (level_results_c&)_level_results;
    bool success = false;
    iffstream_c iff(user_path("scores.dat").get());
    if (!iff.good()) goto done;
    iff_group_s list;
    if (iff.first(IFF_LIST, IFF_CGLR, list)) {
        iff_chunk_s level_chunk;
        while (iff.next(list, IFF_CGLR, level_chunk)) {
            level_results.emplace_back();
            auto &level_result = level_results.back();
            if (!level_result.load(iff, level_chunk)) {
                goto done;
            }
        }
    }
    success = level_results.size() <= levels().size();
done:
    if (!success) {
        level_results.clear();
    }
    while (level_results.size() < levels().size()) {
        level_results.emplace_back();
    }
}

bool cgasset_manager::save_level_results() const {
    iffstream_c iff(user_path("scores.dat").get(), fstream_c::input | fstream_c::output);
    if (!iff.good()) {
        return false;
    }
    auto &results = level_results();
    iff_group_s list;
    if (iff.begin(list, IFF_LIST)) {
        iff.write(&IFF_CGLR_ID);
        for (const auto &result : results) {
            result.save(iff);
        }
        return iff.end(list);
    }
    return false;
}
