//
//  system.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-11.
//

#include "system.hpp"
#include "forward_list.hpp"
#include "algorithm.hpp"
#include "memory.hpp"

extern "C" {

    using namespace toystd;
    using namespace toybox;
    
#ifndef __M68000__
#include <unistd.h>
#include "system_host.hpp"
#endif

static int g_timer_ref_counts[2] = { 0 };

typedef struct __packed_struct {
    uint8_t freq;
    uint8_t cnt;
    timer_c::func_a_t func;
    void *context;
} timer_func_s;
    
#define TIMER_FUNC_MAX_CNT 16
typedef forward_list_c<timer_func_s, TIMER_FUNC_MAX_CNT> timer_func_list_c;
#ifdef __M68000__
static_assert(sizeof(timer_func_list_c::_node_s) == 14, "timer_func_list_c::_node_s) size mismatch");
#endif
    
timer_func_list_c g_vbl_functions;
volatile uint32_t g_vbl_tick = 0;
timer_func_list_c g_clock_functions;
volatile uint32_t g_clock_tick = 0;
    
static rect_s g_mouse_limit;
static uint8_t g_prev_mouse_butons;
uint8_t g_mouse_buttons;
static mouse_c::state_e g_mouse_button_states[2];
point_s g_mouse_position;

#ifdef __M68000__
#   if TOYBOX_TARGET_ATARI
    extern timer_c::func_t g_system_vbl_interupt;
    extern void g_vbl_interupt();
    extern int16_t g_system_vbl_freq;
    extern timer_c::func_t g_system_clock_interupt;
    extern void g_clock_interupt();
    extern timer_c::func_a_t g_system_mouse_interupt;
    extern void g_mouse_interupt(void *);
    static _KBDVECS *g_keyboard_vectors = nullptr;
#   else
#       error "Usupported target"
#   endif
#else
    void (*g_yield_function)() = nullptr;

    static void g_do_timer(timer_func_list_c &timer_funcs, int freq) {
        for (auto &timer_func : timer_funcs) {
            bool trigger = false;
            int cnt = (int)timer_func.cnt - timer_func.freq;
            if (cnt <= 0) {
                trigger = true;
                cnt += freq;
            }
            timer_func.cnt = (uint8_t)cnt;
            if (trigger) {
                timer_func.func(timer_func.context);
            }
        }
    }
    
    void g_vbl_interupt() {
        if (g_timer_ref_counts[timer_c::vbl] > 0) {
            g_vbl_tick++;
            g_do_timer(g_vbl_functions, 50);
        }
    }

    void g_clock_interupt() {
        if (g_timer_ref_counts[timer_c::clock] > 0) {
            g_clock_tick++;
            g_do_timer(g_clock_functions, 200);
        }
    }
    
    // Host must call when mouse state changes
    void g_update_mouse(point_s position, bool left, bool right) {
        g_mouse_position = position;
        g_mouse_buttons = (left ? 2 : 0) | (right ? 1 : 0);
    }
#endif
}

template<>
timer_func_list_c::allocator::type timer_func_list_c::allocator::first_block = nullptr;

template<>
detail::shared_count_t::allocator::type detail::shared_count_t::allocator::first_block = nullptr;

machine_c &machine_c::shared() {
    static machine_c s_machine;
    return s_machine;
}

machine_c::machine_c() {
#if TOYBOX_TARGET_ATARI
#   ifdef __M68000__
    _old_super = Super(0);
    _old_modes[0] = Blitmode(-1);
    Blitmode(0);
    _old_modes[1] = Getrez();
    Setscreen((void *)-1, (void *)-1, 0);
    _old_modes[2] = *((uint8_t*)0x484);
    *((uint8_t*)0x484) = 0;
#   endif
#else
#   error "Unsupported target"
#endif
}

machine_c::~machine_c() {
#if TOYBOX_TARGET_ATARI
#   ifdef __M68000__
    *((uint8_t*)0x484) = (uint8_t)_old_modes[2];
    Setscreen((void *)-1, (void *)-1, _old_modes[1]);
    Blitmode(_old_modes[0]);
    Super(_old_super);
#   endif
#else
#   error "Unsupported target"
#endif
}

#if TOYBOX_TARGET_ATARI
static uint32_t get_cookie(uint32_t cookie, uint32_t def_value = 0) {
#ifdef __M68000__
    uint32_t *cookie_jar = *((uint32_t**)0x5A0);
    if (cookie_jar) {
        while ((cookie_jar[0] != 0)) {
            if (cookie_jar[0] == cookie) {
                return cookie_jar[1];
            }
            cookie_jar += 2;
        }
    }
    return def_value;
#else
    return def_value;
#endif
}
#endif

machine_c::type_e machine_c::type() const {
#if TOYBOX_TARGET_ATARI
#   ifdef __M68000__
    return (type_e)(get_cookie(0x5F4D4348) >> 16); // '_MCH'
#   else
    return ste;
#   endif
#else
#   error "Unsupported target"
#endif
}

