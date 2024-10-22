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
    
    /**
     A `screen_c` is an abstracyion for displaying a screen of content.
     Contains an `image_c` for the bitmap data, a `canvas_c` to draw graphics,
     amd a `dirtymap_c` to restore dirty areas.
     */
    class screen_c {
    public:
        screen_c(size_s screen_size = TOYBOX_SCREEN_SIZE_DEFAULT);
        ~screen_c();
        
        image_c &image() const { return *(image_c*)&_image; }
        canvas_c &canvas() const { return *(canvas_c*)&_canvas; }
        dirtymap_c *dirtymap() const { return _dirtymap; }
        
        size_s size() const __pure { return _image.size(); }
        point_s offset() const __pure { return _offset; }
        void set_offset(point_s o) { _offset = o; }
        
    private:
        image_c _image;
        canvas_c _canvas;
        point_s _offset;
        dirtymap_c *_dirtymap;
    };
    
}

#endif /* screen_hpp */
