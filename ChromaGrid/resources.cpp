//
//  resources.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-03-01.
//

#include "resources.hpp"
#include "game.hpp"

static void remap_to(color_e col, cgimage_c::remap_table_t table, uint8_t masked_idx = cgimage_c::MASKED_CIDX) {
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
    table[masked_idx] = cgimage_c::MASKED_CIDX;
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

cgresources_c::cgresources_c() :
    background(data_path("BACKGRND.IFF", "Load images"), false),
    tiles(data_path("TILES.IFF"), false),
    orbs(data_path("ORBS.IFF"), true, 6),
    cursor(data_path("CURSOR.IFF"), true, 0),
    button(data_path("BUTTON.IFF"), true, 6),
    _font_image(data_path("FONT.IFF", "Load fonts"), true, 0),
    _small_font_image(data_path("FONT6.IFF"), true, 0),
    font(_font_image, (cgsize_t){8, 8}, 4, 2, 4),
    small_font(_small_font_image, (cgsize_t){6, 6}, 3, 0, 6),
    music(data_path("music.snd", "Load music"))
{
    background.set_offset((cgpoint_t){0, 0});
    tiles.set_offset((cgpoint_t){0, 0});
    for (int x = 1; x < 3; x++) {
        printf("Initialize tiles %d.\n\r", x);
        cgrect_t rect = {{static_cast<int16_t>(x * 48), 0}, {48, 80}};
        tiles.draw(tiles, (cgrect_t){{0, 0}, {48, 80}}, rect.origin);
        cgimage_c::remap_table_t table;
        cgimage_c::make_noremap_table(table);
        if (x == 1) {
            remap_to(color_e::gold, table);
        } else {
            remap_to(color_e::silver, table);
        }
        tiles.remap_colors(table, rect);
    }
    orbs.set_offset((cgpoint_t){0, 0});
    cursor.set_offset((cgpoint_t){1, 2});

    printf("Initialize stencils.\n\r");
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j <= cgimage_c::STENCIL_FULLY_OPAQUE; j++) {
            cgimage_c::make_stencil(stencils[i][j], (cgimage_c::stencil_type_e)i, j);
        }
    }

}
