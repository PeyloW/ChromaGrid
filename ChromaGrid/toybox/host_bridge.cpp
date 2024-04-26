//
//  host_bridge.cpp
//  toybox
//
//  Created by Fredrik on 2024-04-25.
//

#include "host_bridge.hpp"

using namespace toybox;

extern "C" {
    extern void g_vbl_interupt();
    extern void g_clock_interupt();
    extern void g_update_mouse(point_s position, bool left, bool right);
}

static host_bridge_c *s_bridge = nullptr;

host_bridge_c& host_bridge_c::shared() {
    assert(s_bridge);
    return *s_bridge;
}
void host_bridge_c::set_shared(host_bridge_c &bridge) {
    s_bridge = &bridge;
}

// Host must call on a 50hz interval
void host_bridge_c::vbl_interupt() {
    g_vbl_interupt();
}

// Host must call on a 200hz interval
void host_bridge_c::clock_interupt() {
    g_clock_interupt();
}
        
// Host must call when mouse state changes
void host_bridge_c::update_mouse(point_s position, bool left, bool right) {
    g_update_mouse(position, left, right);
}
