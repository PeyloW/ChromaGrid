//
//  scene.hpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-03-01.
//

#ifndef scene_hpp
#define scene_hpp

#include "system.hpp"
#include "screen.hpp"
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
        
    class scene_c : public nocopy_c {
    public:
        scene_c(scene_manager_c &manager) : manager(manager) {};
        virtual ~scene_c() {};
        
        virtual void will_appear(screen_c &screen, bool obsured) = 0;
        virtual void will_disappear(bool obscured) {};
        virtual void update_background(screen_c &screen, int ticks) {};
        virtual void update_foreground(screen_c &screen, int ticks) {};
        
    protected:
        scene_manager_c &manager;
    };
    
    class transition_c : public nocopy_c {
    public:
        transition_c() {};
        virtual ~transition_c() {}
        
        virtual bool tick(screen_c &phys_screen, screen_c &log_screen, int ticks) = 0;
        
        static transition_c *create(canvas_c::stencil_type_e dither);
        static transition_c *create(canvas_c::stencil_type_e dither, uint8_t through);
        static transition_c *create(color_c through);
    };
        
    class scene_manager_c : public nocopy_c {
    private:
        int32_t _super_token;
        int16_t _old_blitter_mode;
        uint8_t _old_conterm;
    public:
        scene_manager_c(size_s screen_size = size_s(320, 200));
        ~scene_manager_c();
        
        void run(scene_c *rootscene, scene_c *overlay_scene = nullptr, transition_c *transition = nullptr);
        
        void set_overlay_scene(scene_c *overlay_cene);
        scene_c *overlay_scene() const { return _overlay_scene; };
        
        scene_c &top_scene() const {
            return *_scene_stack.back();
        };
        void push(scene_c *scene, transition_c *transition = transition_c::create(canvas_c::random, 0));
        void pop(transition_c *transition  = transition_c::create(canvas_c::random, 0), int count = 1);
        void replace(scene_c *scene, transition_c *transition = transition_c::create(canvas_c::random));
        
        timer_c vbl;
        timer_c clock;
        mouse_c mouse;
        
        screen_c &background_screen() { return _screens.back(); }
        
    private:
        transition_c *_transition;
        scene_c *_overlay_scene;
        vector_c<scene_c *, 8> _scene_stack;
        vector_c<scene_c *, 8> _deletion_stack;
        
        inline void enqueue_delete(scene_c *scene) {
            _deletion_stack.push_back(scene);
        }
        inline void set_transition(transition_c *transition, bool done = false);
        
        vector_c<screen_c, 3> _screens;
        int _active_physical_screen;
    };
    
}

#endif /* scene_hpp */
