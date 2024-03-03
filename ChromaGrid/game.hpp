//
//  game.hpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-03.
//

#ifndef game_hpp
#define game_hpp

#include "grid.hpp"
#include "scene.hpp"
#include "resources.hpp"
#include "button.hpp"

class cggame_scene_c : public cgscene_c {
public:
    cggame_scene_c(cgmanager_c &manager) :
        cgscene_c(manager),
        rsc(cgresources_c::shared()) {};
    const cgresources_c &rsc;
};

class cgoverlay_scene_c : public cggame_scene_c {
public:
    cgoverlay_scene_c(cgmanager_c &manager) : cggame_scene_c(manager) {}
    virtual void will_appear(cgimage_c &screen, bool obsured);
    virtual void will_disappear(bool obscured);
    virtual void tick(cgimage_c &screen);
};

class cgintro_scene_c : public cggame_scene_c {
public:
    cgintro_scene_c(cgmanager_c &manager);
    virtual void will_appear(cgimage_c &screen, bool obsured);
    virtual void tick(cgimage_c &screen);
private:
    cgbutton_group_c<6> _menu_buttons;
};

class cgcredits_scene_c : public cggame_scene_c {
public:
    cgcredits_scene_c(cgmanager_c &manager);
    virtual void will_appear(cgimage_c &screen, bool obsured);
    virtual void tick(cgimage_c &screen);
private:
    cgbutton_group_c<1> _menu_buttons;
};

#endif /* game_hpp */
