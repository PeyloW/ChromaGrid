//
//  resources.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-03-01.
//

#include "resources.hpp"
#include "game.hpp"
#include "iff_file.hpp"

static void remap_to(color_e col, image_c::remap_table_t table, uint8_t masked_idx = image_c::MASKED_CIDX) {
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

const cgresources_c& cgresources_c::shared() {
    static cgresources_c *rsc = nullptr;
    if (rsc == nullptr) {
        rsc = new cgresources_c();
    }
    return *rsc;
}

static const char *data_path(const char *file, const char *m = nullptr) {
    static char buffer[256];
    if (m) {
        printf("%s\n\r", m);
    }
#ifdef __M68000__
    strcpy(buffer, "data\\");
#else
    strcpy(buffer, "");
#endif
    strcat(buffer, file);
    return buffer;
}

static const char *user_path(const char *file, const char *m = nullptr) {
#ifdef __M68000__
    return file;
#else
    static char buffer[256];
    strcpy(buffer, "/tmp/");
    strcat(buffer, file);
    return buffer;
#endif
}


cgresources_c::cgresources_c() :
    background(data_path("BACKGRND.IFF", "Load images"), false),
    tiles(data_path("TILES.IFF"), false),
    orbs(data_path("ORBS.IFF"), true, 6),
    cursor(data_path("CURSOR.IFF"), true, 0),
    button(data_path("BUTTON.IFF"), true, 6),
    selection(data_path("SELECT.IFF"), true, 6),
    _font_image(data_path("FONT.IFF", "Load fonts"), true, 0),
    _small_font_image(data_path("FONT6.IFF"), true, 0),
    font(_font_image, (size_s){8, 8}, 4, 2, 4),
    mono_font(_font_image, (size_s){8, 8}),
    small_font(_small_font_image, (size_s){6, 6}, 3, 0, 6),
    small_mono_font(_small_font_image, (size_s){6, 6}),
    drop_orb(data_path("drop.aif", "Load audio")),
    take_orb(data_path("take.aif")),
    fuse_orb(data_path("fuse.aif")),
    no_drop_orb(data_path("tock.aif")),
    break_tile(data_path("break.aif")),
    fuse_break_tile(data_path("fusebrk.aif")),
    music(data_path("music.snd", "Load music"))
{
    background.set_offset((point_s){0, 0});
    tiles.set_offset((point_s){0, 0});
    selection.set_offset((point_s){0, 0});
    for (int x = 1; x < 3; x++) {
        printf("Initialize tiles %d.\n\r", x);
        rect_s rect = {{static_cast<int16_t>(x * 48), 0}, {48, 80}};
        tiles.draw(tiles, (rect_s){{0, 0}, {48, 80}}, rect.origin);
        image_c::remap_table_t table;
        image_c::make_noremap_table(table);
        if (x == 1) {
            remap_to(color_e::gold, table);
        } else {
            remap_to(color_e::silver, table);
        }
        tiles.remap_colors(table, rect);
    }
    orbs.set_offset((point_s){0, 0});
    cursor.set_offset((point_s){1, 2});

    printf("Pre-warm stencils.\n\r");
    image_c::get_stencil(image_c::orderred, 0);
    
    printf("Loading user levels.\n\r");
    int max_recipe_size = sizeof(level_recipe_t) + sizeof(tilestate_t) * (12 * 12);
    uint8_t *recipes = (uint8_t *)calloc(10, level_recipe_t::MAX_SIZE);
    for (int i = 0; i < 10; i++) {
        user_levels.push_back((level_recipe_t *)(recipes + i * max_recipe_size));
    }
    load_user_levels();
    
    printf("Loading levels.\n\r");
    load_levels();
    load_level_results();
    /*
        // Make one, if we cannot load.
        level_recipe_t *recipe = (level_recipe_t *)calloc(1, sizeof(level_recipe_t) + sizeof(tilestate_t) * 4);
        recipe->header.width = 2;
        recipe->header.height = 2;
        recipe->header.time = 75;
        recipe->header.orbs[0] = 3;
        recipe->header.orbs[1] = 4;
        recipe->text = nullptr;
        recipe->tiles[0] = { tiletype_e::regular, color_e::none, color_e::none, color_e::none };
        recipe->tiles[1] = { tiletype_e::glass, color_e::gold, color_e::none, color_e::none };
        recipe->tiles[2] = { tiletype_e::blocked, color_e::none, color_e::none, color_e::none };
        recipe->tiles[3] = { tiletype_e::regular, color_e::silver, color_e::none, color_e::gold };
        levels.push_back(recipe);
    */
}

void cgresources_c::load_levels() {
    iff_file_c  iff(data_path("levels.dat"));
    assert(iff.get_pos() == 0);
    iff.with_hard_asserts(true, [&] {
        iff_group_s list;
        iff.first(IFF_LIST, IFF_CGLV, list);
        uint8_t *data = (uint8_t *)calloc(1, list.size);
        iff_group_s level_group;
        while (levels.size() < 45 && iff.next(list, IFF_FORM, level_group)) {
            level_recipe_t *recipe = (level_recipe_t *)(data + iff.get_pos());
            recipe->load(iff, level_group);
            levels.emplace_back(recipe);
        }
    });
}

bool cgresources_c::load_user_levels() {
    FILE *file =  fopen(user_path("levels.dat"), "r");
    if (!file) {
        return false;
    }
    iff_file_c iff(file);
    iff.with_hard_asserts(true, [&] {
        iff_group_s list;
        iff.first(IFF_LIST, IFF_CGLV, list);
        int index = 0;
        iff_group_s level_group;
        while (iff.next(list, IFF_FORM, level_group)) {
            auto recipe = user_levels[index++];
            recipe->load(iff, level_group);
        }
    });
    fclose(file);
    return true;
}

bool cgresources_c::save_user_levels() const {
    iff_file_c iff(user_path("levels.dat"), "w+");
    if (iff.get_pos() < 0) {
        return false;
    }
    iff.with_hard_asserts(true, [&] {
        iff_group_s list;
        iff.begin(list, IFF_LIST);
        iff.write(IFF_CGLV_ID);
        for (int index = 0; index < user_levels.size(); index++) {
            auto &recipe = user_levels[index];
            if (!recipe->empty()) {
                recipe->save(iff);
            }
        }
        iff.end(list);
    });
    return true;
}

bool cgresources_c::load_level_results() {
    bool success = false;
    FILE *file =  fopen(user_path("scores.dat"), "r");
    if (file) {
        iff_file_c iff(file);
        iff_group_s list;
        if (iff.first(IFF_LIST, IFF_CGLR, list)) {
            iff_chunk_s level_chunk;
            while (iff.next(list, IFF_CGLR, level_chunk)) {
                level_results.push_back();
                auto &level_result = level_results.back();
                if (!level_result.load(iff, level_chunk)) {
                    goto done;
                }
            }
        }
        fclose(file);
        success = true;
    }
done:
    if (!success) {
        level_results.clear();
    }
    while (level_results.size() < levels.size()) {
        level_results.push_back();
    }
    return success;
}

bool cgresources_c::save_level_results() const {
    iff_file_c iff(user_path("scores.dat"), "w+");
    iff_group_s list;
    if (iff.begin(list, IFF_LIST)) {
        iff.write(IFF_CGLR_ID);
        for (auto result = level_results.begin(); result != level_results.end(); result++) {
            result->save(iff);
        }
        return iff.end(list);
    }
    return false;
}
