//
//  system_host.hpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-11.
//

#ifdef __M68000__
#error "For host machine only"
#else

#ifndef system_host_h
#define system_host_h

#include "system_helpers.hpp"

namespace toybox {
    
    using namespace toybox;
    
    class sound_c;
    
    /**
     A `host_bridge_c` is the abstraction needed for running a target emulated
     on a modern host machine, such as macOS.
     TODO: Abstract this for more targets such as Linux/SDL
     */
    class host_bridge_c : nocopy_c {
    public:
        static host_bridge_c& shared();
        static void set_shared(host_bridge_c &bridge);

        // Host must call on a 50hz interval
        void vbl_interupt();
        
        // Host must call on a 200hz interval
        void clock_interupt();
                
        // Host must call when mouse state changes
        void update_mouse(point_s position, bool left, bool right);
                
        // Host must provide a yield function
        virtual void yield() = 0;

        virtual void pause_timers() = 0;
        virtual void resume_timers() = 0;

        // Host should provide a play function
        virtual void play(const sound_c &sound) {};
        
    private:
        
    };
    
}

#endif /* system_host_h */
#endif
