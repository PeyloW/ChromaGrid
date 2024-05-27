//
//  game_level_edit.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-03-06.
//

#include "game.hpp"
#include "machine.hpp"

extern "C" {
#ifdef __M68000__
#include <ext.h>
#else
#include <unistd.h>
#endif
}

#define MAX_ORBS 50
#define MAX_TIME (5*60)

#define LEVEL_EDIT_TEMPLATE_SIZE_WIDTH (16 * 7)
#define LEVEL_EDIT_TEMPLATE_SIZE_HEIGHT (16)
#define LEVEL_EDIT_TEMPLATE_ORIGIN_X (MAIN_MENU_ORIGIN_X + (MAIN_MENU_SIZE_WIDTH - LEVEL_EDIT_TEMPLATE_SIZE_WIDTH) / 2)
#define LEVEL_EDIT_TEMPLATE_ORIGIN_Y (108)

#define LEVEL_EDIT_BUTTON_ORIGIN_X (MAIN_MENU_ORIGIN_X + MAIN_MENU_MARGINS * 2)
#define LEVEL_EDIT_BUTTON_ORIGIN_Y (56)
#define LEVEL_EDIT_BUTTON_ORIGIN (point_s(LEVEL_EDIT_BUTTON_ORIGIN_X, LEVEL_EDIT_BUTTON_ORIGIN_Y))
#define LEVEL_EDIT_BUTTON_SIZE_WIDTH    (48)
#define LEVEL_EDIT_BUTTON_SIZE (size_s(LEVEL_EDIT_BUTTON_SIZE_WIDTH, 14))
#define LEVEL_EDIT_BUTTON_SPACING (2)

static int button_to_level_idx(int button_idx) {
    int i = (10 - button_idx);
    return (i & 1) ? i - 1 : i + 1;
};

class cglevel_edit_persistence_scene_c : public cggame_scene_c {
public:
    cglevel_edit_persistence_scene_c(scene_manager_c &manager) :
        cggame_scene_c(manager),
        _menu_buttons(MAIN_MENU_BUTTONS_ORIGIN, MAIN_MENU_BUTTONS_SIZE, MAIN_MENU_BUTTONS_SPACING),
        _recipe(nullptr)
    {
        add_buttons();
    }

    cglevel_edit_persistence_scene_c(scene_manager_c &manager, level_recipe_t *recipe) :
        cggame_scene_c(manager),
        _menu_buttons(MAIN_MENU_BUTTONS_ORIGIN, MAIN_MENU_BUTTONS_SIZE, MAIN_MENU_BUTTONS_SPACING),
        _recipe(recipe)
    {
        add_buttons();
    }

    virtual void will_appear(screen_c &clear_screen, bool obsured) {
        auto &canvas = clear_screen.canvas();
        rect_s rect(0, 0, MAIN_MENU_ORIGIN_X, 200);
        canvas.with_stencil(canvas_c::stencil(canvas_c::orderred, 32), [this, &canvas, &rect] {
            canvas.draw_aligned(background, rect, rect.origin);
        });
        rect = rect_s(
            MAIN_MENU_ORIGIN_X, 0,
            MAIN_MENU_SIZE_WIDTH, 200
        );
        canvas.draw_aligned(background, rect, rect.origin);
        _menu_buttons.draw_all(canvas);
    }
    
