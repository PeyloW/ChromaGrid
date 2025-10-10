//
//  resources.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-03-01.
//

#include "machine.hpp"
#include "resources.hpp"
#include "game.hpp"
#include "iffstream.hpp"

extern "C" __neverinline void read_cheats(bool &max_time, bool &max_orbs) {
    uint16_t cheat = (uint16_t)machine_c::shared().get_cookie(0x5F434743, -1); // '_CGC'
    if (cheat != 0xffff) {
        switch (cheat) {
            case 0x0001:
                max_time = true;
                break;
            case 0x0100:
                max_orbs = true;
                break;
            case 0x0101:
                max_time = true;
                max_orbs = true;
                break;
            default:
                break;
        }
    }
#ifdef __M68000__
    uint16_t val = *(uint16_t*)0x382;
    if (~val == *(uint16_t*)0x380) {
        switch (val) {
            case 0x0001:
                max_time = true;
                break;
            case 0x0100:
                max_orbs = true;
                break;
            case 0x0101:
                max_time = true;
                max_orbs = true;
                break;
            default:
                break;
        }
    }
#endif
}

cgasset_manager::cgasset_manager() :
    asset_manager_c(), _max_time(false), _max_orbs(false)
{
    /*
     FONT, MONO_FONT, SMALL_FONT, SMALL_MONO_FONT,
     DROP_ORB, TAKE_ORB, FUSE_ORB, NO_DROP_ORB, BREAK_TILE, FUSE_BREAK_TILE,
     MUSIC,
     LEVELS, LEVEL_RESULTS, USER_LEVELS,
     */
    read_cheats(*(bool*)&_max_time, *(bool*)&_max_orbs);

    constexpr pair_c<int,asset_def_s> asset_defs[] = {
        { INTRO, asset_def_s(asset_c::type_e::image, 1, "intro.iff") },
        { BACKGROUND, asset_def_s(asset_c::type_e::image, 2, "backgrnd.iff") },
        { TILES_A, asset_def_s(asset_c::type_e::tileset, 2, "tiles1.iff") },
        { TILES_B, asset_def_s(asset_c::type_e::tileset, 2, "tiles2.iff") },
        { TILES_C, asset_def_s(asset_c::type_e::tileset, 2, "tiles3.iff") },
        { EMPTY_TILE, asset_def_s(asset_c::type_e::tileset, 2, "emptyt.iff") },
        { ORBS, asset_def_s(asset_c::type_e::tileset, 2, "orbs.iff", [](const asset_manager_c &manager, const char *path) -> asset_c* {
            return new tileset_c(new image_c(path), size_s(16, 10));
        })},
        { CURSOR, asset_def_s(asset_c::type_e::image, 2, "cursor.iff") },
        { BUTTON, asset_def_s(asset_c::type_e::image, 2, "button.iff") },
        { SELECTION, asset_def_s(asset_c::type_e::image, 2, "select.iff") },
        { SHIMMER, asset_def_s(asset_c::type_e::tileset, 2, "shimmer.iff") },
        { FONT, asset_def_s(asset_c::type_e::font, 2, "font.iff", [](const asset_manager_c &manager, const char *path) -> asset_c* {
            auto image = new image_c(path);
            return new font_c(image, size_s(8, 8), 4, 2, 4);
        })},
        { MONO_FONT, asset_def_s(asset_c::type_e::font, 2, nullptr, [](const asset_manager_c &manager, const char *path) -> asset_c* {
            return new font_c(manager.font(FONT).image(), size_s(8, 8));
        })},
        { SMALL_FONT, asset_def_s(asset_c::type_e::font, 2, "font6.iff", [](const asset_manager_c &manager, const char *path) -> asset_c* {
            auto image = new image_c(path);
            return new font_c(image, size_s(6, 6), 3, 0, 6);
        })},
        { SMALL_MONO_FONT, asset_def_s(asset_c::type_e::font, 2, nullptr, [](const asset_manager_c &manager, const char *path) -> asset_c* {
            return new font_c(manager.font(SMALL_FONT).image(), size_s(6, 6));
        })},
        { DISK, asset_def_s(asset_c::type_e::image, 2, "disk.iff") },
        { SPOT, asset_def_s(asset_c::type_e::image, 2, "spot.iff") },
        { DROP_ORB, asset_def_s(asset_c::type_e::sound, 4, "drop.aif") },
        { TAKE_ORB, asset_def_s(asset_c::type_e::sound, 4, "take.aif") },
        { FUSE_ORB, asset_def_s(asset_c::type_e::sound, 4, "fuse.aif") },
        { NO_DROP_ORB, asset_def_s(asset_c::type_e::sound, 4, "tock.aif") },
        { BREAK_TILE, asset_def_s(asset_c::type_e::sound, 4, "break.aif") },
        { FUSE_BREAK_TILE, asset_def_s(asset_c::type_e::sound, 4, "fusebrk.aif") },
        { MUSIC, asset_def_s(asset_c::type_e::music, 2, "music.snd") },
        { LEVELS, asset_def_s(asset_c::type_e::custom, 2, nullptr, [](const asset_manager_c &manager, const char *path) -> asset_c* {
            return new levels_c();
        })},
        { LEVEL_RESULTS, asset_def_s(asset_c::type_e::custom, 2, nullptr, [](const asset_manager_c &manager, const char *path) -> asset_c* {
            return new level_results_c(((cgasset_manager&)manager).levels().size());
        })},
        { USER_LEVELS, asset_def_s(asset_c::type_e::custom, 2, nullptr, [](const asset_manager_c &manager, const char *path) -> asset_c* {
            return new user_levels_c();
        })},
        { MENU_SCROLL, asset_def_s(asset_c::type_e::custom, 2, "menu.txt", [](const asset_manager_c &manager, const char *path) -> asset_c* {
            return new scroll_text_c(path);
        })}
    };
    
    for (const auto &asset_def : asset_defs) {
        add_asset_def(asset_def.first, asset_def.second);
    }
    
}

