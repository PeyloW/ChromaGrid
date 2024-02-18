//
//  audio.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-18.
//

#include "audio.hpp"
#include "system.hpp"


cgsount_t::cgsount_t(char *path) {
    // Load from aiff
}

cgsount_t::~cgsount_t() {
    // Free memory
}

void cgsount_t::set_active() {
    // Play sample
}

#ifdef __M68000__
static uint16_t pMusicInitCode[8];
static uint16_t pMusicExitCode[8];
static uint16_t pMusicVBLCode[8];
#endif

cgmusic_t::cgmusic_t(const char *path) : _track(0) {
    FILE *file = fopen(path, "r");
    assert(file);
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0L, SEEK_SET);
    
    sndh = malloc(size);
    size_t read = fread(sndh, size, 1, file);
    assert(read == 1);
    assert(memcmp((char *)sndh + 12, "SNDH", 4) == 0);
#ifdef __M68000__
    generate_safe_trampoline(pMusicInitCode, sndh, false);
    generate_safe_trampoline(pMusicExitCode, sndh + 4, false);
    generate_safe_trampoline(pMusicVBLCode, sndh + 8, false);
#endif
}

cgmusic_t::~cgmusic_t() {
    if (_track > 0) {
        set_active(0);
    }
}

void cgmusic_t::set_active(int track) {
    if (_track != track) {
        cgtimer_t::with_paused_timers([this, track] {
#ifdef __M68000__
            cgtimer_t vbl(cgtimer_t::vbl);
            if (_track > 0) {
                // Exit driver
                ((cgtimer_t::func_t)pMusicExitCode)();
                // remove VBL
                vbl.remove_func((cgtimer_t::func_t)pMusicVBLCode);
            }
            if (track > 0) {
                // init driver
                ((cgtimer_t::func_i_t)pMusicInitCode)(track);
                // add VBL
                vbl.add_func((cgtimer_t::func_t)pMusicVBLCode);
            }
#endif
        });
        _track = track;
    }
}
