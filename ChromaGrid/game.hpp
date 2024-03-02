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

struct cgbutton_t : cgnocopy_c {
    typedef enum __packed {
        normal,
        pressed,
        disabled
    } state_t;
    inline cgbutton_t() : text("") {}
    inline cgbutton_t(const char *text, cgrect_t rect) : text(text), rect(rect), state(normal) {}

    void draw_in(cgimage_c &image) const;
    
    const char *text;
    cgrect_t rect;
    state_t state;
};

class cggame_scene_c : public cgscene_c {
public:
    cggame_scene_c(cgmanager_c &manager) :
        cgscene_c(manager),
        rsc(cgresources_c::shared()) {};
    const cgresources_c &rsc;
};

class cgroot_scene_c : public cggame_scene_c {
public:
    cgroot_scene_c(cgmanager_c &manager) : cggame_scene_c(manager) {}
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
    cgvector_c<cgbutton_t, 6> _buttons;
};


#endif /* game_hpp */
