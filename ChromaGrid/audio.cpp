//
//  audio.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-18.
//

#include "audio.hpp"
#include "system.hpp"


cgsount_c::cgsount_c(char *path) {
    // Load from aiff
}

cgsount_c::~cgsount_c() {
    // Free memory
}

void cgsount_c::set_active() {
    // Play sample
}

#ifdef __M68000__
static uint16_t cgg_music_init_code[8];
static uint16_t cgg_music_exit_code[8];
static uint16_t cgg_music_vbl_function_code[8];
#endif

cgmusic_c::cgmusic_c(const char *path) : _track(0) {
    FILE *file = fopen(path, "r");
    assert(file);
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0L, SEEK_SET);
    
    _sndh = malloc(size);
    size_t read = fread(_sndh, size, 1, file);
    assert(read == 1);
    assert(memcmp((char *)_sndh + 12, "SNDH", 4) == 0);
#ifdef __M68000__
    cgcodegen_t::make_trampoline(cgg_music_init_code, _sndh, false);
    cgcodegen_t::make_trampoline(cgg_music_exit_code, _sndh + 4, false);
    cgcodegen_t::make_trampoline(cgg_music_vbl_function_code, _sndh + 8, false);
#endif
}

cgmusic_c::~cgmusic_c() {
    if (_track > 0) {
        set_active(0);
    }
}

void cgmusic_c::set_active(int track) {
    if (_track != track) {
        cgtimer_c::with_paused_timers([this, track] {
#ifdef __M68000__
            cgtimer_c vbl(cgtimer_c::vbl);
            if (_track > 0) {
                // Exit driver
                ((cgtimer_c::func_t)cgg_music_exit_code)();
                // remove VBL
                vbl.remove_func((cgtimer_c::func_t)cgg_music_vbl_function_code);
            }
            if (track > 0) {
                // init driver
                ((cgtimer_c::func_i_t)cgg_music_init_code)(track);
                // add VBL
                vbl.add_func((cgtimer_c::func_t)cgg_music_vbl_function_code);
            }
#endif
        });
        _track = track;
    }
}