    virtual void update_clear(screen_c &clear_screen, int ticks) {
        auto &canvas = clear_screen.canvas();
        int button = update_button_group(canvas, _menu_buttons);
        auto transition = transition_c::create(canvas_c::random);
        if (button == 0) {
            manager.pop(transition);
        } else if (button > 0) {
            auto &user_levels = assets.user_levels();
            if (_recipe) {
                auto &disk = cgasset_manager::shared().image(DISK);
                manager.screen(scene_manager_c::screen_e::front).canvas().draw(disk, point_s(288, 8));
                memcpy(user_levels[button_to_level_idx(button)], _recipe, level_recipe_t::MAX_SIZE);
                // TODO: Handle error and ask user to retry
                assets.user_levels().save();
                #ifndef __M68000__
                sleep(2);
                #endif
                manager.pop(transition);
            } else {
                manager.pop(nullptr);
                manager.replace(new cglevel_edit_scene_c(manager, user_levels[button_to_level_idx(button)]), transition);
            }
        }
    }
    
private:
    void add_buttons() {
        auto &user_levels = assets.user_levels();
        bool save = _recipe != nullptr;
        static char buf[4*10];
        strstream_c str(buf, 4 * 10);
        _menu_buttons.add_button("Cancel");
        char *start = buf;
        char *prev_start;
        for (int button_idx = 1; button_idx < 11; button_idx++) {
            int level_idx = button_to_level_idx(button_idx);
            bool empty = user_levels[level_idx]->empty();
            int pair_idx = level_idx & 0x1;
            
            start = str.str() + str.tell();
            str << level_idx + 1 << ends;
            if (pair_idx == 1) {
                _menu_buttons.add_button_pair(prev_start, start);
                if (save) {
                    if (!user_levels[level_idx - 1]->empty()) {
                        _menu_buttons.buttons[button_idx - 1].style = cgbutton_t::destructive;
                    }
                    if (!empty) {
                        _menu_buttons.buttons[button_idx].style = cgbutton_t::destructive;
                    }
                } else {
                    if (user_levels[level_idx - 1]->empty()) {
                        _menu_buttons.buttons[button_idx - 1].state = cgbutton_t::disabled;
                    }
                    if (empty) {
                        _menu_buttons.buttons[button_idx].state = cgbutton_t::disabled;
                    }
                }
            }
            prev_start = start;
        }
    }
    cgbutton_group_c<11> _menu_buttons;
    level_recipe_t *_recipe;
};

cglevel_edit_scene_c::cglevel_edit_scene_c(scene_manager_c &manager, level_recipe_t *recipe) :
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
    
    _tile_templates.push_back((tilestate_t){ tiletype_e::empty, color_e::both, color_e::none, color_e::none});
    _tile_templates.push_back((tilestate_t){ tiletype_e::regular, color_e::none, color_e::none, color_e::none});
    _tile_templates.push_back((tilestate_t){ tiletype_e::magnetic, color_e::none, color_e::none, color_e::none});
    _tile_templates.push_back((tilestate_t){ tiletype_e::glass, color_e::none, color_e::none, color_e::none});
    _tile_templates.push_back((tilestate_t){ tiletype_e::broken, color_e::none, color_e::none, color_e::none});
    _tile_templates.push_back((tilestate_t){ tiletype_e::blocked, color_e::none, color_e::none, color_e::none});
    _tile_templates.push_back((tilestate_t){ tiletype_e::empty, color_e::none, color_e::none, color_e::both});
    
    _shimmer_ticks = 0;
}

static const char *_template_help_texts[7] = {
    "Place gold or silver targets with left or right mouse button. Select again to remove target.",
    "Place regular tiles with left mouse button, remove with right mouse button. Select again to rotate target color.",
    "Place magnetic tiles with left mouse button, remove with right mouse button. Select again to rotate the tile's current color.",
    "Place glass tiles with left mouse button, remove with right mouse button. Select again to rotate the tile's current color.",
    "Place broken glass tiles with left mnouse button, remove with right mouse button. Select again to rotate the tile's current color.",
    "Place blocked tiles with left mouse button, remove with right mouse button.",
    "Place gold or silver orbs with left or right mouse button. Select again to remove orb.",
};

void cglevel_edit_scene_c::will_appear(screen_c &clear_screen, bool obsured) {
    auto &canvas = clear_screen.canvas();
    canvas.draw_aligned(background, point_s());
    _menu_buttons.draw_all(canvas);
    _count_buttons.draw_all(canvas);
    draw_counts(canvas);
    _selected_template = 1;
    draw_tile_templates(canvas);
    for (int y = 0; y < 12; y++) {
        for (int x = 0; x < 12; x++) {
            draw_level_grid(canvas, x, y);
        }
    }
    _scroller.restore();
}

