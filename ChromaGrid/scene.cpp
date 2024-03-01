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
    mouse((cgrect_t){{0,0}, {320, 192}}),
    _physical_screen((cgsize_t){320, 208}, false, nullptr),
    _logical_screen((cgsize_t){320, 208}, false, nullptr)
{
    _dirtymap = (bool *)calloc(_physical_screen.dirtymap_size(), 1);
}

cgmanager_c::~cgmanager_c() {
    cgsuper(_super_token);
}

#define DEBUG_COLORS 1
#define DEBUG_NO_SET_SCREEN 0

void cgmanager_c::run(cgscene_c *rootscene) {
#if !DEBUG_NO_SET_SCREEN
    int16_t old_mode = cgset_screen(nullptr, nullptr, 0);
    _physical_screen.set_active();
#endif

    push(rootscene);

#if DEBUG_COLORS
    cgcolor_c color_in_transition(31, 0, 0);
    cgcolor_c color_in_top_tick(0, 191, 0);
    cgcolor_c color_in_restore(0, 00, 95);
    cgcolor_c color_in_root_tick(0, 47, 0);
    
    cgcolor_c color_done(0, 0, 0);
#endif
    vbl.reset_tick();
    while (_scene_count > 0) {
        if (_transition_shade > 0) {
#if DEBUG_COLORS
            color_in_transition.set_at(0);
#endif
            _physical_screen.with_stencil(cgimage_c::get_stencil(_transion_type, cgimage_c::STENCIL_FULLY_OPAQUE - _transition_shade), [&, this] {
                auto old_tick = vbl.tick();
                _physical_screen.draw_aligned(_logical_screen, (cgpoint_t){0, 0});
                auto ticks = MAX(1, vbl.tick() - old_tick);
#if DEBUG_NO_SET_SCREEN
                printf("Transition (%d) %d ticks.\n\r", old_tick, ticks);
#endif
                _transition_shade -= ticks;
            });
        } else {
            if (_scene_count > 1) {
#if DEBUG_COLORS
            color_in_top_tick.set_at(0);
#endif
                _logical_screen.with_dirtymap(_dirtymap, [this] {
                    top_scene().tick(_logical_screen);
                });
            }
#if DEBUG_COLORS
            color_in_restore.set_at(0);
#endif
            if (_full_restore) {
                _physical_screen.draw_aligned(_logical_screen, (cgpoint_t){0, 0});
                _full_restore = false;
            } else {
                _physical_screen.restore(_logical_screen, _dirtymap);
            }
#if DEBUG_COLORS
            color_in_root_tick.set_at(0);
#endif
            _physical_screen.with_dirtymap(_dirtymap, [this] {
                _scene_stack[0]->tick(_physical_screen);
            });
            while (_delete_count-- > 0) {
                delete _deletion_stack[_delete_count];
            }
        }
#if DEBUG_COLORS
            color_done.set_at(0);
#endif
        vbl.wait();
    }
#if !DEBUG_NO_SET_SCREEN
    (void)cgset_screen(nullptr, nullptr, old_mode);
#endif
}

void cgmanager_c::push(cgscene_c *scene, cgimage_c::stencil_type_e transition) {
    if (_scene_count > 0) {
        top_scene().will_disappear(true);
    }
    _scene_stack[_scene_count++] = scene;
    _logical_screen.with_dirtymap(nullptr, [this] {
        top_scene().will_appear(_logical_screen, false);
    });
    _full_restore = true;
    _transion_type = transition;
    _transition_shade = transition != cgimage_c::none ? cgimage_c::STENCIL_FULLY_OPAQUE : 0;
}

void cgmanager_c::pop(cgimage_c::stencil_type_e transition, int count) {
    while (--count > 0) {
        top_scene().will_disappear(false);
        enqueue_delete(&top_scene());
        _scene_count--;
    }
    _logical_screen.with_dirtymap(nullptr, [this, count] {
        top_scene().will_appear(_logical_screen, true);
    });
    _full_restore = true;
    _transion_type = transition;
    _transition_shade = transition != cgimage_c::none ? cgimage_c::STENCIL_FULLY_OPAQUE : 0;
}

void cgmanager_c::replace(cgscene_c *scene, cgimage_c::stencil_type_e transition) {
    top_scene().will_disappear(false);
    enqueue_delete(&top_scene());
    _scene_stack[_scene_count - 1] = scene;
    _logical_screen.with_dirtymap(nullptr, [this] {
        top_scene().will_appear(_logical_screen, false);
    });
    _full_restore = true;
    _transion_type = transition;
    _transition_shade = transition != cgimage_c::none ? cgimage_c::STENCIL_FULLY_OPAQUE : 0;
}

