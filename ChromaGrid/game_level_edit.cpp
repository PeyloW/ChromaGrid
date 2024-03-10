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

static int button_to_level_idx(int button_idx) {
    int i = (10 - button_idx);
    return (i & 1) ? i - 1 : i + 1;
};

class cglevel_edit_persistence_scene_c : public cggame_scene_c {
public:
    cglevel_edit_persistence_scene_c(cgmanager_c &manager) :
        cggame_scene_c(manager),
        _menu_buttons(MAIN_MENU_BUTTONS_ORIGIN, MAIN_MENU_BUTTONS_SIZE, MAIN_MENU_BUTTONS_SPACING),
        _recipe(nullptr)
    {
        add_buttons();
    }

    cglevel_edit_persistence_scene_c(cgmanager_c &manager, level_recipe_t *recipe) :
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
        int button = update_button_group(screen, _menu_buttons);
        if (button == 0) {
            manager.pop();
        } else if (button > 0) {
            if (_recipe) {
                memcpy(rsc.user_levels[button_to_level_idx(button)], _recipe, level_recipe_t::MAX_SIZE);
                rsc.save_user_levels();
                manager.pop();
            } else {
                manager.pop();
                manager.replace(new cglevel_edit_scene_c(manager, rsc.user_levels[button_to_level_idx(button)]));
            }
        }
    }
    
private:
    void add_buttons() {
        bool save = _recipe != nullptr;
        static char buf[4*10];
        _menu_buttons.add_button("Cancel");
        char *start = buf;
        char *prev_start;
        for (int button_idx = 1; button_idx < 11; button_idx++) {
            int level_idx = button_to_level_idx(button_idx);
            bool empty = rsc.user_levels[level_idx]->empty();
            int pair_idx = level_idx & 0x1;
            
            if (save && !empty) {
                sprintf(start, "#%1d", level_idx + 1);
            } else {
                sprintf(start, "%1d", level_idx + 1);
            }
            if (pair_idx == 1) {
                _menu_buttons.add_buttons(prev_start, start);
                if (!save) {
                    if (rsc.user_levels[level_idx - 1]->empty()) {
                        _menu_buttons.buttons[button_idx - 1].state = cgbutton_t::disabled;
                    }
                    if (empty) {
                        _menu_buttons.buttons[button_idx].state = cgbutton_t::disabled;
                    }
                }
            }
            prev_start = start;
            start += 4;
        }
    }
    cgbutton_group_c<11> _menu_buttons;
    level_recipe_t *_recipe;
};

cglevel_edit_scene_c::cglevel_edit_scene_c(cgmanager_c &manager, level_recipe_t *recipe) :
    cggame_scene_c(manager),
    _menu_buttons(MAIN_MENU_BUTTONS_ORIGIN, MAIN_MENU_BUTTONS_SIZE, MAIN_MENU_BUTTONS_SPACING),
    _selected_template(0)
{
    memset(_level_grid, 0, sizeof(_level_grid));
    if (recipe) {
        _header = recipe->header;
        
        int off_x = (12 - recipe->header.width) / 2;
        int off_y = (12 - recipe->header.height) / 2;

        for (int y = 0; y < recipe->header.height; y++) {
            for (int x = 0; x < recipe->header.width; x++) {
                auto &src_tile = recipe->tiles[x + y * recipe->header.width];
                auto &dst_tile = _level_grid[off_x + x][off_y + y];
                dst_tile = src_tile;
            }
        }
    } else {
        _header.width = 12;
        _header.height = 12;
        _header.orbs[0] = 5;
        _header.orbs[1] = 5;
        _header.time = 60;
    }
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
}

void cglevel_edit_scene_c::tick(cgimage_c &screen, int ticks) {
    static union __attribute__((aligned(2))) {
        level_recipe_t recipe;
        uint8_t _dummy[level_recipe_t::MAX_SIZE];
    } temp;
    int button = update_button_group(screen, _menu_buttons);
    switch (button) {
        case 0:
            manager.pop();
            break;
        case 1: // Load level
            manager.push(new cglevel_edit_persistence_scene_c(manager));
            break;
        case 2: // Save level
            make_recipe(temp.recipe);
            manager.push(new cglevel_edit_persistence_scene_c(manager, &temp.recipe));
            break;
        case 3: // Try level
            make_recipe(temp.recipe);
            manager.push(new cglevel_scene_c(manager, &temp.recipe));
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

void cglevel_edit_scene_c::make_recipe(level_recipe_t &recipe) const {
    recipe.header.width = 12;
    recipe.header.height = 12;
    recipe.header.time = 60;
    recipe.header.orbs[0] = 10;
    recipe.header.orbs[1] = 10;
    recipe.text = nullptr;
    for (int y = 0; y < 12; y++) {
        for (int x = 0; x < 12; x++) {
            int i = x + y * 12;
            recipe.tiles[i] = _level_grid[x][y];
        }
    }
}