tilestate_t cglevel_edit_scene_c::next_state(const tilestate_t &current, mouse_c::button_e button) const {
    static const tilestate_t empty = (tilestate_t){ tiletype_e::empty, color_e::none, color_e::none, color_e::none };
    auto copy = current;
    auto &selected = _tile_templates[_selected_template];
    if (selected.orb != none) {
        if (current.orb != none) {
            copy.orb = none;
        } else {
            copy.orb = (color_e)(2 - button);
        }
    } else if (selected.target != none) {
        if (current.target != none) {
            copy.target = none;
        } else {
            copy.target = (color_e)(2 - button);
        }
    } else if (button == mouse_c::right) {
        copy = empty;
        copy.target = current.target;
    } else if (selected.type == blocked) {
        copy = selected;
    } else if (selected.type == current.type && current.target != none) {
        copy.current = current.current == none ? current.target : none;
    } else {
        copy.type = selected.type;
        copy.current = none;
        copy.orb = none;
    }
    return copy;
}

void cglevel_edit_scene_c::update_clear(screen_c &clear_screen, int ticks) {
    auto &canvas = clear_screen.canvas();
    static union {
        level_recipe_t recipe;
        uint8_t _dummy[level_recipe_t::MAX_SIZE];
    } temp;
    int button = update_button_group(canvas, _menu_buttons);
    switch (button) {
        case 0:
            manager.pop();
            break;
        case 1: // Load level
        case 2: {// Save level
            auto transition = transition_c::create(canvas_c::random);
            if (button == 1) {
                manager.push(new cglevel_edit_persistence_scene_c(manager), transition);
            } else {
                make_recipe(temp.recipe);
                manager.push(new cglevel_edit_persistence_scene_c(manager, &temp.recipe), transition);
            }
            break;
        }
        case 3: { // Try level
            make_recipe(temp.recipe);
            auto color = machine_c::shared().active_palette()->colors[0];
            auto transition = transition_c::create(color);
            manager.push(new cglevel_scene_c(manager, &temp.recipe), transition);
            break;
        }
        default:
            break;
    }
    
    button = update_button_group(canvas, _count_buttons);
    if (button >= 0) {
        int step = (button & 1) ? -1 : 1;
        int group = button / 2;
        switch (group) {
            case 0:
                _header.time += (step * 5);
                _header.time = MAX(10, MIN(_header.time, MAX_TIME));
                break;
            default:
                _header.orbs[group - 1] += step;
                _header.orbs[group - 1] = MAX(0, (int8_t)MIN((int8_t)_header.orbs[group - 1], MAX_ORBS));
                break;
        }
        draw_counts(canvas);
    }
    
    bool lb = mouse.state(mouse_c::left) == mouse_c::clicked;
    bool rb = mouse.state(mouse_c::right) == mouse_c::clicked;
    const auto pos = mouse.postion();
    
    if (pos.x < MAIN_MENU_ORIGIN_X && pos.y < MAIN_MENU_SIZE_HEIGHT) {
        int tx = pos.x / 16;
        int ty = pos.y / 16;
        auto &current = _level_grid[tx][ty];
        auto l_next = next_state(current, mouse_c::left);
        auto r_next = next_state(current, mouse_c::right);
        if (lb || rb) {
            _level_grid[tx][ty] = lb ? l_next : r_next;
            draw_level_grid(canvas, tx, ty);
        } else if (current.type == _tile_templates[_selected_template].type && _tile_templates[_selected_template].orb != color_e::both && _tile_templates[_selected_template].target != color_e::both) {
            draw_tile_template_at(canvas, l_next, _selected_template);
        } else {
            draw_tile_template_at(canvas, _tile_templates[_selected_template], _selected_template);
        }
    }
    
    if (lb || rb) {
        rect_s rect(
            LEVEL_EDIT_TEMPLATE_ORIGIN_X, LEVEL_EDIT_TEMPLATE_ORIGIN_Y,
            LEVEL_EDIT_TEMPLATE_SIZE_WIDTH, LEVEL_EDIT_TEMPLATE_SIZE_HEIGHT
        );
        if (lb && rect.contains(pos)) {
            int tx = (pos.x - LEVEL_EDIT_TEMPLATE_ORIGIN_X) / 16;
            int ty = (pos.y - LEVEL_EDIT_TEMPLATE_ORIGIN_Y) / 16;
            int i = ty * 6 + tx;
            if (i < _tile_templates.size()) {
                _selected_template = i;
                draw_tile_templates(canvas);
            }
        }
    }
    _scroller.update(clear_screen);
}

