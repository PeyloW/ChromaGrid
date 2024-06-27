//
//  audio.hpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-18.
//

#ifndef audio_hpp
#define audio_hpp

#include "cincludes.hpp"
#include "asset.hpp"
#include "types.hpp"
#include "memory.hpp"

namespace toybox {
    
    using namespace toybox;
    
    /**
     A `cound_c` is an 8 bit PCM sound sample.
     Sounds can be loaded for EA 85 AIFF files.
     */
    class sound_c : public asset_c {
        friend class audio_mixer_c;
    public:
        sound_c(const char *path);
        virtual ~sound_c() {};
        
        type_e asset_type() const { return sound; }
        
        const int8_t *sample() const { return _sample.get(); }
        uint32_t length() const { return _length; }
        uint16_t rate() const { return _rate; }
        
    private:
        unique_ptr_c<int8_t> _sample;
        uint32_t _length;
        uint16_t _rate;
    };
    
    /**
     A `music_c` is an abstract collection of music, containing one or more songs.
     TODO: Support mods.
     */
    class music_c : public asset_c {
        friend class audio_mixer_c;
    public:
        music_c() {};
        virtual ~music_c() {};
        
        type_e asset_type() const { return music; }
        
        virtual const char *title() const = 0;
        virtual const char *composer() const = 0;
        virtual int track_count() const = 0;
        virtual uint8_t replay_freq() const = 0;
    };
    
#if TOYBOX_TARGET_ATARI
    /**
     `ymmusic_c` is a concrete `music_c` representing YM-Music for Atari target.
     YB-music can be loaded from SNDH files.
     */
    class ymmusic_c : public music_c {
        friend class audio_mixer_c;
    public:
        ymmusic_c(const char *path);
        virtual ~ymmusic_c() {};

        type_e asset_type() const { return music; }
        
        const char *title() const { return _title; }
        const char *composer() const { return _composer; }
        int track_count() const { return _track_count; }
        uint8_t replay_freq() const { return _freq; }
        
    private:
        unique_ptr_c<uint8_t> _sndh;
        size_t _length;
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
#endif

}

#endif /* audio_hpp */
