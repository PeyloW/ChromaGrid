//
//  game.hpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-03.
//

#ifndef game_hpp
#define game_hpp

#include "level.hpp"
#include "scene.hpp"
#include "resources.hpp"
#include "button.hpp"

class cggame_scene_c : public scene_c {
public:
#define MAIN_MENU_ORIGIN_X (192)
#define MAIN_MENU_SIZE_WIDTH (128)
#define MAIN_MENU_SIZE_HEIGHT (192)
#define MAIN_MENU_MARGINS (8)
#define MAIN_MENU_BUTTONS_ORIGIN ((point_s){MAIN_MENU_ORIGIN_X + MAIN_MENU_MARGINS, MAIN_MENU_SIZE_HEIGHT - MAIN_MENU_MARGINS})
#define MAIN_MENU_BUTTONS_SIZE ((size_s){MAIN_MENU_SIZE_WIDTH - MAIN_MENU_MARGINS * 2, 14})
#define MAIN_MENU_BUTTONS_SPACING ((int16_t)-6)
    cggame_scene_c(scene_manager_c &manager) :
        scene_c(manager),
        rsc(cgresources_c::shared()) {};
    const cgresources_c &rsc;
    template<class BG>
    int update_button_group(image_c &screen, BG &buttons) const {
        return buttons.update_buttons(screen, manager.mouse.get_postion(), manager.mouse.get_state(mouse_c::left));
    }
};

class cgoverlay_scene_c : public cggame_scene_c {
public:
    cgoverlay_scene_c(scene_manager_c &manager) : cggame_scene_c(manager) {}
    virtual void will_appear(image_c &screen, bool obsured);
    virtual void will_disappear(bool obscured);
    virtual void tick(image_c &screen, int ticks);
};

class cgintro_scene_c : public cggame_scene_c {
public:
    cgintro_scene_c(scene_manager_c &manager);
    virtual void will_appear(image_c &screen, bool obsured);
    virtual void tick(image_c &screen, int ticks);
private:
    cgbutton_group_c<6> _menu_buttons;
};

class cgcredits_scene_c : public cggame_scene_c {
public:
    typedef enum __packed {
        credits, recognitions, dedications, greetings
    } page_e;
    cgcredits_scene_c(scene_manager_c &manager, page_e page = credits);
    virtual void will_appear(image_c &screen, bool obsured);
    virtual void tick(image_c &screen, int ticks);
private:
    page_e _page;
    cgbutton_group_c<5> _menu_buttons;
};

class cgscores_scene_c : public cggame_scene_c {
public:
    typedef enum __packed {
        score, time, moves
    } scoring_e;
    cgscores_scene_c(scene_manager_c &manager, scoring_e scoring = score);
    virtual void will_appear(image_c &screen, bool obsured);
    virtual void tick(image_c &screen, int ticks);
private:
    scoring_e _scoring;
    cgbutton_group_c<4> _menu_buttons;
};

class cglevel_scene_c : public cggame_scene_c {
    friend void tick_second(cglevel_scene_c *that);
public:
    static const int TEST_LEVEL = -1;
    cglevel_scene_c(scene_manager_c &manager, int level);
    cglevel_scene_c(scene_manager_c &manager, level_recipe_t *recipe);

    virtual void will_appear(image_c &screen, bool obsured);
    virtual void will_disappear(bool obscured);
    virtual void tick(image_c &screen, int ticks);
private:
    int _passed_seconds;
    cgbutton_group_c<2> _menu_buttons;
    int _level_num;
    level_recipe_t *_recipe;
    level_t _level;
};

class cglevel_select_scene_c : public cggame_scene_c {
public:
    cglevel_select_scene_c(scene_manager_c &manager);

    virtual void will_appear(image_c &screen, bool obsured);
    virtual void tick(image_c &screen, int ticks);
private:
    cgbutton_group_c<1> _menu_buttons;
    vector_c<cgbutton_group_c<5>, 9> _select_button_groups;
};

class cglevel_edit_scene_c : public cggame_scene_c {
public:
    cglevel_edit_scene_c(scene_manager_c &manager, level_recipe_t *recipe);
    
    virtual void will_appear(image_c &screen, bool obsured);
    virtual void tick(image_c &screen, int ticks);
private:
    tilestate_t next_state(const tilestate_t &current, mouse_c::button_e button) const;
    void draw_counts(image_c &screen) const;
    void draw_tile_template_at(image_c &screen, tilestate_t &state, int index) const;
    void draw_tile_templates(image_c &screen) const;
    void draw_level_grid(image_c &screen, int x, int y) const;
    
    void make_recipe(level_recipe_t &recipe) const;
    
    cgbutton_group_c<4> _menu_buttons;
    cgbutton_group_c<6> _count_buttons;
    vector_c<tilestate_t, 6> _tile_templates;
    int _selected_template;
    level_recipe_t::header_t _header;
    tilestate_t _level_grid[12][12];
};

#endif /* game_hpp */
