//
//  screen.hpp
//  toybox
//
//  Created by Fredrik on 2024-04-17.
//

#ifndef screen_hpp
#define screen_hpp

#include "canvas.hpp"

namespace toybox {
    
    class screen_c {
    public:
        screen_c(size_s screen_size = size_s(320, 200));
        ~screen_c();
        
        image_c &get_image() const __pure;
        canvas_c &get_canvas() const __pure;
        dirtymap_c *get_dirtymap() const __pure;
        
        size_s get_size() const __pure { return _image.get_size(); }
        point_s get_offset() const __pure { return _offset; }
        void set_offset(point_s o) { _offset = o; }

        void set_active() const;
        
    private:
        image_c _image;
        canvas_c _canvas;
        point_s _offset;
        dirtymap_c *_dirtymap;
    };
    
}

#endif /* screen_hpp */