static inline bool _support_audio() {
    auto &machine = machine_c::shared();
    // Support audio if on a STe or newer machine with more than 500k RAM.
    return (machine.type() >= machine_c::type_e::ste); // && (machine.max_memory() > 512 * 1024);
}

bool cgasset_manager::support_audio() const {
    static bool s_support_audio = _support_audio();
    return s_support_audio;
}


asset_c *cgasset_manager::create_asset(int id, const asset_def_s &def) const {
    auto asset = asset_manager_c::create_asset(id, def);
    if (id >= TILES_A && id <= TILES_C) {
        tileset_c &tiles = *(tileset_c*)asset;
        auto remap = [&](int x) {
            assert(x == 1 || x == 2);
            constexpr canvas_c::remap_table_c tables[2] = {
                canvas_c::remap_table_c({ {2, 12}, {3, 13}, {4, 14} }),
                canvas_c::remap_table_c({ {2, 11}, {3, 8}, {4, 9} })
            };
            canvas_c tiles_cnv(*tiles.image());
            rect_s rect(x * 48, 0, 48, 80);
            tiles_cnv.draw(*tiles.image(), rect_s(0, 0, 48, 80), rect.origin);
            canvas_c::remap_table_c table;
            tiles_cnv.remap_colors(tables[x - 1], rect);
        };
        remap(1);
        remap(2);
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
        str << "levels" << (int16_t)(i++) << ".dat" << ends;
        iffstream_c iff(asset_manager_c::shared().data_path(str.str()).get(), fstream_c::openmode_e::input);
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

user_levels_c::user_levels_c() {
    uint8_t *recipes = (uint8_t *)_calloc(10, level_recipe_t::MAX_SIZE);
    for (int i = 0; i < 10; i++) {
        push_back((level_recipe_t *)(recipes + i * level_recipe_t::MAX_SIZE));
    }
    iffstream_c iff(asset_manager_c::shared().user_path("levels.dat").get(), fstream_c::openmode_e::input);
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
    iffstream_c iff(asset_manager_c::shared().user_path("levels.dat").get(), fstream_c::openmode_e::input | fstream_c::openmode_e::output);
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

level_results_c::level_results_c(int level_count) {
    auto &levels = cgasset_manager::shared().levels();
    bool success = false;
    iffstream_c iff(asset_manager_c::shared().user_path("scores.dat").get());
    if (!iff.good()) goto done;
    iff_group_s list;
    if (iff.first(IFF_LIST, IFF_CGLR, list)) {
        iff_chunk_s level_chunk;
        while (iff.next(list, IFF_CGLR, level_chunk)) {
            const uint16_t check = levels[size()]->f16check();
            emplace_back();
            auto &level_result = back();
            if (level_result.load(iff, level_chunk)) {
                if (level_result.f16check != check) {
                    memset(&level_result, 0, sizeof(level_result));
                    level_result.f16check = check;
                }
            } else {
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
        const uint16_t check = levels[size()]->f16check();
        emplace_back();
        back().f16check = check;
    }
}

bool level_results_c::save() const {
    iffstream_c iff(asset_manager_c::shared().user_path("scores.dat").get(), fstream_c::openmode_e::input | fstream_c::openmode_e::output);
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
    file.seek(0, stream_c::seekdir_e::end);
    auto size = file.tell();
    file.seek(0, stream_c::seekdir_e::beg);
    char *text = (char*)malloc(size + 1);
    file.read((uint8_t *)text, size);
    text[size] = 0;
    _text.reset(text);
}
