//
//  game_level_edit.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-03-06.
//

#include "game.hpp"


#define LEVEL_EDIT_TEMPLATE_SIZE_WIDTH (16 * 6)
#define LEVEL_EDIT_TEMPLATE_SIZE_HEIGHT (16)
#define LEVEL_EDIT_TEMPLATE_ORIGIN_X (MAIN_MENU_ORIGIN_X + (MAIN_MENU_SIZE_WIDTH - LEVEL_EDIT_TEMPLATE_SIZE_WIDTH) / 2)
#define LEVEL_EDIT_TEMPLATE_ORIGIN_Y (108)

#define LEVEL_EDIT_BUTTON_ORIGIN_X (MAIN_MENU_ORIGIN_X + MAIN_MENU_MARGINS * 2)
#define LEVEL_EDIT_BUTTON_ORIGIN_Y (56)
#define LEVEL_EDIT_BUTTON_ORIGIN (cgpoint_t){LEVEL_EDIT_BUTTON_ORIGIN_X, LEVEL_EDIT_BUTTON_ORIGIN_Y}
#define LEVEL_EDIT_BUTTON_SIZE_WIDTH    (48)
#define LEVEL_EDIT_BUTTON_SIZE (cgsize_t){LEVEL_EDIT_BUTTON_SIZE_WIDTH, 14}
#define LEVEL_EDIT_BUTTON_SPACING (2)

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
                _menu_buttons.add_button_pair(prev_start, start);
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
    _count_buttons(LEVEL_EDIT_BUTTON_ORIGIN, LEVEL_EDIT_BUTTON_SIZE, LEVEL_EDIT_BUTTON_SPACING),
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
    _menu_buttons.add_button_pair("Load", "Save");
    _menu_buttons.add_button("Try Level");
    
    for (int i = 0; i < 3; i++) {
        _count_buttons.add_button_pair("+", "-", 2);
    }
    
    _tile_templates.push_back((tilestate_t){ tiletype_e::regular, color_e::none, color_e::none, color_e::none});
    _tile_templates.push_back((tilestate_t){ tiletype_e::magnetic, color_e::none, color_e::none, color_e::none});
    _tile_templates.push_back((tilestate_t){ tiletype_e::glass, color_e::none, color_e::none, color_e::none});
    _tile_templates.push_back((tilestate_t){ tiletype_e::broken, color_e::none, color_e::none, color_e::none});
    _tile_templates.push_back((tilestate_t){ tiletype_e::blocked, color_e::none, color_e::none, color_e::none});
    _tile_templates.push_back((tilestate_t){ tiletype_e::empty, color_e::none, color_e::none, color_e::both});
}

void cglevel_edit_scene_c::will_appear(cgimage_c &screen, bool obsured) {
    screen.draw_aligned(rsc.background, (cgpoint_t){0, 0});
    _menu_buttons.draw_all(screen);
    _count_buttons.draw_all(screen);
    draw_counts(screen);
    draw_tile_templates(screen);
    for (int y = 0; y < 12; y++) {
        for (int x = 0; x < 12; x++) {
            draw_level_grid(screen, x, y);
        }
    }
}

tilestate_t cglevel_edit_scene_c::next_state(const tilestate_t &current, cgmouse_c::button_e button) const {
    static const tilestate_t empty = (tilestate_t){ tiletype_e::empty, color_e::none, color_e::none, color_e::none };
    auto &selected = _tile_templates[_selected_template];
    switch (current.type) {
        case tiletype_e::empty:
            return selected.orb == none ? selected : current;
            break;
        case tiletype_e::blocked:
            if (selected.orb != none) {
                return current;
            } else if (selected.type == current.type) {
                return button == cgmouse_c::right ? empty : current;
            }
            // fall through
        default: {
            auto copy = current;
            if (selected.orb != none) {
                if (current.orb != none) {
                    copy.orb = none;
                } else {
                    copy.orb = (color_e)(2 - button);
                }
            } else if (selected.type == current.type) {
                if (button == cgmouse_c::left) {
                    copy.target = (color_e)(current.target + 1);
                    if (copy.target >= color_e::both) {
                        copy = empty;
                    } else if (copy.target != copy.current && copy.current != color_e::none) {
                        copy.current = color_e::none;
                    }
                } else if (button == cgmouse_c::right) {
                    if (current.current == color_e::none) {
                        copy.current = current.target;
                    } else {
                        copy = empty;
                    }
                }
            } else {
                copy = selected;
            }
            return copy;
        }
    }
    return current;
}

