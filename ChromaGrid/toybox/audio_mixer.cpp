//
//  audio_mixer.cpp
//  toybox
//
//  Created by Fredrik on 2024-04-20.
//

#include "audio_mixer.hpp"
#include "system.hpp"
#ifndef __M68000__
#include "system_host.hpp"
#endif

using namespace toybox;

audio_mixer_c& audio_mixer_c::shared() {
    static audio_mixer_c s_mixer;
    return s_mixer;
}

void audio_mixer_c::play(const sound_c &sound, uint8_t priority) {
#ifdef __M68000__
#   if TOYBOX_TARGET_ATARI
    uint8_t * ste_dma_control  = (uint8_t*)0xffff8901;
    uint8_t * ste_dma_mode  = (uint8_t*)0xffff8921;
    uint8_t * ste_dma_start = (uint8_t*)0xffff8903;
    uint8_t * ste_dma_end   = (uint8_t*)0xffff890f;
    // Stop audio
    *ste_dma_control &= 0xFE;
    // Set start address, high to low byte
    size_t tmp = (size_t)sound._sample.get();
    ste_dma_start[0] = (uint8_t)((tmp >> 16)&0xff);
    ste_dma_start[2] = (uint8_t)((tmp >>  8)&0xff);
    ste_dma_start[4] = (uint8_t)((tmp       )&0xff);
    // Set end address, high to low byte
    tmp += sound._length;
    ste_dma_end[0] = (uint8_t)((tmp >> 16)&0xff);
    ste_dma_end[2] = (uint8_t)((tmp >>  8)&0xff);
    ste_dma_end[4] = (uint8_t)((tmp       )&0xff);
    // Set mode, and start
    *ste_dma_mode = 0x81; // 8 bit mono @ 12.5kHz
    *ste_dma_control = 1; // Play once
#   else
#       error "Unsupported target"
#   endif
#else
    g_active_sound = &sound;
#endif
}

void audio_mixer_c::stop(const sound_c &sound) {
    // No-op for now.
}

#if TOYBOX_TARGET_ATARI

void audio_mixer_c::play(const music_c &music, int track) {
    if (_active_music) {
        stop(*_active_music);
    }
    assert(track > 0);
#ifdef __M68000__
    timer_c::with_paused_timers([this, &music, track] {
        timer_c clock(timer_c::clock);
        // init driver
        ((timer_c::func_i_t)music._music_init_code)(track);
        // add VBL
        clock.add_func((timer_c::func_t)music._music_play_code, music._freq);
    });
#endif
    _active_music = &music;
    _active_track = track;
}

void audio_mixer_c::stop(const music_c &music) {
    assert(_active_music == &music);
#ifdef __M68000__
    timer_c::with_paused_timers([this, &music] {
        timer_c clock(timer_c::clock);
        // Exit driver
        ((timer_c::func_t)music._music_exit_code)();
        // remove timer func
        clock.remove_func((timer_c::func_t)music._music_play_code);
    });
#endif
    _active_music = nullptr;
    _active_track = 0;
}

#endif

void audio_mixer_c::stop_all() {
    if (_active_music) {
        stop(*_active_music);
    }
}


audio_mixer_c::audio_mixer_c() : _active_music(nullptr), _active_track(0) {
#ifdef __M68000__
#   if TOYBOX_TARGET_ATARI
    g_microwire_write(0x4c | 40); // Max master volume (0 to 40)
    g_microwire_write(0x50 | 20); // Right volume (0 to 20)
    g_microwire_write(0x54 | 20); // Left volume (0 to 20)
#   endif
#endif
}

audio_mixer_c::~audio_mixer_c() {
    stop_all();
};

