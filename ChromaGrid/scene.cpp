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
    mouse((cgrect_t){{0,0}, {320, 200}}),
    _physical_screen((cgsize_t){320, 208}, false, nullptr),
    _logical_screen((cgsize_t){320, 208}, false, nullptr)
{
    _dirtymap = (bool *)calloc(_physical_screen.dirtymap_size(), 1);
}

cgmanager_c::~cgmanager_c() {
    cgsuper(_super_token);
}

void cgmanager_c::run(cgscene_c *rootscene) {
    int16_t old_mode = cgset_screen(nullptr, nullptr, 0);
    _physical_screen.set_active();

    push(rootscene);

    cgcolor_c blue(0, 0, 95);
    vbl.reset_tick();
    while (_scene_count > 0) {
        if (_scene_count > 1) {
            _logical_screen.with_dirtymap(_dirtymap, [this] {
                top_scene().tick(_logical_screen);
            });
        }
        if (_full_restore) {
            _physical_screen.draw(_logical_screen, (cgpoint_t){0, 0});
        } else {
            _physical_screen.restore(_logical_screen, _dirtymap);
        }
        _physical_screen.with_dirtymap(_dirtymap, [this] {
            _scene_stack[0]->tick(_physical_screen);
        });
        while (_delete_count-- > 0) {
            delete _deletion_stack[_delete_count];
        }
        vbl.wait();
    }
    (void)cgset_screen(nullptr, nullptr, old_mode);
}

void cgmanager_c::push(cgscene_c *scene) {
    if (_scene_count > 0) {
        top_scene().will_disappear(true);
    }
    _scene_stack[_scene_count++] = scene;
    _logical_screen.with_dirtymap(nullptr, [this] {
        top_scene().will_appear(_logical_screen, false);
    });
    _full_restore = true;
}

void cgmanager_c::pop(int count) {
    while (--count > 0) {
        top_scene().will_disappear(false);
        enqueue_delete(&top_scene());
        _scene_count--;
    }
    _logical_screen.with_dirtymap(nullptr, [this, count] {
        top_scene().will_appear(_logical_screen, true);
    });
    _full_restore = true;
}

void cgmanager_c::replace(cgscene_c *scene) {
    top_scene().will_disappear(false);
    enqueue_delete(&top_scene());
    _scene_stack[_scene_count - 1] = scene;
    _logical_screen.with_dirtymap(nullptr, [this] {
        top_scene().will_appear(_logical_screen, false);
    });
    _full_restore = true;
}

