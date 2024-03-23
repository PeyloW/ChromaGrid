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
#include "vector.hpp"

#ifdef __M68000__
#   define DEBUG_RESTORE_SCREEN 0
#else
#   define DEBUG_RESTORE_SCREEN 1
#endif

namespace toybox {
    
#define DEBUG_CPU_RUN_TRANSITION 0x100
#define DEBUG_CPU_TOP_SCENE_TICK 0x030
#define DEBUG_CPU_PHYS_RESTORE 0x004
#define DEBUG_CPU_OVERLAY_SCENE_TICK 0x010
#define DEBUG_CPU_DONE 0x000
    
    using namespace toystd;
    
    class scene_manager_c;
        
    class scene_c : private nocopy_c {
    public:
        scene_c(scene_manager_c &manager) : manager(manager) {};
        virtual ~scene_c() {};
        
        virtual void will_appear(image_c &screen, bool obsured) = 0;
        virtual void will_disappear(bool obscured) {};
        virtual void tick(image_c &screen, int ticks) = 0;
    protected:
        scene_manager_c &manager;
    };
    
    class transition_c : private nocopy_c {
    public:
        transition_c() {};
        virtual ~transition_c() {}
        
        virtual bool tick(image_c &phys_screen, image_c &log_screen, int ticks) = 0;
        
        static transition_c *create(image_c::stencil_type_e dither, uint8_t through);
        static transition_c *create(color_c through);
    };
        
    class scene_manager_c : private nocopy_c {
    private:
        int32_t _super_token;
    public:
        scene_manager_c();
        ~scene_manager_c();
        
        void run(scene_c *rootscene, scene_c *overlay_scene = nullptr, transition_c *transition = nullptr);
        
        void set_overlay_scene(scene_c *overlay_cene);
        scene_c *overlay_scene() const { return _overlay_scene; };
        
        scene_c &top_scene() const {
            return *_scene_stack.back();
        };
        void push(scene_c *scene, transition_c *transition = transition_c::create(image_c::noise, 0));
        void pop(transition_c *transition  = transition_c::create(image_c::noise, 0), int count = 1);
        void replace(scene_c *scene, transition_c *transition  = transition_c::create(image_c::noise, 0));
        
        timer_c vbl;
        timer_c clock;
        mouse_c mouse;
        
        image_c &get_logical_screen() { return _screens.back().image; }
        
    private:
        transition_c *_transition;
        scene_c *_overlay_scene;
        vector_c<scene_c *, 8> _scene_stack;
        vector_c<scene_c *, 8> _deletion_stack;
        
        inline void enqueue_delete(scene_c *scene) {
            _deletion_stack.push_back(scene);
        }
        inline void set_transition(transition_c *transition, bool done = false);
        
        class screen_c {
        public:
            image_c image;
            dirtymap_c *dirtymap;
            screen_c() : image((size_s){320, 208}, false, nullptr) {
                dirtymap = dirtymap_c::create(image);
            }
        };
        vector_c<screen_c, 3> _screens;
        int _active_physical_screen;
    };
    
}

#endif /* scene_hpp */
