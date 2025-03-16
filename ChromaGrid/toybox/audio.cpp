//
//  audio.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-18.
//

#include "audio.hpp"
#include "iffstream.hpp"
//#include "system_helpers.hpp"

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
namespace toybox {
    template<>
    struct struct_layout<aiff_common_s> {
        static constexpr const char *value = "1w1l6w";
    };
}

struct __packed_struct aiff_ssnd_data_s {
    uint32_t offset;
    uint32_t block_size;
    uint8_t data[];
};
static_assert(sizeof(aiff_ssnd_data_s) == 8, "ssnd_data_t size mismatch");
namespace toybox {
    template<>
    struct struct_layout<aiff_ssnd_data_s> {
        static constexpr const char *value = "2l";
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
