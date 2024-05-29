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
            table[2] = 12;
            table[3] = 13;
            table[4] = 14;
            break;
        case color_e::silver:
            table[2] = 11;
            table[3] = 8;
            table[4] = 9;
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
    
    add_asset_def(INTRO, asset_def_s(asset_c::image, 1, "intro.iff"));
    add_asset_def(BACKGROUND, asset_def_s(asset_c::image, 2, "backgrnd.iff"));
    add_asset_def(TILES, asset_def_s(asset_c::tileset, 2, "tiles.iff"));
    add_asset_def(TILES_B, asset_def_s(asset_c::tileset, 2, "tiles2.iff"));
    add_asset_def(TILES_C, asset_def_s(asset_c::tileset, 2, "tiles3.iff"));
    add_asset_def(EMPTY_TILE, asset_def_s(asset_c::tileset, 2, "emptyt.iff"));
    add_asset_def(ORBS, asset_def_s(asset_c::tileset, 2, "orbs.iff", [](const asset_manager_c &manager, const char *path) -> asset_c* {
        return new tileset_c(new image_c(path), size_s(16, 10));
    }));
    add_asset_def(CURSOR, asset_def_s(asset_c::image, 2, "cursor.iff"));
    add_asset_def(BUTTON, asset_def_s(asset_c::image, 2, "button.iff"));
    add_asset_def(SELECTION, asset_def_s(asset_c::image, 2, "select.iff"));
    add_asset_def(SHIMMER, asset_def_s(asset_c::tileset, 2, "shimmer.iff"));
    
    add_asset_def(FONT, asset_def_s(asset_c::font, 2, "font.iff", [](const asset_manager_c &manager, const char *path) -> asset_c* {
        auto image = new image_c(path);
        return new font_c(image, size_s(8, 8), 4, 2, 4);
    }));
    add_asset_def(MONO_FONT, asset_def_s(asset_c::font, 2, nullptr, [](const asset_manager_c &manager, const char *path) -> asset_c* {
        return new font_c(manager.font(FONT).image(), size_s(8, 8));
    }));
    add_asset_def(SMALL_FONT, asset_def_s(asset_c::font, 2, "font6.iff", [](const asset_manager_c &manager, const char *path) -> asset_c* {
        auto image = new image_c(path);
        return new font_c(image, size_s(6, 6), 3, 0, 6);
    }));
    add_asset_def(SMALL_MONO_FONT, asset_def_s(asset_c::font, 2, nullptr, [](const asset_manager_c &manager, const char *path) -> asset_c* {
        return new font_c(manager.font(SMALL_FONT).image(), size_s(6, 6));
    }));
    add_asset_def(DISK, asset_def_s(asset_c::image, 2, "disk.iff"));
    
    add_asset_def(DROP_ORB, asset_def_s(asset_c::sound, 2, "drop.aif"));
    add_asset_def(TAKE_ORB, asset_def_s(asset_c::sound, 2, "take.aif"));
    add_asset_def(FUSE_ORB, asset_def_s(asset_c::sound, 2, "fuse.aif"));
    add_asset_def(NO_DROP_ORB, asset_def_s(asset_c::sound, 2, "tock.aif"));
    add_asset_def(BREAK_TILE, asset_def_s(asset_c::sound, 2, "break.aif"));
    add_asset_def(FUSE_BREAK_TILE, asset_def_s(asset_c::sound, 2, "fusebrk.aif"));
    
    add_asset_def(MUSIC, asset_def_s(asset_c::music, 2, "music.snd"));
    
    add_asset_def(LEVELS, asset_def_s(asset_c::custom, 2, nullptr, [](const asset_manager_c &manager, const char *path) -> asset_c* {
        return new levels_c();
    }));
    add_asset_def(LEVEL_RESULTS, asset_def_s(asset_c::custom, 2, nullptr, [](const asset_manager_c &manager, const char *path) -> asset_c* {
        return new level_results_c(((cgasset_manager&)manager).levels().size());
    }));
    add_asset_def(USER_LEVELS, asset_def_s(asset_c::custom, 2, nullptr, [](const asset_manager_c &manager, const char *path) -> asset_c* {
        return new user_levels_c();
    }));
    
    add_asset_def(MENU_SCROLL, asset_def_s(asset_c::custom, 2, "menu.txt", [](const asset_manager_c &manager, const char *path) -> asset_c* {
        return new scroll_text_c(path);
    }));
}

