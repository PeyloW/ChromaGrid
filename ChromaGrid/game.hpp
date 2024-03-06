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

class cggame_scene_c : public cgscene_c {
public:
    cggame_scene_c(cgmanager_c &manager) :
        cgscene_c(manager),
        rsc(cgresources_c::shared()) {};
    const cgresources_c &rsc;
    template<class BG>
    int update_button_group(cgimage_c &screen, BG &buttons) const {
        return buttons.update_buttons(screen, manager.mouse.get_postion(), manager.mouse.get_state(cgmouse_c::left));
    }
};

class cgoverlay_scene_c : public cggame_scene_c {
public:
    cgoverlay_scene_c(cgmanager_c &manager) : cggame_scene_c(manager) {}
    virtual void will_appear(cgimage_c &screen, bool obsured);
    virtual void will_disappear(bool obscured);
    virtual void tick(cgimage_c &screen, int ticks);
};

class cgintro_scene_c : public cggame_scene_c {
public:
    cgintro_scene_c(cgmanager_c &manager);
    virtual void will_appear(cgimage_c &screen, bool obsured);
    virtual void tick(cgimage_c &screen, int ticks);
private:
    cgbutton_group_c<6> _menu_buttons;
};

class cgcredits_scene_c : public cggame_scene_c {
public:
    cgcredits_scene_c(cgmanager_c &manager);
    virtual void will_appear(cgimage_c &screen, bool obsured);
    virtual void tick(cgimage_c &screen, int ticks);
private:
    cgbutton_group_c<1> _menu_buttons;
};

class cglevel_scene_c : public cggame_scene_c {
public:
    cglevel_scene_c(cgmanager_c &manager, int level);

    virtual void will_appear(cgimage_c &screen, bool obsured);
    virtual void tick(cgimage_c &screen, int ticks);
private:
    cgbutton_group_c<2> _menu_buttons;
    int _level_num;
    level_t _level;
};

class cglevel_ended_scene_c : public cggame_scene_c {
public:
    typedef struct {
        int level_num;
        level_t::state_e state;
        uint8_t orbs_left;
        uint8_t time_left;
    } level_state_t;
    
    cglevel_ended_scene_c(cgmanager_c &manager, level_state_t &state);

    virtual void will_appear(cgimage_c &screen, bool obsured);
    virtual void tick(cgimage_c &screen, int ticks);
private:
    cgbutton_group_c<2> _menu_buttons;
    level_state_t _state;
};

#endif /* game_hpp */