size_s machine_c::screen_size() const {
    return size_s(320, 200);
}

size_t machine_c::max_memory() const {
#if TOYBOX_TARGET_ATARI
#   ifdef __M68000__
    return *((uint32_t*)0x436);
#   else
    return 0x100000;
#   endif
#else
#   error "Unsupported target"
#endif
}

size_t machine_c::user_memory() const {
#if TOYBOX_TARGET_ATARI
#   ifdef __M68000__
    return max_memory() - *((uint32_t*)0x432);
#   else
    return max_memory() - 0x10000;
#   endif
#else
#   error "Unsupported target"
#endif
}

extern "C" {
    const palette_c *g_active_palette = nullptr;
    const image_c *g_active_image = nullptr;
}

const image_c *machine_c::active_image() const {
    return g_active_image;
}

void machine_c::set_active_image(const image_c *image, point_s offset) {
    assert(offset.x == 0 && offset.y == 0);
    timer_c::with_paused_timers([image] {
        g_active_image = image;
    });
}

const palette_c *machine_c::active_palette() const {
    return g_active_palette;
}

void machine_c::set_active_palette(const palette_c *palette) {
#ifdef __M68000__
#   if TOYBOX_TARGET_ATARI
    memcpy(reinterpret_cast<uint16_t*>(0xffff8240), palette, 32);
#   else
#       error "Unsupported target"
#   endif
#endif
    g_active_palette = palette;
}

timer_c::timer_c(timer_e timer) : _timer(timer) {
    assert(timer == vbl || timer == clock);
    machine_c::shared();
    g_timer_ref_counts[timer]++;
    if (g_timer_ref_counts[timer] == 1) {
        with_paused_timers([timer] {
            switch (timer) {
                case vbl:
                #ifdef __M68000__
                    g_system_vbl_interupt = *((func_t *)0x0070);
                    *((func_t *)0x0070) = &g_vbl_interupt;
                    g_system_vbl_freq = *(uint8_t*)0xffff820a == 0 ? 60 : 50;
                #endif
                    break;
                case clock:
                #ifdef __M68000__
                    g_system_clock_interupt = *((func_t *)0x0114);
                    *((func_t *)0x0114) = &g_clock_interupt;
                #endif
                    break;
            }
        });
    }
}

timer_c::~timer_c() {
    g_timer_ref_counts[_timer]--;
    assert(g_timer_ref_counts[_timer] >= 0);
    if (g_timer_ref_counts[_timer] == 0) {
        with_paused_timers([this] {
#   if TOYBOX_TARGET_ATARI
            switch (_timer) {
                case vbl:
                #ifdef __M68000__
                    *((func_t *)0x0070) = g_system_vbl_interupt;
                #endif
                    break;
                case clock:
                #ifdef __M68000__
                    *((func_t *)0x0114) = g_system_clock_interupt;
                #endif
                    break;
            }
#else
#  error "Unsupported target"
#endif
        });
    }
}

uint8_t timer_c::base_freq() const {
    switch (_timer) {
        case vbl:
#ifdef __M68000__
            return g_system_vbl_freq;
#else
            return 50;
#endif
            //
        case clock:
            return 200;
        default:
            assert(0);
            return 0;
    }
}

void timer_c::add_func(func_t func, uint8_t freq) {
    add_func((func_a_t)func, nullptr, freq);
}

void timer_c::remove_func(func_t func) {
    remove_func((func_a_t)func, nullptr);
}

void timer_c::add_func(func_a_t func, void *context, uint8_t freq) {
    if (freq == 0) {
        freq = base_freq();
    }
    with_paused_timers([this, func, context, freq] {
        auto &functions = _timer == vbl ? g_vbl_functions : g_clock_functions;
        functions.push_front((timer_func_s){freq, base_freq(), func, context});
    });
}

void timer_c::remove_func(func_a_t func, void *context) {
    with_paused_timers([this, func, context] {
        auto &functions = _timer == vbl ? g_vbl_functions : g_clock_functions;
        auto it = functions.before_begin();
        while (it._node->next) {
            if (it._node->next->value.func == func && it._node->next->value.context == context) {
                functions.erase_after(it);
                return;
            }
            it++;
        }
        assert(0);
    });
}

uint32_t timer_c::tick() {
    return (_timer == vbl) ? g_vbl_tick : g_clock_tick;
}

void timer_c::reset_tick() {
    if (_timer == vbl) {
        g_vbl_tick = 0;
    } else {
        g_clock_tick = 0;
    }
}

void timer_c::wait(int ticks) {
    const auto wait_tick = tick() + ticks;
    while (wait_tick >= tick()) {
#ifndef __M68000__
        g_yield_function();
#endif
    }
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
