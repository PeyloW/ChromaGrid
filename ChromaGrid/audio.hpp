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
    
    void set_active(int track) const;

private:
    void *_sndh;
    mutable int _track;
};

#endif /* audio_hpp */
