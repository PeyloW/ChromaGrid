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

#ifdef __M68000__
#   define DEBUG_RESTORE_SCREEN 0
#else
#   define DEBUG_RESTORE_SCREEN 1
#endif

class cgmanager_c;

class cgscene_c : private cgnocopy_c {
public:
    cgscene_c(cgmanager_c &manager) : manager(manager) {};
    virtual ~cgscene_c() {};
    
    virtual void will_appear(cgimage_c &screen, bool obsured) = 0;
    virtual void will_disappear(bool obscured) {};
    virtual void tick(cgimage_c &screen, int ticks) = 0;
protected:
    cgmanager_c &manager;
};

class cgmanager_c : private cgnocopy_c {
private:
    int32_t _super_token;
public:
    cgmanager_c();
    ~cgmanager_c();
        
    void run(cgscene_c *rootscene, cgscene_c *overlay_scene = nullptr, cgimage_c::stencil_type_e transition = cgimage_c::noise);

    void set_overlay_scene(cgscene_c *overlay_cene);
    cgscene_c *overlay_scene() const { return _overlay_scene; };

    cgscene_c &top_scene() const {
        return *_scene_stack.back();
    };
    void push(cgscene_c *scene, cgimage_c::stencil_type_e transition = cgimage_c::noise);
    void pop(cgimage_c::stencil_type_e transition = cgimage_c::noise, int count = 1);
    void replace(cgscene_c *scene, cgimage_c::stencil_type_e transition = cgimage_c::noise);

    cgtimer_c vbl;
    cgmouse_c mouse;

    cgimage_c &get_logical_screen() { return _screens.back().image; }
    
private:
#ifdef __M68000__
    __forceinline void debug_cpu_color(uint16_t c) const {
        cgcolor_c(c).set_at(0);
    }
#else
    inline void debug_cpu_color(uint16_t) { }
#endif
    cgscene_c *_overlay_scene;
    cgvector_c<cgscene_c *, 8> _scene_stack;
    cgvector_c<cgscene_c *, 8> _deletion_stack;
    
    inline void enqueue_delete(cgscene_c *scene) {
        _deletion_stack.push_back(scene);
    }

    class screen_t {
    public:
        cgimage_c image;
        bool *dirtymap;
        screen_t() : image((cgsize_t){320, 208}, false, nullptr) {
            dirtymap = (bool *)calloc(1, image.dirtymap_size());
        }
    };
    cgvector_c<screen_t, 3> _screens;
    int _active_physical_screen;

    void run_transition(screen_t &physical_screen, int ticks);
    
    struct {
        int full_restores_left;
        cgimage_c::stencil_type_e type;
        int shade;
    } _transition_state;
    inline void enqueue_transition(cgimage_c::stencil_type_e type) {
        _transition_state.full_restores_left = 2;
        _transition_state.type = type;
        _transition_state.shade = 0;
    }
};

#endif /* scene_hpp */
