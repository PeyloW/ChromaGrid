//
//  scene.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-03-01.
//

#include "scene.hpp"

using namespace toybox;

class no_transition_c : public transition_c {
public:
    no_transition_c() : transition_c() {
        _full_restores_left = 2;
    }
    bool tick(canvas_c &phys_screen, canvas_c &log_screen, int ticks) {
        phys_screen.draw_aligned(log_screen.get_image(), (point_s){0,0});
        return --_full_restores_left <= 0;
    }
private:
    int _full_restores_left;
};

scene_manager_c::scene_manager_c() :
    _super_token(super(0)),
    _old_blitter_mode(blitter_mode(-1)),
    vbl(timer_c::vbl),
    clock(timer_c::clock),
    mouse((rect_s){{0,0}, {320, 192}})
{
#ifdef __M68000__
    blitter_mode(0);
    _old_conterm = *(uint8_t*)0x484;
    *(uint8_t*)0x484 = 0;
#endif
    _overlay_scene = nullptr;
    _active_physical_screen = 0;
    _transition = nullptr;
    _screens.emplace_back();
    _screens.emplace_back();
    _screens.emplace_back();
    srand48(time(nullptr));
}

scene_manager_c::~scene_manager_c() {
#ifdef __M68000__
    *(uint8_t*)0x484 = _old_conterm;
#endif
    blitter_mode(_old_blitter_mode);
    super(_super_token);
}

#define DEBUG_NO_SET_SCREEN 0

void scene_manager_c::run(scene_c *rootscene, scene_c *overlayscene, transition_c *transition) {
#if !DEBUG_NO_SET_SCREEN
    int16_t old_mode = set_screen(nullptr, nullptr, 0);
#endif

    set_overlay_scene(overlayscene);
    push(rootscene, transition);

    vbl.reset_tick();
    int32_t previous_tick = vbl.tick();
    while (_scene_stack.size() > 0) {
        vbl.wait();
        int32_t tick = vbl.tick();
        int32_t ticks = tick - previous_tick;
        previous_tick = tick;
        
        mouse.update_state();
        screen_c &physical_screen = _screens[_active_physical_screen];
        screen_c &logical_screen = _screens.back();
        
        if (_transition) {
            debug_cpu_color(DEBUG_CPU_RUN_TRANSITION);
            bool done = _transition->tick(physical_screen.canvas, get_background_screen(), ticks);
            if (done) {
                set_transition(nullptr, true);
            }
        } else {
            debug_cpu_color(DEBUG_CPU_TOP_SCENE_TICK);
            auto &scene = top_scene();
            logical_screen.canvas.with_dirtymap(logical_screen.dirtymap, [&scene, ticks, &logical_screen] {
                scene.update_background(logical_screen.canvas, ticks);
            });
            if (scene == top_scene()) {
                // Merge dirty maps here!
                _screens[0].dirtymap->merge(*logical_screen.dirtymap);
                _screens[1].dirtymap->merge(*logical_screen.dirtymap);
#if DEBUG_RESTORE_SCREEN && DEBUG_DIRTYMAP
                logical_screen.dirtymap->debug("log");
                physical_screen.dirtymap->debug("phys");
#endif
                logical_screen.dirtymap->clear();
                debug_cpu_color(DEBUG_CPU_PHYS_RESTORE);
                physical_screen.dirtymap->restore(physical_screen.canvas, logical_screen.image);
                
                physical_screen.canvas.with_dirtymap(physical_screen.dirtymap, [this, &scene, ticks, &physical_screen] {
                    debug_cpu_color(DEBUG_CPU_TOP_SCENE_TICK);
                    if (&scene == &top_scene()) {
                        scene.update_foreground(physical_screen.canvas, ticks);
                    }
                    if (_overlay_scene) {
                        debug_cpu_color(DEBUG_CPU_OVERLAY_SCENE_TICK);
                        _overlay_scene->update_foreground(physical_screen.canvas, ticks);
                    }
                });
#if DEBUG_RESTORE_SCREEN && DEBUG_DIRTYMAP
                physical_screen.dirtymap->debug("AF next");
#endif
            }

            for (auto scene = _deletion_stack.begin(); scene != _deletion_stack.end(); scene++) {
                delete *scene;
            }
            _deletion_stack.clear();
        }
        debug_cpu_color(DEBUG_CPU_DONE);
        timer_c::with_paused_timers([this, &physical_screen] {
            physical_screen.canvas.get_image().set_active();
            _active_physical_screen = (_active_physical_screen + 1) & 0x1;
        });
    }
#if !DEBUG_NO_SET_SCREEN
    (void)set_screen(nullptr, nullptr, old_mode);
#endif
}

void scene_manager_c::set_overlay_scene(scene_c *overlay_scene) {
    if (_overlay_scene != overlay_scene) {
        if (_overlay_scene) {
            _overlay_scene->will_disappear(false);
        }
        _overlay_scene = overlay_scene;
        _overlay_scene->will_appear(get_background_screen(), false);
    }
}

void scene_manager_c::push(scene_c *scene, transition_c *transition) {
    if (_scene_stack.size() > 0) {
        top_scene().will_disappear(true);
    }
    _scene_stack.push_back(scene);
    get_background_screen().with_dirtymap(nullptr, [this] {
        top_scene().will_appear(get_background_screen(), false);
    });
    set_transition(transition);
}

void scene_manager_c::pop(transition_c *transition, int count) {
    while (count-- > 0) {
        auto &top = top_scene();
        top.will_disappear(false);
        enqueue_delete(&top);
        _scene_stack.pop_back();
    }
    if (_scene_stack.size() > 0) {
        get_background_screen().with_dirtymap(nullptr, [this, count] {
            top_scene().will_appear(get_background_screen(), true);
        });
    }
    set_transition(transition);
}

void scene_manager_c::replace(scene_c *scene, transition_c *transition) {
    top_scene().will_disappear(false);
    enqueue_delete(&top_scene());
    _scene_stack.back() = scene;
    get_background_screen().with_dirtymap(nullptr, [this] {
        top_scene().will_appear(get_background_screen(), false);
    });
    set_transition(transition);
}

void scene_manager_c::set_transition(transition_c *transition, bool done) {
    if (_transition) delete _transition;
    if (transition) {
        _transition = transition;
    } else if (done) {
        _transition = nullptr;
    } else {
        _transition = new no_transition_c();
    }
}
