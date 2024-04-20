//
//  audio.hpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-18.
//

#ifndef audio_hpp
#define audio_hpp

#include "cincludes.hpp"
#include "types.hpp"
#include "memory.hpp"

namespace toybox {
    
    using namespace toystd;
    
    class sound_c : public nocopy_c {
        friend class audio_mixer_c;
    public:
        sound_c(const char *path);
        ~sound_c() = default;
        
        const int8_t *sample() const { return _sample.get(); }
        uint32_t length() const { return _length; }
        uint16_t rate() const { return _rate; }
        
    private:
        unique_ptr_c<int8_t> _sample;
        uint32_t _length;
        uint16_t _rate;
    };
    
#if TOYBOX_TARGET_ATARI
    class music_c : public nocopy_c {
        friend class audio_mixer_c;
    public:
        music_c(const char *path);
        ~music_c() = default;
                
        const char *title() const { return _title; }
        const char *composer() const { return _composer; }
        int track_count() const { return _track_count; }
        uint8_t replay_freq() const { return _freq; }
        
    private:
        unique_ptr_c<uint8_t> _sndh;
        char *_title;
        char *_composer;
        int _track_count;
        uint8_t _freq;
#ifdef __M68000__
        uint16_t _music_init_code[8];
        uint16_t _music_exit_code[8];
        uint16_t _music_play_code[8];
#endif
    };
#else
#   error "Unsupported target"
#endif

}

#endif /* audio_hpp */
