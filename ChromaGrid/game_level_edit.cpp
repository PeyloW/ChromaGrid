//
//  game_level_edit.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-03-06.
//

#include "game.hpp"


#define LEVEL_EDIT_TEMPLATE_SIZE_WIDTH (16 * 6)
#define LEVEL_EDIT_TEMPLATE_SIZE_HEIGHT (16 * 3)
#define LEVEL_EDIT_TEMPLATE_ORIGIN_X (MAIN_MENU_ORIGIN_X + (MAIN_MENU_SIZE_WIDTH - LEVEL_EDIT_TEMPLATE_SIZE_WIDTH) / 2)
#define LEVEL_EDIT_TEMPLATE_ORIGIN_Y (72)

class cglevel_edit_persistence_scene_c : public cggame_scene_c {
public:
    cglevel_edit_persistence_scene_c(cgmanager_c &manager) :
        cggame_scene_c(manager),
        _menu_buttons(MAIN_MENU_BUTTONS_ORIGIN, MAIN_MENU_BUTTONS_SIZE, MAIN_MENU_BUTTONS_SPACING),
        _recipe(nullptr)
    {
        add_buttons();
    }

    cglevel_edit_persistence_scene_c(cgmanager_c &manager, level_t::recipe_t *recipe) :
        cggame_scene_c(manager),
        _menu_buttons(MAIN_MENU_BUTTONS_ORIGIN, MAIN_MENU_BUTTONS_SIZE, MAIN_MENU_BUTTONS_SPACING),
        _recipe(recipe)
    {
        add_buttons();
    }

    virtual void will_appear(cgimage_c &screen, bool obsured) {
        cgrect_t rect = (cgrect_t) {
            {0,0},
            {MAIN_MENU_ORIGIN_X, 200}
        };
        screen.with_stencil(cgimage_c::get_stencil(cgimage_c::orderred, 32), [this, &screen, &rect] {
            screen.draw_aligned(rsc.background, rect, rect.origin);
        });
        rect = (cgrect_t){
            (cgpoint_t){MAIN_MENU_ORIGIN_X, 0},
            (cgsize_t){MAIN_MENU_SIZE_WIDTH, 200}
        };
        screen.draw_aligned(rsc.background, rect, rect.origin);
        _menu_buttons.draw_all(screen);
    }
    
    virtual void tick(cgimage_c &screen, int ticks) {
    }
    
private:
    void add_buttons() {
        static char buf[3*10];
        _menu_buttons.add_button("Cancel");
        for (int i = 0; i < 5; i++) {
            sprintf(buf + i * 6, "%1d", 9 - (i * 2));
            sprintf(buf + i * 6 + 3, "%1d", 10 - (i * 2));
            _menu_buttons.add_buttons(buf + i * 6, buf + i * 6 + 3);
        }
    }
    cgbutton_group_c<11> _menu_buttons;
    level_t::recipe_t *_recipe;
};

cglevel_edit_scene_c::cglevel_edit_scene_c(cgmanager_c &manager, level_t::recipe_t *recipe) :
    cggame_scene_c(manager),
    _menu_buttons(MAIN_MENU_BUTTONS_ORIGIN, MAIN_MENU_BUTTONS_SIZE, MAIN_MENU_BUTTONS_SPACING),
    _selected_template(0),
    _tested_recipe(nullptr)
{
    memset(_level_grid, 0, sizeof(_level_grid));
    _menu_buttons.add_button("Main Menu");
    _menu_buttons.add_buttons("Load", "Save");
    _menu_buttons.add_button("Try Level");
    
    _tile_templates.push_back((tilestate_t){ tiletype_e::regular, color_e::none, color_e::none, color_e::none});
    _tile_templates.push_back((tilestate_t){ tiletype_e::regular, color_e::gold, color_e::none, color_e::none});
    _tile_templates.push_back((tilestate_t){ tiletype_e::regular, color_e::silver, color_e::none, color_e::none});
    _tile_templates.push_back((tilestate_t){ tiletype_e::magnetic, color_e::none, color_e::none, color_e::none});
    _tile_templates.push_back((tilestate_t){ tiletype_e::magnetic, color_e::gold, color_e::none, color_e::none});
    _tile_templates.push_back((tilestate_t){ tiletype_e::magnetic, color_e::silver, color_e::none, color_e::none});
    _tile_templates.push_back((tilestate_t){ tiletype_e::glass, color_e::none, color_e::none, color_e::none});
    _tile_templates.push_back((tilestate_t){ tiletype_e::glass, color_e::gold, color_e::none, color_e::none});
    _tile_templates.push_back((tilestate_t){ tiletype_e::glass, color_e::silver, color_e::none, color_e::none});
    _tile_templates.push_back((tilestate_t){ tiletype_e::broken, color_e::none, color_e::none, color_e::none});
    _tile_templates.push_back((tilestate_t){ tiletype_e::broken, color_e::gold, color_e::none, color_e::none});
    _tile_templates.push_back((tilestate_t){ tiletype_e::broken, color_e::silver, color_e::none, color_e::none});
    _tile_templates.push_back((tilestate_t){ tiletype_e::blocked, color_e::none, color_e::none, color_e::none});
    _tile_templates.push_back((tilestate_t){ tiletype_e::empty, color_e::none, color_e::none, color_e::gold});
    _tile_templates.push_back((tilestate_t){ tiletype_e::empty, color_e::none, color_e::none, color_e::silver});
}

