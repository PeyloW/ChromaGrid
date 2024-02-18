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

class cgsount_t {
public:
    cgsount_t(char *path);
    ~cgsount_t();
  
    void set_active();
    
private:
    int8_t *sample;
    size_t length;
};

class cgmusic_t {
public:
    cgmusic_t(const char *path);
    ~cgmusic_t();
    
    void set_active(int track);

private:
    void *sndh;
    int _track;
    
};

#endif /* audio_hpp */
