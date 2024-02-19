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

class cgsount_c {
public:
    cgsount_c(char *path);
    ~cgsount_c();
  
    void set_active();
    
private:
    int8_t *_sample;
    size_t _length;
};

class cgmusic_c {
public:
    cgmusic_c(const char *path);
    ~cgmusic_c();
    
    void set_active(int track);

private:
    void *_sndh;
    int _track;
    
};

#endif /* audio_hpp */