void cglevel_edit_scene_c::will_appear(cgimage_c &screen, bool obsured) {
    screen.draw_aligned(rsc.background, (cgpoint_t){0, 0});
    _menu_buttons.draw_all(screen);
    draw_tile_templates(screen);
    for (int y = 0; y < 12; y++) {
        for (int x = 0; x < 12; x++) {
            draw_level_grid(screen, x, y);
        }
    }
    if (_tested_recipe) {
        free(_tested_recipe);
        _tested_recipe = nullptr;
    }
}

void cglevel_edit_scene_c::tick(cgimage_c &screen, int ticks) {
    int button = update_button_group(screen, _menu_buttons);
    switch (button) {
        case 0:
            manager.pop();
            break;
        case 1: // Load level
            manager.push(new cglevel_edit_persistence_scene_c(manager));
            break;
        case 2: // Save level
            manager.push(new cglevel_edit_persistence_scene_c(manager, make_recipe()));
            break;
        case 3: // Try level
            manager.push(new cglevel_scene_c(manager, make_recipe()));
            break;
        default:
            break;
    }
    
    auto &mouse = manager.mouse;
    bool lb = mouse.get_state(cgmouse_c::left) == cgmouse_c::clicked;
    bool rb = mouse.get_state(cgmouse_c::right) == cgmouse_c::clicked;
    const auto pos = mouse.get_postion();
    if (lb || rb) {
        if (pos.x < MAIN_MENU_ORIGIN_X && pos.y < MAIN_MENU_SIZE_HEIGHT) {
            int tx = pos.x / 16;
            int ty = pos.y / 16;
            if (lb) {
                auto temp = _tile_templates[_selected_template];
                if (temp.orb != none) {
                    if (_level_grid[tx][ty].can_have_orb()) {
                        _level_grid[tx][ty].orb = temp.orb;
                    }
                } else {
                    auto orb = _level_grid[tx][ty].orb;
                    _level_grid[tx][ty] = _tile_templates[_selected_template];
                    if (_level_grid[tx][ty].can_have_orb()) {
                        _level_grid[tx][ty].orb = orb;
                    }
                }
            } else if (rb) {
                auto temp = _tile_templates[_selected_template];
                if (temp.orb != none) {
                    _level_grid[tx][ty].orb = none;
                } else {
                    _level_grid[tx][ty] = (tilestate_t) { empty, none, none, none};
                }
            }
            draw_level_grid(screen, tx, ty);
        }
        cgrect_t rect = (cgrect_t){
            {LEVEL_EDIT_TEMPLATE_ORIGIN_X, LEVEL_EDIT_TEMPLATE_ORIGIN_Y},
            {LEVEL_EDIT_TEMPLATE_SIZE_WIDTH, LEVEL_EDIT_TEMPLATE_SIZE_HEIGHT}
        };
        if (lb && rect.contains(pos)) {
            int tx = (pos.x - LEVEL_EDIT_TEMPLATE_ORIGIN_X) / 16;
            int ty = (pos.y - LEVEL_EDIT_TEMPLATE_ORIGIN_Y) / 16;
            int i = ty * 6 + tx;
            if (i < _tile_templates.size()) {
                _selected_template = i;
                draw_tile_templates(screen);
            }
        }
    }
}

void cglevel_edit_scene_c::draw_tile_templates(cgimage_c &screen) const {
    for (int i = 0; i < 15; i++) {
        int row = i / 6;
        int col = i % 6;
        int16_t x = LEVEL_EDIT_TEMPLATE_ORIGIN_X + col * 16;
        int16_t y = LEVEL_EDIT_TEMPLATE_ORIGIN_Y + row * 16;
        cgpoint_t at = (cgpoint_t){x, y};
        draw_tilestate(screen, _tile_templates[i], at, _selected_template == i);
    }
}

void cglevel_edit_scene_c::draw_level_grid(cgimage_c &screen, int x, int y) const {
    cgpoint_t at = (cgpoint_t){(int16_t)(x * 16), (int16_t)(y * 16)};
    draw_tilestate(screen, _level_grid[x][y], at);
}

level_t::recipe_t *cglevel_edit_scene_c::make_recipe() const {
    level_t::recipe_t *recipe = (level_t::recipe_t *)calloc(1, sizeof(level_t::recipe_t) + sizeof(tilestate_t) * 12 * 12);
    recipe->width = 12;
    recipe->height = 12;
    recipe->time = 60;
    recipe->orbs[0] = 10;
    recipe->orbs[1] = 10;
    for (int y = 0; y < 12; y++) {
        for (int x = 0; x < 12; x++) {
            int i = x + y * 12;
            recipe->tiles[i] = _level_grid[x][y];
        }
    }
    return recipe;
}
