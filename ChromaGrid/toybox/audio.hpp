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
    public:
        sound_c(const char *path);
        ~sound_c() = default;
        
        void set_active() const;
        
        const int8_t *get_sample() const { return _sample.get(); }
        uint32_t get_length() const { return _length; }
        uint16_t get_rate() const { return _rate; }
        
    private:
        unique_ptr_c<int8_t> _sample;
        uint32_t _length;
        uint16_t _rate;
    };
    
    class music_c : public nocopy_c {
    public:
        music_c(const char *path);
        ~music_c();
        
        void set_active(int track) const; // Track starts at 1, not 0
        
        const char *get_title() const { return _title; }
        const char *get_composer() const { return _composer; }
        int get_track_count() const { return _track_count; }
        uint8_t get_replay_freq() const { return _freq; }
        
    private:
        unique_ptr_c<uint8_t> _sndh;
        char *_title;
        char *_composer;
        int _track_count;
        uint8_t _freq;
        mutable int _track;
    };
    
}

#endif /* audio_hpp */
