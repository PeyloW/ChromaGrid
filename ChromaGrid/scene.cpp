//
//  scene.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-03-01.
//

#include "scene.hpp"

cgmanager_c::cgmanager_c() :
    _super_token(cgsuper(0)),
    vbl(cgtimer_c::vbl),
    mouse((cgrect_t){{0,0}, {320, 192}})
{
    _overlay_scene = nullptr;
    _active_physical_screen = 0;
    _transition_state.full_restores_left = 0;
    _screens.emplace_back();
    _screens.emplace_back();
    _screens.emplace_back();
}

cgmanager_c::~cgmanager_c() {
    cgsuper(_super_token);
}

#define DEBUG_NO_SET_SCREEN 0

void cgmanager_c::run_transition(screen_t &physical_screen) {
    debug_cpu_color(0x400);
    if (_transition_state.type == cgimage_c::none) {
        physical_screen.image.draw_aligned(get_logical_screen(), (cgpoint_t){0,0});
        _transition_state.full_restores_left--;
    } else {
        auto old_tick = vbl.tick();
        auto shade = MIN(cgimage_c::STENCIL_FULLY_OPAQUE, _transition_state.shade);
        physical_screen.image.with_stencil(cgimage_c::get_stencil(_transition_state.type, shade), [this, &physical_screen] {
            physical_screen.image.draw_aligned(get_logical_screen(), (cgpoint_t){0, 0});
        });
        if (shade == cgimage_c::STENCIL_FULLY_OPAQUE) {
            _transition_state.full_restores_left--;
        }
        auto ticks = MAX(1, vbl.tick() - old_tick);
        _transition_state.shade += ticks;
    }
}

void cgmanager_c::run(cgscene_c *rootscene, cgscene_c *overlayscene, cgimage_c::stencil_type_e transition) {
#if !DEBUG_NO_SET_SCREEN
    int16_t old_mode = cgset_screen(nullptr, nullptr, 0);
#endif

    set_overlay_scene(overlayscene);
    push(rootscene, transition);

    vbl.reset_tick();
    while (_scene_stack.size() > 0) {
        mouse.update_state();
        screen_t &physical_screen = _screens[_active_physical_screen];
        screen_t &logical_screen = _screens.back();
        
        if (_transition_state.full_restores_left > 0) {
            run_transition(physical_screen);
        } else {
            debug_cpu_color(0x050);
            logical_screen.image.with_dirtymap(logical_screen.dirtymap, [this, &logical_screen] {
                top_scene().tick(logical_screen.image);
            });
            debug_cpu_color(0x404);
            // Merge dirty maps here!
            logical_screen.image.merge_dirtymap(_screens[0].dirtymap, logical_screen.dirtymap);
            logical_screen.image.merge_dirtymap(_screens[1].dirtymap, logical_screen.dirtymap);
#if DEBUG_RESTORE_SCREEN
            logical_screen.image.debug_dirtymap(logical_screen.dirtymap, "LOG");
            logical_screen.image.debug_dirtymap(physical_screen.dirtymap, "AF");
#endif
            memset(logical_screen.dirtymap, 0, logical_screen.image.dirtymap_size());
            debug_cpu_color(0x005);
            physical_screen.image.restore(logical_screen.image, physical_screen.dirtymap);

            if (_overlay_scene) {
                debug_cpu_color(0x020);
                physical_screen.image.with_dirtymap(physical_screen.dirtymap, [this, &physical_screen] {
                    _overlay_scene->tick(physical_screen.image);
                });
            }
#if DEBUG_RESTORE_SCREEN
            logical_screen.image.debug_dirtymap(physical_screen.dirtymap, "AF next");
#endif

            for (auto scene = _deletion_stack.begin(); scene != _deletion_stack.end(); scene++) {
                delete *scene;
            }
            _deletion_stack.clear();
        }
        debug_cpu_color(0x000);
        cgtimer_c::with_paused_timers([this, &physical_screen] {
            physical_screen.image.set_active();
            _active_physical_screen = (_active_physical_screen + 1) & 0x1;
        });
        vbl.wait();
    }
#if !DEBUG_NO_SET_SCREEN
    (void)cgset_screen(nullptr, nullptr, old_mode);
#endif
}

void cgmanager_c::set_overlay_scene(cgscene_c *overlay_scene) {
    if (_overlay_scene != overlay_scene) {
        if (_overlay_scene) {
            _overlay_scene->will_disappear(false);
        }
        _overlay_scene = overlay_scene;
        _overlay_scene->will_appear(get_logical_screen(), false);
    }
}

void cgmanager_c::push(cgscene_c *scene, cgimage_c::stencil_type_e transition) {
    if (_scene_stack.size() > 0) {
        top_scene().will_disappear(true);
    }
    _scene_stack.push_back(scene);
    get_logical_screen().with_dirtymap(nullptr, [this] {
        top_scene().will_appear(get_logical_screen(), false);
    });
    enqueue_transition(transition);
}

void cgmanager_c::pop(cgimage_c::stencil_type_e transition, int count) {
    while (count-- > 0) {
        auto &top = top_scene();
        top.will_disappear(false);
        enqueue_delete(&top);
        _scene_stack.pop_back();
    }
    if (_scene_stack.size() > 0) {
        get_logical_screen().with_dirtymap(nullptr, [this, count] {
            top_scene().will_appear(get_logical_screen(), true);
        });
    }
    enqueue_transition(transition);
}

void cgmanager_c::replace(cgscene_c *scene, cgimage_c::stencil_type_e transition) {
    top_scene().will_disappear(false);
    enqueue_delete(&top_scene());
    _scene_stack.back() = scene;
    get_logical_screen().with_dirtymap(nullptr, [this] {
        top_scene().will_appear(get_logical_screen(), false);
    });
    enqueue_transition(transition);
}

