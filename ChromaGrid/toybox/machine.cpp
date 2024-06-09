//
//  machine.cpp
//  toybox
//
//  Created by Fredrik on 2024-04-24.
//

#include "machine.hpp"
#include "timer.hpp"
#include "image.hpp"

using namespace toybox;


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

machine_c::type_e machine_c::type() const {
#if TOYBOX_TARGET_ATARI
#   ifdef __M68000__
    return (type_e)((get_cookie(0x5F4D4348) >> 16) + 1); // '_MCH'
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


#if TOYBOX_TARGET_ATARI
#   ifdef __M68000__
struct mem_chunk {
    long valid;
#define VAL_ALLOC 0xa11c0abcL
    struct mem_chunk *next;
    unsigned long size;
};
#define BORDER_EXTRA ((sizeof(struct mem_chunk) + sizeof(long) + 7) & ~7)
void machine_c::free_system_memory() {
    mem_chunk *p = *(mem_chunk **)(0x44e);
    p->valid = VAL_ALLOC;
    p->next = nullptr;
    p->size = 32000;
    p++;
    _free(p);
}
#   else
void machine_c::free_system_memory() {}
#   endif
#endif

uint32_t machine_c::get_cookie(uint32_t cookie, uint32_t def_value) const {
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

extern "C" {
    const palette_c *g_active_palette = nullptr;
    const image_c *g_active_image = nullptr;
}

const image_c *machine_c::active_image() const {
    return g_active_image;
}

void machine_c::set_active_image(const image_c *image, point_s offset) {
    assert(offset.x == 0 && offset.y == 0);
    timer_c::with_paused_timers([this, image] {
        g_active_image = image;
        if (type() > ste) {
#ifdef __M68000__
            *((uint16_t*)0x452) = 1;
            *((uint16_t **)0x45E) = image->_bitmap.get();
#endif
        }
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
    g_active_palette = palette;
#else
    timer_c::with_paused_timers([palette]{
        g_active_palette = palette;
    });
#endif
}

