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
         
         mouse_c(rect_s limit);
         ~mouse_c();
         
         void update_state();  // Call once per vbl
         
         bool is_pressed(button_e button) const;
         state_e state(button_e button) const;
         
         point_s postion();
         
     };
    
    class joystick_c : public nocopy_c {
    };

    class keyboard_c : public nocopy_c {
    };

}

#endif /* input_hpp */
