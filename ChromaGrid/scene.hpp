//
//  scene.hpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-03-01.
//

#ifndef scene_hpp
#define scene_hpp

#include "system.hpp"
#include "graphics.hpp"

class cgmanager_c;

class cgscene_c : private cgnocopy_c {
public:
    cgscene_c(cgmanager_c &manager) : manager(manager) {};
    virtual ~cgscene_c() {};
    
    virtual void will_appear(cgimage_c &screen, bool obsured) = 0;
    virtual void will_disappear(bool obscured) {};
    virtual void tick(cgimage_c &screen) = 0;
protected:
    cgmanager_c &manager;
};

class cgmanager_c : private cgnocopy_c {
private:
    int32_t _super_token;
public:
    cgmanager_c();
    ~cgmanager_c();
    
    void run(cgscene_c *rootscene);

    cgscene_c &top_scene() const {
        assert(_scene_count > 0);
        return *_scene_stack[_scene_count - 1];
    };
    void push(cgscene_c *scene, cgimage_c::stencil_type_e transition = cgimage_c::noise);
    void pop(cgimage_c::stencil_type_e transition = cgimage_c::noise, int count = 1);
    void replace(cgscene_c *scene, cgimage_c::stencil_type_e transition = cgimage_c::noise);

    cgtimer_c vbl;
    cgmouse_c mouse;

    cgimage_c &get_logical_screen() { return _logical_screen; }
    
private:
    inline void enqueue_delete(cgscene_c *scene) {
        _deletion_stack[_delete_count++] = scene;
    }
    int _scene_count;
    cgscene_c *_scene_stack[8];
    int _delete_count;
    cgscene_c *_deletion_stack[8];

    cgimage_c _physical_screen;
    cgimage_c _logical_screen;
    bool *_dirtymap;
    bool _full_restore;
    cgimage_c::stencil_type_e _transion_type;
    int _transition_shade;
};

#endif /* scene_hpp */
