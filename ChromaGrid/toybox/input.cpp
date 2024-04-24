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

static rect_s g_mouse_limit;
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

mouse_c::mouse_c(rect_s limit) {
    machine_c::shared();
    g_mouse_limit = limit;
    g_mouse_position = point_s(
        limit.origin.x + limit.size.width / 2,
        limit.origin.y + limit.size.height / 2
    );
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

void mouse_c::update_state() {
    for (int button = 2; --button != -1; ) {
        if (g_mouse_buttons & (1 << button)) {
            g_mouse_button_states[button] = pressed;
        } else if (g_prev_mouse_butons & (1 << button)) {
            g_mouse_button_states[button] = clicked;
        } else {
            g_mouse_button_states[button] = released;
        }
    }
    g_prev_mouse_butons = g_mouse_buttons;
}

bool mouse_c::is_pressed(button_e button) const {
    return (g_mouse_buttons & (1 << button)) != 0;
}

mouse_c::state_e mouse_c::state(button_e button) const {
    return g_mouse_button_states[button];
}

point_s mouse_c::postion() {
    point_s clamped_point = point_s(
        MIN(g_mouse_limit.max_x(), MAX(g_mouse_position.x, g_mouse_limit.origin.x)),
        MIN(g_mouse_limit.max_y(), MAX(g_mouse_position.y, g_mouse_limit.origin.y))
    );
    g_mouse_position = clamped_point;
    return clamped_point;
}