void cglevel_edit_scene_c::tick(cgimage_c &screen, int ticks) {
    static union {
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
    
    button = update_button_group(screen, _count_buttons);
    if (button >= 0) {
        int step = (button & 1) ? -1 : 1;
        int group = button / 2;
        switch (group) {
            case 0:
                _header.time += (step * 5);
                _header.time = MAX(10, MIN(_header.time, 300));
                break;
            default:
                _header.orbs[group - 1] += step;
                _header.orbs[group - 1] = MAX(0, (int8_t)MIN((int8_t)_header.orbs[group - 1], 20));
                break;
        }
        draw_counts(screen);
    }
    
    auto &mouse = manager.mouse;
    bool lb = mouse.get_state(cgmouse_c::left) == cgmouse_c::clicked;
    bool rb = mouse.get_state(cgmouse_c::right) == cgmouse_c::clicked;
    const auto pos = mouse.get_postion();
    
    if (pos.x < MAIN_MENU_ORIGIN_X && pos.y < MAIN_MENU_SIZE_HEIGHT) {
        int tx = pos.x / 16;
        int ty = pos.y / 16;
        auto &current = _level_grid[tx][ty];
        auto l_next = next_state(current, cgmouse_c::left);
        auto r_next = next_state(current, cgmouse_c::right);
        if (lb || rb) {
            _level_grid[tx][ty] = lb ? l_next : r_next;
            draw_level_grid(screen, tx, ty);
        } else if (current.type == _tile_templates[_selected_template].type && _tile_templates[_selected_template].orb != color_e::both) {
            draw_tile_template_at(screen, l_next, _selected_template);
        } else {
            draw_tile_template_at(screen, _tile_templates[_selected_template], _selected_template);
        }
    }
    
    if (lb || rb) {
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

void cglevel_edit_scene_c::draw_counts(cgimage_c &screen) const {
    int min = _header.time / 60;
    int sec = _header.time % 60;
    char buf[5];
    buf[0] = '0' + min;
    buf[1] = ':';
    buf[2] = '0' + (sec / 10);
    buf[3] = '0' + (sec % 10);
    buf[4] = 0;
    const cgpoint_t at = (cgpoint_t){ 320 - 32 - MAIN_MENU_MARGINS * 2, LEVEL_EDIT_BUTTON_ORIGIN_Y + 3};
    const cgrect_t rect = (cgrect_t){at, (cgsize_t){32, 8}};
    screen.draw(rsc.background, rect, at);
    screen.draw(rsc.mono_font, buf, at, cgimage_c::align_left);
    for (int i = 1; i < 3; i++) {
        cgpoint_t at = (cgpoint_t){ 320 - 32 - 8 - MAIN_MENU_MARGINS * 2, (int16_t)(LEVEL_EDIT_BUTTON_ORIGIN_Y + 3 + 16 * i)};
        draw_orb(screen, (color_e)i, at);
        
        char buf[3];
        auto d1 = _header.orbs[i - 1] / 10;
        buf[0] = d1 ? '0' + d1 :  ' ';
        buf[1] = '0' + _header.orbs[i - 1] % 10;
        buf[2] = 0;
        at = (cgpoint_t){ 320 - 16 - MAIN_MENU_MARGINS * 2, (int16_t)(LEVEL_EDIT_BUTTON_ORIGIN_Y + 3 + 16 * i)};
        cgrect_t rect = (cgrect_t){ at, {16, 8}};
        screen.draw(rsc.background, rect, at);
        screen.draw(rsc.mono_font, buf, at, cgimage_c::align_left);
    }
}


void cglevel_edit_scene_c::draw_tile_template_at(cgimage_c &screen, tilestate_t &state, int index) const {
    int16_t x = LEVEL_EDIT_TEMPLATE_ORIGIN_X + index * 16;
    cgpoint_t at = (cgpoint_t){x, LEVEL_EDIT_TEMPLATE_ORIGIN_Y};
    draw_tilestate(screen, state, at, _selected_template == index);
}

void cglevel_edit_scene_c::draw_tile_templates(cgimage_c &screen) const {
    for (int i = 0; i < _tile_templates.size(); i++) {
        draw_tile_template_at(screen, _tile_templates[i], i);
    }
}

void cglevel_edit_scene_c::draw_level_grid(cgimage_c &screen, int x, int y) const {
    cgpoint_t at = (cgpoint_t){(int16_t)(x * 16), (int16_t)(y * 16)};
    draw_tilestate(screen, _level_grid[x][y], at);
}

void cglevel_edit_scene_c::make_recipe(level_recipe_t &recipe) const {
    int x1 = 12, x2 = -1;
    int y1 = 12, y2 = -1;
    for (int y = 0; y < 12; y++) {
        for (int x = 0; x < 12; x++) {
            if (_level_grid[x][y].type != empty) {
                x1 = MIN(x1, x);
                x2 = MAX(x2, x);
                y1 = MIN(y1, y);
                y2 = MAX(y2, y);
            }
        }
    }
    if (x2 == -1) {
        memset(&recipe, 0, level_recipe_t::MAX_SIZE);
    } else {
        recipe.header.width = x2 - x1 + 1;
        recipe.header.height = y2 - y1 + 1;
        recipe.header.time = _header.time;
        recipe.header.orbs[0] = _header.orbs[0];
        recipe.header.orbs[1] = _header.orbs[1];
        recipe.text = nullptr;
        int i = 0;
        for (int y = y1; y <= y2; y++) {
            for (int x = x1; x <= x2; x++) {
                recipe.tiles[i++] = _level_grid[x][y];
            }
        }
    }
}
