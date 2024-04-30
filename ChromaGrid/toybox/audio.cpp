//
//  audio.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-18.
//

#include "audio.hpp"
#include "iffstream.hpp"
#include "system_helpers.hpp"

using namespace toybox;

DEFINE_IFF_ID (AIFF);
DEFINE_IFF_ID (COMM);
DEFINE_IFF_ID (SSND);

struct __packed_struct extended80_s {
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
static_assert(sizeof(extended80_s) == 10, "extended80_t size mismatch");

struct __packed_struct aiff_common_s {
    int16_t num_channels;
    uint32_t num_sample_frames;
    int16_t sample_size;
    extended80_s sample_rate;
};
static_assert(sizeof(aiff_common_s) == 18, "aiff_common_t size mismatch");
namespace toystd {
    template<>
    struct struct_layout<aiff_common_s> {
        static constexpr char *value = "1w1l6w";
    };
}

struct __packed_struct aiff_ssnd_data_s {
    uint32_t offset;
    uint32_t block_size;
    uint8_t data[];
};
static_assert(sizeof(aiff_ssnd_data_s) == 8, "ssnd_data_t size mismatch");
namespace toystd {
    template<>
    struct struct_layout<aiff_ssnd_data_s> {
        static constexpr char *value = "2l";
    };
}

sound_c::sound_c(const char *path) :
    _sample(nullptr),
    _length(0),
    _rate(0)
{
    iffstream_c file(path);
    iff_group_s form;
    if (!file.good() || !file.first(IFF_FORM, IFF_AIFF, form)) {
        hard_assert(0);
        return; // Not a AIFF
    }
    iff_chunk_s chunk;
    aiff_common_s common;
    while (file.next(form, "*", chunk)) {
        if (iff_id_match(chunk.id, IFF_COMM)) {
            if (!file.read(&common)) {
                return;
            }
            assert(common.num_channels == 1); // Only mono supported
            assert(common.sample_size == 8); // Only 8 bit audio
            _length = common.num_sample_frames;
            _rate = common.sample_rate.to_uint16();
            assert(_rate >= 11000 && _rate <= 14000); // Ball park ;)
        } else if (iff_id_match(chunk.id, IFF_SSND)) {
            aiff_ssnd_data_s data;
            if (!file.read(&data)) {
                return;
            }
            assert(data.offset == 0);
            assert(chunk.size - 8 == common.num_sample_frames);
#ifndef __M68000__
            // For target we hack, and make _sample point to full AIFF file.
            _length = form.size + 8;
            _sample.reset((int8_t *)_malloc(_length));
            file.seek(0, stream_c::beg);
            file.read(_sample.get(), _length);
            return;
#else
            _sample.reset((int8_t *)_malloc(_length));
            file.read(_sample.get(), _length);
#endif
        } else {
#ifndef __M68000__
            printf("Skipping '%c%c%c%c'\n", (chunk.id >> 24) & 0xff, (chunk.id >> 16) & 0xff, (chunk.id >> 8) & 0xff, chunk.id & 0xff);
#endif
            file.skip(chunk);
        }
    }
}

#if TOYBOX_TARGET_ATARI

// libcmini (version used) has a buggy strncmp :(
static int _strncmp(const char *s1, const char *s2, size_t max)
{
    int cmp = 0;
    while (cmp == 0 && max && *s1) {
        cmp = *(unsigned char *)s1 - *(unsigned char *)s2;
        s1++; s2++;
        max--;
    }
    return cmp;
}

music_c::music_c(const char *path) {
    fstream_c file(path);
    hard_assert(file.good());
    file.seek(0, stream_c::end);
    size_t size = file.tell();
    file.seek(0, toystd::fstream_c::beg);
    
    _sndh.reset((uint8_t *)_malloc(size));
    _length = size;
    size_t read = file.read(_sndh.get(), size);
    assert(read == 1);
    assert(memcmp(_sndh + 12, "SNDH", 4) == 0);
    _title = nullptr;
    _composer = nullptr;
    _track_count = 1;
    _freq = 50;
    char *header_str = (char *)(_sndh + 16);
    while (_strncmp(header_str, "HDNS", 4) != 0 && ((uint8_t*)header_str < _sndh + 200)) {
        int len = (int)strlen(header_str);
         if (len > 0) {
            if (len > 100) {
                break;
            }
            if (_strncmp(header_str, "TITL", 4) == 0) {
                _title = header_str + 4;
            } else if (_strncmp(header_str, "COMM", 4) == 0) {
                _composer = header_str + 4;
            } else if (strncmp(header_str, "##", 2) == 0) {
                _track_count = atoi(header_str + 2);
            } else if (_strncmp(header_str, "TA", 2) == 0 ||
                       _strncmp(header_str, "TB", 2) == 0 ||
                       _strncmp(header_str, "TC", 2) == 0 ||
                       _strncmp(header_str, "TD", 2) == 0 ||
                       _strncmp(header_str, "!V", 2) == 0) {
                _freq = atoi(header_str + 2);
                assert(_freq != 0);
            }
        }
        header_str += len;
        while (*++header_str == 0);
    }
#ifdef __M68000__
    codegen_s::make_trampoline(_music_init_code, _sndh + 0, false);
    codegen_s::make_trampoline(_music_exit_code, _sndh + 4, false);
    codegen_s::make_trampoline(_music_play_code, _sndh + 8, false);
#endif
}

#else
#   error "Unsupported target"
#endif