asset_c *cgasset_manager::create_asset(int id, const asset_def_s &def) const {
    auto asset = asset_manager_c::create_asset(id, def);
    if (id >= TILES && id <= TILES_C) {
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
        //tiles.image()->save("/tmp/tiles.iff", compression_type_e::compression_type_none, false);
    }
    return asset;
}

levels_c::levels_c() {
    int i = 1;
    char buf[14];
    strstream_c str(buf, 14);
    while (size() < 45) {
        str.reset();
        str << "levels" << i++ << ".dat" << ends;
        iffstream_c iff(asset_manager_c::shared().data_path(str.str()).get(), fstream_c::input);
        if (!iff.good()) {
            break;
        }
        iff.set_assert_on_error(true);
        iff_group_s list;
        iff.first(IFF_LIST, IFF_CGLV, list);
        uint8_t *data = (uint8_t *)_calloc(1, list.size);
        iff_group_s level_group;
        while (size() < 45 && iff.next(list, IFF_FORM, level_group)) {
            level_recipe_t *recipe = (level_recipe_t *)(data + iff.tell());
            recipe->load(iff, level_group);
            emplace_back(recipe);
        }
    }
    assert(size() > 0);
}

size_t levels_c::memory_cost() const {
    size_t size = sizeof(*this);
    for (auto &recipie : *this) {
        size += sizeof(level_recipe_t);
        size += sizeof(tilestate_t) * recipie->header.width * recipie->header.height;
    }
    return size;
}

user_levels_c::user_levels_c() {
    uint8_t *recipes = (uint8_t *)_calloc(10, level_recipe_t::MAX_SIZE);
    for (int i = 0; i < 10; i++) {
        push_back((level_recipe_t *)(recipes + i * level_recipe_t::MAX_SIZE));
    }
    iffstream_c iff(asset_manager_c::shared().user_path("levels.dat").get(), fstream_c::input);
    if (iff.good()) {
        iff.set_assert_on_error(true);
        iff_group_s list;
        iff.first(IFF_LIST, IFF_CGLV, list);
        int index = 0;
        iff_group_s level_group;
        while (iff.next(list, IFF_FORM, level_group)) {
            auto recipe = (*this)[index++];
            recipe->load(iff, level_group);
        }
    }
}

bool user_levels_c::save() const {
    iffstream_c iff(asset_manager_c::shared().user_path("levels.dat").get(), fstream_c::input | fstream_c::output);
    if (!iff.good()) {
        return false;
    }
    iff.set_assert_on_error(true);
    iff_group_s list;
    iff.begin(list, IFF_LIST);
    iff.write(&IFF_CGLV_ID);
    for (int index = 0; index < size(); index++) {
        auto &recipe = (*this)[index];
        if (!recipe->empty()) {
            recipe->save(iff);
        }
    }
    iff.end(list);
    return true;
}

size_t user_levels_c::memory_cost() const {
    size_t size = sizeof(user_levels_c);
    size += 10 * level_recipe_t::MAX_SIZE;
    return size;
}

level_results_c::level_results_c(int level_count) {
    bool success = false;
    iffstream_c iff(asset_manager_c::shared().user_path("scores.dat").get());
    if (!iff.good()) goto done;
    iff_group_s list;
    if (iff.first(IFF_LIST, IFF_CGLR, list)) {
        iff_chunk_s level_chunk;
        while (iff.next(list, IFF_CGLR, level_chunk)) {
            emplace_back();
            auto &level_result = back();
            if (!level_result.load(iff, level_chunk)) {
                goto done;
            }
        }
    }
    success = size() <= level_count;
done:
    if (!success) {
        clear();
    }
    while (size() < level_count) {
        emplace_back();
    }
}

size_t level_results_c::memory_cost() const {
    return sizeof(level_results_c);
}

bool level_results_c::save() const {
    iffstream_c iff(asset_manager_c::shared().user_path("scores.dat").get(), fstream_c::input | fstream_c::output);
    if (!iff.good()) {
        return false;
    }
    iff_group_s list;
    if (iff.begin(list, IFF_LIST)) {
        iff.write(&IFF_CGLR_ID);
        for (const auto &result : *this) {
            result.save(iff);
        }
        return iff.end(list);
    }
    return false;
}


scroll_text_c::scroll_text_c(const char *path) {
    fstream_c file(path);
    file.seek(0, toybox::stream_c::end);
    auto size = file.tell();
    file.seek(0, toybox::stream_c::beg);
    char *text = (char*)malloc(size + 1);
    file.read((uint8_t *)text, size);
    text[size] = 0;
    _text.reset(text);
}

size_t scroll_text_c::memory_cost() const {
    return strlen(text());
}
