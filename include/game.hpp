//
//  game.hpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-03.
//

#pragma once

#include "level.hpp"
#include "scene.hpp"
#include "resources.hpp"
#include "button.hpp"
#include "scroller.hpp"

class cgintro_scene_c : public scene_c {
public:
    cgintro_scene_c(scene_manager_c &manager);
    virtual configuration_s &configuration() const;
    virtual void will_appear(screen_c &clear_screen, bool obsured);
    virtual void update_clear(screen_c &clear_screen, int ticks);
private:
    static void update_preload(int loaded, int count);
    int _update_count;
    cgbutton_group_c<1> _menu_buttons;
};

class cggame_scene_c : public scene_c {
public:
#define MAIN_MENU_ORIGIN_X (192)
#define MAIN_MENU_SIZE_WIDTH (128)
#define MAIN_MENU_SIZE_HEIGHT (192)
#define MAIN_MENU_MARGINS (8)
#define MAIN_MENU_BUTTONS_ORIGIN (point_s(MAIN_MENU_ORIGIN_X + MAIN_MENU_MARGINS, MAIN_MENU_SIZE_HEIGHT - 4))
#define MAIN_MENU_BUTTONS_SIZE (size_s(MAIN_MENU_SIZE_WIDTH - MAIN_MENU_MARGINS * 2, 14))
#define MAIN_MENU_BUTTONS_SPACING ((int16_t)-6)
    cggame_scene_c(scene_manager_c &manager);
    virtual configuration_s &configuration() const;
    template<class BG>
    int update_button_group(canvas_c &screen, BG &buttons) const {
        return buttons.update_buttons(screen, mouse);
    }
protected:
    mouse_c &mouse;
    cgasset_manager &assets;
    image_c &background;
    font_c &font;
    font_c &small_font;
};

class cgoverlay_scene_c : public cggame_scene_c {
public:
    cgoverlay_scene_c(scene_manager_c &manager) : 
        cggame_scene_c(manager)
    {}
    virtual configuration_s &configuration() const;
    virtual void will_appear(screen_c &clear_screen, bool obsured) {};
    virtual void update_back(screen_c &back_screen, int ticks);
};

class cgerror_scene_c : public cggame_scene_c {
public:
    enum class choice_e : uint8_t {
        retry, cancel
    };
    using choice_f = void(scene_c::*)(choice_e choice);
    cgerror_scene_c(scene_manager_c &manager, const char *title, const char *text, choice_f callback, scene_c &target);

    virtual void will_appear(screen_c &clear_screen, bool obsured);
    virtual void update_back(screen_c &back_screen, int ticks);
private:
    const char *_title;
    const char *_text;
    cgbutton_group_c<2> _buttons;
    const choice_f _callback;
    scene_c &_target;
};

class cgmenu_scene_c : public cggame_scene_c {
public:
    cgmenu_scene_c(scene_manager_c &manager);
    virtual void will_appear(screen_c &clear_screen, bool obsured);
    virtual void update_clear(screen_c &clear_screen, int ticks);
private:
    cgbutton_group_c<5> _menu_buttons;
    scroller_c _scroller;
};

class cgcredits_scene_c : public cggame_scene_c {
public:
    enum class page_e : uint8_t {
        credits, recognitions, dedications, greetings
    };
    cgcredits_scene_c(scene_manager_c &manager, page_e page = page_e::credits);
    virtual void will_appear(screen_c &clear_screen, bool obsured);
    virtual void update_clear(screen_c &clear_screen, int ticks);
private:
    page_e _page;
    cgbutton_group_c<5> _menu_buttons;
};

class cghelp_scene_c : public cggame_scene_c {
public:
    enum class page_e : uint8_t {
        basics, special_tiles, scoring, level_editor
    };
    cghelp_scene_c(scene_manager_c &manager, page_e page = page_e::basics);
    virtual void will_appear(screen_c &clear_screen, bool obsured);
    virtual void update_clear(screen_c &clear_screen, int ticks);
private:
    page_e _page;
    cgbutton_group_c<5> _menu_buttons;
};

class cgscores_scene_c : public cggame_scene_c {
public:
    enum class scoring_e : uint8_t {
        score, time, moves
    };
    cgscores_scene_c(scene_manager_c &manager, scoring_e scoring = scoring_e::score);
    virtual void will_appear(screen_c &clear_screen, bool obsured);
    virtual void update_clear(screen_c &clear_screen, int ticks);
private:
    scoring_e _scoring;
    cgbutton_group_c<4> _menu_buttons;
};

class cglevel_scene_c : public cggame_scene_c {
    friend void tick_second(cglevel_scene_c *that);
public:
    static constexpr int TEST_LEVEL = -1;
    cglevel_scene_c(scene_manager_c &manager, int level);
    cglevel_scene_c(scene_manager_c &manager, level_recipe_t *recipe);

    virtual void will_appear(screen_c &clear_screen, bool obsured);
    virtual void will_disappear(bool obscured);
    virtual void update_clear(screen_c &clear_screen, int ticks);
    virtual void update_back(screen_c &back_screen, int ticks);
private:
    int _shimmer_ticks;
    int _shimmer_tile;
    int _passed_seconds;
    cgbutton_group_c<2> _menu_buttons;
    int _level_num;
    level_recipe_t *_recipe;
    level_t _level;
};

class cglevel_select_scene_c : public cggame_scene_c {
public:
    cglevel_select_scene_c(scene_manager_c &manager);

    virtual void will_appear(screen_c &clear_screen, bool obsured);
    virtual void update_clear(screen_c &clear_screen, int ticks);
private:
    cgbutton_group_c<1> _menu_buttons;
    vector_c<cgbutton_group_c<5>, 9> _select_button_groups;
};

class cglevel_edit_scene_c : public cggame_scene_c {
public:
    cglevel_edit_scene_c(scene_manager_c &manager, level_recipe_t *recipe);
    
    virtual void will_appear(screen_c &clear_screen, bool obsured);
    virtual void update_clear(screen_c &clear_screen, int ticks);
    virtual void update_back(screen_c &back_screen, int ticks);

private:
    tilestate_t next_state(const tilestate_t &current, mouse_c::button_e button) const;
    void draw_counts(canvas_c &screen) const;
    void draw_tile_template_at(canvas_c &screen, const tilestate_t &state, int index) const;
    void draw_tile_templates(canvas_c &screen) const;
    void draw_level_grid(canvas_c &screen, int x, int y) const;
    
    void make_recipe(level_recipe_t &recipe) const;
    
    cgbutton_group_c<4> _menu_buttons;
    cgbutton_group_c<6> _count_buttons;
    vector_c<tilestate_t, 7> _tile_templates;
    int _selected_template;
    level_recipe_t::header_t _header;
    tilestate_t _level_grid[12][12];
    scroller_c _scroller;
    int _shimmer_ticks;
};
#pragma once
