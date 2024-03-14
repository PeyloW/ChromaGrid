//
//  audio.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-18.
//

#include "audio.hpp"
#include "system.hpp"
#include "iff_file.hpp"
#ifndef __M68000__
#include "system_host.hpp"
#endif

CGDEFINE_ID (AIFF);
CGDEFINE_ID (COMM);
CGDEFINE_ID (SSND);

struct extended80_t {
    uint16_t exp;
    uint16_t fracs[4];
    uint16_t to_uint16() const {
        int16_t exponent = (exp & 0x7fff) - 16383;
        uint16_t significant = 0;
        significant = fracs[0];
        if (exponent > 15) {
            return 0;
        } else {
            return significant >> ( 15 - exponent );
        }
    }
};
static_assert(sizeof(extended80_t) == 10, "extended80_t size mismatch");

struct __packed_struct aiff_common_t {
    int16_t num_channels;
    uint32_t num_sample_frames;
    int16_t sample_size;
    extended80_t sample_rate;
};
static_assert(sizeof(aiff_common_t) == 18, "aiff_common_t size mismatch");

struct aiff_ssnd_data_t {
    uint32_t offset;
    uint32_t block_size;
    uint8_t data[];
};
static_assert(sizeof(aiff_ssnd_data_t) == 8, "ssnd_data_t size mismatch");

#ifndef __M68000__
static void cghton(extended80_t &ext80) {
    cghton(ext80.exp);
    cghton(ext80.fracs[0]);
}
static void cghton(aiff_common_t &common) {
    cghton(common.num_channels);
    cghton(common.num_sample_frames);
    cghton(common.sample_size);
    cghton(common.sample_rate);
}
static void cghton(aiff_ssnd_data_t &ssnd) {
    cghton(ssnd.offset);
    cghton(ssnd.block_size);
}
#endif


cgsount_c::cgsount_c(const char *path) :
    _sample(nullptr),
    _length(0),
    _rate(0)
{
    cgiff_file_c file(path);
    cgiff_group_t form;
    if (!file.first(CGIFF_FORM, CGIFF_AIFF, form)) {
        hard_assert(0);
        return; // Not a AIFF
    }
    cgiff_chunk_t chunk;
    aiff_common_t common;
    while (file.next(form, "*", chunk)) {
        if (cgiff_id_match(chunk.id, CGIFF_COMM)) {
            if (!file.read(common)) {
                return;
            }
            assert(common.num_channels == 1); // Only mono supported
            assert(common.sample_size == 8); // Only 8 bit audio
            _length = common.num_sample_frames;
            _rate = common.sample_rate.to_uint16();
            assert(_rate >= 11000 && _rate <= 14000); // Ball park ;)
        } else if (cgiff_id_match(chunk.id, CGIFF_SSND)) {
            aiff_ssnd_data_t data;
            if (!file.read(data)) {
                return;
            }
            assert(data.offset == 0);
            assert(chunk.size - 8 == common.num_sample_frames);
#ifndef __M68000__
            _length = form.size + 8;
            _sample = (int8_t *)malloc(_length);
            file.set_pos(0);
            file.read(_sample, 1, _length);
            return;
#endif
            _sample = (int8_t *)malloc(_length);
            file.read(_sample, 1, _length);
        } else {
#ifndef __M68000__
            printf("Skipping '%c%c%c%c'\n", (chunk.id >> 24) & 0xff, (chunk.id >> 16) & 0xff, (chunk.id >> 8) & 0xff, chunk.id & 0xff);
#endif
            file.skip(chunk);
        }
    }
}

cgsount_c::~cgsount_c() {
    free(_sample);
    _sample = nullptr;
}

void cgsount_c::set_active() const {
#ifdef __M68000__
    cgg_microwire_write(0x4c | 40); // Max master volume (0 to 40)
    cgg_microwire_write(0x50 | 20); // Right volume (0 to 20)
    cgg_microwire_write(0x54 | 20); // Left volume (0 to 20)

    uint8_t * ste_dma_control  = (uint8_t*)0xffff8901;
    uint8_t * ste_dmo_mode  = (uint8_t*)0xffff8921;
    uint8_t * ste_dma_start = (uint8_t*)0xffff8903;
    uint8_t * ste_dms_end   = (uint8_t*)0xffff890f;
    // Stop audio
    *ste_dma_control &= 0xFE;
    // Set start address, high to low byte
    size_t tmp = (size_t)_sample;
    ste_dma_start[0] = (uint8_t)((tmp >> 16)&0xff);
    ste_dma_start[2] = (uint8_t)((tmp >>  8)&0xff);
    ste_dma_start[4] = (uint8_t)((tmp       )&0xff);
    // Set end address, high to low byte
    tmp += _length;
    ste_dms_end[0] = (uint8_t)((tmp >> 16)&0xff);
    ste_dms_end[2] = (uint8_t)((tmp >>  8)&0xff);
    ste_dms_end[4] = (uint8_t)((tmp       )&0xff);
    // Set mode, and start
    *ste_dmo_mode = 0x81; // 8 bit mono @ 12.5kHz
    *ste_dma_control = 1; // Play once
#else
    cgg_active_sound = (cgsount_c *)this;
#endif
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

void cgmusic_c::set_active(int track) const {
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
