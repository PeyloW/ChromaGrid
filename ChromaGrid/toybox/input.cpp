//
//  input.cpp
//  toybox
//
//  Created by Fredrik on 2024-04-24.
//

#include "input.hpp"
#include "machine.hpp"
#include "timer.hpp"

using namespace toybox;

static uint8_t g_prev_mouse_butons;
uint8_t g_mouse_buttons;
static mouse_c::state_e g_mouse_button_states[2];
point_s g_mouse_position;

extern "C" {
#ifdef __M68000__
#   if TOYBOX_TARGET_ATARI
    extern timer_c::func_a_t g_system_mouse_interupt;
    extern void g_mouse_interupt(void *);
    static _KBDVECS *g_keyboard_vectors = nullptr;
#   else
#       error "Usupported target"
#   endif
#else
    // Host must call when mouse state changes
    void g_update_mouse(point_s position, bool left, bool right) {
        g_mouse_position = position;
        g_mouse_buttons = (left ? 2 : 0) | (right ? 1 : 0);
    }
#endif
}

mouse_c &mouse_c::shared() {
    static mouse_c s_shared;
    return s_shared;
}

const rect_s &mouse_c::limits() const {
    return _limits;
}

void mouse_c::set_limits(const rect_s &limits) {
    _limits = limits;
    g_mouse_position = point_s(
        limits.origin.x + _limits.size.width / 2,
        limits.origin.y + _limits.size.height / 2
    );
}

mouse_c::mouse_c() {
    set_limits(rect_s(point_s(), machine_c::shared().screen_size()));
#ifdef __M68000__
#   if TOYBOX_TARGET_ATARI
    g_keyboard_vectors = Kbdvbase();
    g_system_mouse_interupt = g_keyboard_vectors->mousevec;
    g_keyboard_vectors->mousevec = &g_mouse_interupt;
#   else
#       error "Unsupported target"
#   endif
#endif
}

mouse_c::~mouse_c() {
#ifdef __M68000__
#   if TOYBOX_TARGET_ATARI
    g_keyboard_vectors->mousevec = g_system_mouse_interupt;
#   else
#       error "Unsupported target"
#   endif
#endif
}

static void update_state() {
    for (int button = 2; --button != -1; ) {
        if (g_mouse_buttons & (1 << button)) {
            g_mouse_button_states[button] = toybox::mouse_c::pressed;
        } else if (g_prev_mouse_butons & (1 << button)) {
            g_mouse_button_states[button] = toybox::mouse_c::clicked;
        } else {
            g_mouse_button_states[button] = toybox::mouse_c::released;
        }
    }
    g_prev_mouse_butons = g_mouse_buttons;
}

bool mouse_c::is_pressed(button_e button) const {
    auto tick = timer_c::shared(timer_c::vbl).tick();
    if (tick > _update_tick) {
        update_state();
        _update_tick = tick;
    }
    return (g_mouse_buttons & (1 << button)) != 0;
}

mouse_c::state_e mouse_c::state(button_e button) const {
    update_state();
    return g_mouse_button_states[button];
}

point_s mouse_c::postion() {
    point_s clamped_point = point_s(
        MIN(_limits.max_x(), MAX(g_mouse_position.x, _limits.origin.x)),
        MIN(_limits.max_y(), MAX(g_mouse_position.y, _limits.origin.y))
    );
    g_mouse_position = clamped_point;
    return clamped_point;
}


