//
//  audio_mixer.hpp
//  toybox
//
//  Created by Fredrik on 2024-04-20.
//

#ifndef audio_mixer_hpp
#define audio_mixer_hpp

#include "audio.hpp"

namespace toybox {
    
    class audio_mixer_c : public nocopy_c {
    public:
        static audio_mixer_c& shared();

        int channel_count() const __pure { return 1; }
        void play(const sound_c &sound, uint8_t priority = 0);
        void stop(const sound_c &sound);

        void play(const music_c &music_c, int track = 1); // Track starts at 1, not 0;
        void stop(const music_c &music);
        
        void stop_all();

    private:
        const music_c *_active_music;
        int _active_track;
        audio_mixer_c();
        ~audio_mixer_c();
    };
  
}

#endif /* audio_mixer_hpp */
