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

class cgsount_c : private cgnocopy_c {
public:
    cgsount_c(const char *path);
    ~cgsount_c();
  
    void set_active() const;
    
    const int8_t *get_sample() const { return _sample; }
    uint32_t get_length() const { return _length; }
    uint16_t get_rate() const { return _rate; }

private:
    int8_t *_sample;
    uint32_t _length;
    uint16_t _rate;
};

class cgmusic_c : private cgnocopy_c {
public:
    cgmusic_c(const char *path);
    ~cgmusic_c();
    
    void set_active(int track) const; // Track starts at 1, not 0
    
    const char *get_title() const { return _title; }
    const char *get_composer() const { return _composer; }
    int get_track_count() const { return _track_count; }
    uint8_t get_replay_freq() const { return _freq; }

private:
    void *_sndh;
    char *_title;
    char *_composer;
    int _track_count;
    uint8_t _freq;
    mutable int _track;
};

#endif /* audio_hpp */
