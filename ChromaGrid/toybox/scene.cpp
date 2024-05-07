//
//  scene.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-03-01.
//

#include "scene.hpp"
#include "machine.hpp"
#include "system_helpers.hpp"

using namespace toybox;

class no_transition_c : public transition_c {
public:
    no_transition_c() : transition_c() {
        _full_restores_left = 2;
    }
    bool tick(screen_c &phys_screen, screen_c &log_screen, int ticks) {
        phys_screen.canvas().draw_aligned(log_screen.image(), point_s());
        return --_full_restores_left <= 0;
    }
private:
    int _full_restores_left;
};

scene_manager_c::scene_manager_c(size_s screen_size) :
    vbl(timer_c::shared(timer_c::vbl)),
    clock(timer_c::shared(timer_c::clock))
{
    machine_c::shared();
    _overlay_scene = nullptr;
    _active_physical_screen = 0;
    _transition = nullptr;
    _screens.emplace_back(screen_size);
    _screens.emplace_back(screen_size);
    _screens.emplace_back(screen_size);
    srand48(time(nullptr));
}

#define DEBUG_NO_SET_SCREEN 0

void scene_manager_c::run(scene_c *rootscene, scene_c *overlayscene, transition_c *transition) {
    set_overlay_scene(overlayscene);
    push(rootscene, transition);

    vbl.reset_tick();
    int32_t previous_tick = vbl.tick();
    while (_scene_stack.size() > 0) {
        vbl.wait();
        int32_t tick = vbl.tick();
        int32_t ticks = tick - previous_tick;
        previous_tick = tick;
        
        screen_c &back = screen(screen_e::back);
        screen_c &clear = screen(screen_e::clear);
        
        if (_transition) {
            debug_cpu_color(DEBUG_CPU_RUN_TRANSITION);
            bool done = _transition->tick(back, clear, ticks);
            if (done) {
                set_transition(nullptr, true);
            }
        } else {
            debug_cpu_color(DEBUG_CPU_TOP_SCENE_TICK);
            auto &scene = top_scene();
            clear.canvas().with_dirtymap(clear.dirtymap(), [&scene, ticks, &clear] {
                scene.update_clear(clear, ticks);
            });
            if (_scene_stack.size() > 0 && scene == top_scene()) {
                // Merge dirty maps here!
                _screens[0].dirtymap()->merge(*clear.dirtymap());
                _screens[1].dirtymap()->merge(*clear.dirtymap());
#if TOYBOX_DEBUG_RESTORE_SCREEN && DEBUG_DIRTYMAP
                logical_screen.dirtymap->debug("log");
                physical_screen.dirtymap->debug("phys");
#endif
                clear.dirtymap()->clear();
                debug_cpu_color(DEBUG_CPU_PHYS_RESTORE);
                back.dirtymap()->restore(back.canvas(), clear.image());
                
                back.canvas().with_dirtymap(back.dirtymap(), [this, &scene, ticks, &back] {
                    debug_cpu_color(DEBUG_CPU_TOP_SCENE_TICK);
                    if (&scene == &top_scene()) {
                        scene.update_back(back, ticks);
                    }
                    if (_overlay_scene) {
                        debug_cpu_color(DEBUG_CPU_OVERLAY_SCENE_TICK);
                        _overlay_scene->update_back(back, ticks);
                    }
                });
#if TOYBOX_DEBUG_RESTORE_SCREEN && DEBUG_DIRTYMAP
                physical_screen.dirtymap->debug("AF next");
#endif
            }
            _deletion_stack.clear();
        }
        debug_cpu_color(DEBUG_CPU_DONE);
        timer_c::with_paused_timers([this, &back] {
            machine_c::shared().set_active_image(&back.image());
            swap_screens();
        });
    }
}

void scene_manager_c::set_overlay_scene(scene_c *overlay_scene) {
    if (_overlay_scene != overlay_scene) {
        if (_overlay_scene) {
            _overlay_scene->will_disappear(false);
        }
        _overlay_scene = overlay_scene;
        auto &clear = screen(screen_e::clear);
        _overlay_scene->will_appear(clear, false);
    }
}

void scene_manager_c::push(scene_c *scene, transition_c *transition) {
    if (_scene_stack.size() > 0) {
        top_scene().will_disappear(true);
    }
    _scene_stack.push_back(scene);
    auto &clear = screen(screen_e::clear);
    clear.canvas().with_dirtymap(nullptr, [this, &clear] {
        top_scene().will_appear(clear, false);
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
        auto &clear = screen(screen_e::clear);
        clear.canvas().with_dirtymap(nullptr, [this, &clear, count] {
            top_scene().will_appear(clear, true);
        });
    }
    set_transition(transition);
}

void scene_manager_c::replace(scene_c *scene, transition_c *transition) {
    top_scene().will_disappear(false);
    enqueue_delete(&top_scene());
    _scene_stack.back() = scene;
    auto &clear = screen(screen_e::clear);
    clear.canvas().with_dirtymap(nullptr, [this, &clear] {
        top_scene().will_appear(clear, false);
    });
    set_transition(transition);
}

screen_c &scene_manager_c::screen(screen_e id) const {
    switch (id) {
        case screen_e::clear:
            return (screen_c&)_screens[2];
        default: {
            int idx = ((int)id + _active_physical_screen) & 0x1;
            return (screen_c&)_screens[idx];
        }
    }
}

void scene_manager_c::swap_screens() {
    _active_physical_screen = (_active_physical_screen + 1) & 0x1;
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
