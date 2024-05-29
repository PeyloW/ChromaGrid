//
//  scene.hpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-03-01.
//

#ifndef scene_hpp
#define scene_hpp

#include "timer.hpp"
#include "input.hpp"
#include "screen.hpp"
#include "vector.hpp"

namespace toybox {
    
#define DEBUG_CPU_RUN_TRANSITION 0x100
#define DEBUG_CPU_TOP_SCENE_TICK 0x030
#define DEBUG_CPU_PHYS_RESTORE 0x004
#define DEBUG_CPU_OVERLAY_SCENE_TICK 0x010
#define DEBUG_CPU_DONE 0x000
    
    using namespace toybox;
    
    class scene_manager_c;
        
    class scene_c : public nocopy_c {
    public:
        struct configuration_s : public nocopy_c {
            configuration_s(const palette_c &palette, int buffer_count = 2, bool use_clear = true) :
                palette(palette), buffer_count(buffer_count), use_clear(use_clear) {}
            const palette_c &palette;
            const int buffer_count;
            const bool use_clear;
        };
        scene_c(scene_manager_c &manager) : manager(manager) {};
        virtual ~scene_c() {};
        
        virtual configuration_s &configuration() const = 0;
        
        virtual void will_appear(screen_c &clear_screen, bool obsured) = 0;
        virtual void will_disappear(bool obscured) {};
        virtual void update_clear(screen_c &clear_screen, int ticks) {};
        virtual void update_back(screen_c &back_screen, int ticks) {};
        
    protected:
        scene_manager_c &manager;
    };
    
    class transition_c : public nocopy_c {
    public:
        transition_c() {};
        virtual ~transition_c() {}
        
        virtual void will_begin(const scene_c *from, const scene_c *to) = 0;
        virtual bool tick(screen_c &phys_screen, screen_c &log_screen, int ticks) = 0;
        
        static transition_c *create(canvas_c::stencil_type_e dither);
        static transition_c *create(canvas_c::stencil_type_e dither, uint8_t through);
        static transition_c *create(color_c through);
    };
        
    class scene_manager_c : public nocopy_c {
    public:
        enum class screen_e : int8_t {
            clear = -1, front, back
        };
        scene_manager_c(size_s screen_size = TOYBOX_SCREEN_SIZE_MAX);
        ~scene_manager_c() = default;
        
        void run(scene_c *rootscene, scene_c *overlay_scene = nullptr, transition_c *transition = nullptr);
        
        void set_overlay_scene(scene_c *overlay_cene);
        scene_c *overlay_scene() const { return _overlay_scene; };
        
        scene_c &top_scene() const {
            return *_scene_stack.back();
        };
        void push(scene_c *scene, transition_c *transition = transition_c::create(canvas_c::random, 0));
        void pop(transition_c *transition  = transition_c::create(canvas_c::random, 0), int count = 1);
        void replace(scene_c *scene, transition_c *transition = transition_c::create(canvas_c::random));
        
        timer_c &vbl;
        timer_c &clock;
        
        screen_c &screen(screen_e id) const;
        
    private:
        transition_c *_transition;
        scene_c *_overlay_scene;
        vector_c<scene_c *, 8> _scene_stack;
        vector_c<unique_ptr_c<scene_c>, 8> _deletion_stack;
        
        void swap_screens();
        
        inline void enqueue_delete(scene_c *scene) {
            _deletion_stack.emplace_back(scene);
        }
        inline void begin_transition(transition_c *transition, const scene_c *from, const scene_c *to);
        inline void end_transition();

        vector_c<screen_c, 3> _screens;
        int _active_physical_screen;
    };
    
}

#endif /* scene_hpp */