static int next_shimmer_ticks() {
    return 25 + (uint16_t)rand() % 50;
}

void cglevel_edit_scene_c::update_back(screen_c &back_screen, int ticks) {
    _shimmer_ticks -= ticks;
    if (_shimmer_ticks < -7) {
        _shimmer_ticks = next_shimmer_ticks();
    } else if (_shimmer_ticks <= 0) {
        int16_t idx = ABS(_shimmer_ticks);
        point_s at(LEVEL_EDIT_TEMPLATE_ORIGIN_X + _selected_template * 16, LEVEL_EDIT_TEMPLATE_ORIGIN_Y);
        back_screen.canvas().draw(assets.tileset(SHIMMER), idx, at);
    }
}

void cglevel_edit_scene_c::draw_counts(canvas_c &screen) const {
    auto &mono_font = assets.font(MONO_FONT);
    int min = _header.time / 60;
    int sec = _header.time % 60;
    char buf[5];
    buf[0] = '0' + min;
    buf[1] = ':';
    buf[2] = '0' + (sec / 10);
    buf[3] = '0' + (sec % 10);
    buf[4] = 0;
    const point_s at = point_s( 320 - 32 - MAIN_MENU_MARGINS * 2, LEVEL_EDIT_BUTTON_ORIGIN_Y + 3);
    const rect_s rect = rect_s(at, size_s(32, 8));
    screen.draw(background, rect, at);
    screen.draw(mono_font, buf, at, canvas_c::align_left);
    for (int i = 1; i < 3; i++) {
        point_s at(320 - 32 - 8 - MAIN_MENU_MARGINS * 2 + 3, LEVEL_EDIT_BUTTON_ORIGIN_Y + 3 + 16 * i);
        draw_orb(screen, (color_e)i, at);
        
        char buf[3];
        auto d1 = _header.orbs[i - 1] / 10;
        buf[0] = d1 ? '0' + d1 :  ' ';
        buf[1] = '0' + _header.orbs[i - 1] % 10;
        buf[2] = 0;
        at.x = 320 - 16 - MAIN_MENU_MARGINS * 2;
        rect_s rect(at, size_s(16, 8));
        screen.draw(background, rect, at);
        screen.draw(mono_font, buf, at, canvas_c::align_left);
    }
}


void cglevel_edit_scene_c::draw_tile_template_at(canvas_c &screen, const tilestate_t &state, int index) const {
    int16_t x = LEVEL_EDIT_TEMPLATE_ORIGIN_X + index * 16;
    point_s at(x, LEVEL_EDIT_TEMPLATE_ORIGIN_Y);
    draw_tilestate(screen, state, at, _selected_template == index);
}

void cglevel_edit_scene_c::draw_tile_templates(canvas_c &screen) const {
    for (int i = 0; i < _tile_templates.size(); i++) {
        draw_tile_template_at(screen, _tile_templates[i], i);
    }
    ((cglevel_edit_scene_c*)this)->_scroller.reset(_template_help_texts[_selected_template]);
    screen.fill(0, rect_s(0, 192, 320, 16));
}

void cglevel_edit_scene_c::draw_level_grid(canvas_c &screen, int x, int y) const {
    point_s at(x * 16, y * 16);
    draw_tilestate(screen, _level_grid[x][y], at);
}

void cglevel_edit_scene_c::make_recipe(level_recipe_t &recipe) const {
    int x1 = 12, x2 = -1;
    int y1 = 12, y2 = -1;
    for (int y = 0; y < 12; y++) {
        for (int x = 0; x < 12; x++) {
            if (_level_grid[x][y].type != empty || _level_grid[x][y].target != none) {
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
