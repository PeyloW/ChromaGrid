//
//  input.hpp
//  toybox
//
//  Created by Fredrik on 2024-04-24.
//

#ifndef input_hpp
#define input_hpp

#include "cincludes.hpp"
#include "types.hpp"

namespace toybox {
    
    
    /**
     A `mouse_c` is an abstraction for mouse input.
     The mouse is a lazy initialized singleton.
     */
    class mouse_c : public nocopy_c {
    public:
        typedef enum __packed {
            right, left
        } button_e;
        typedef enum __packed {
            released,
            pressed,
            clicked
        } state_e;
        
        static mouse_c &shared();
        
        const rect_s &limits() const;
        void set_limits(const rect_s &limits);
        
        bool is_pressed(button_e button) const;
        state_e state(button_e button) const;
        
        point_s postion();
        
    private:
        mouse_c();
        ~mouse_c();
        rect_s _limits;
        mutable uint32_t _update_tick;
    };
    
    /**
     A `joystick_c` is an abstraction for joystick/joypad input.
     The joysticks are lazy initialized singleton.
     TODO: Implement this
     */
    class joystick_c : public nocopy_c {
    };
    
    /**
     A `keyboard_c` is an abstraction for keyboard input.
     The keyboard is a lazy initialized singleton.
     TODO: Implement this
     */
    class keyboard_c : public nocopy_c {
    };

}

#endif /* input_hpp */
