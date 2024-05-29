# TOYBOX

A minimal C++ framework for writing Atari ST entertainment software.

### Project Requirement

* GCC-4.6.4 with fastcall support (https://github.com/PeyloW/gcc-4.6.4)
* libcmini-0.47 (https://github.com/freemint/libcmini)

## Project philosofy

Toybox should be small, fast and convenient. In order to be small toybox shall use a bare minimum of libcmini, and not inlcude or implement anything not directly needed by a client program. In order to be fast toybox shall rely on C++ compiler optimizations, and rely on error checking on host machine not M68k target. In order to be convenient API shall be designed similar to C++ standard library and/or boost.

Types uses suffixes:

* `_t` - Plain typedef of POD
* `_e` - Enumeration
* `_s` - Struct
* `_c` - Class

Structs should be primary about member variable, and all members are public.

Classes should be primary about member functions, and member variables pribate or protected. To avoid accidental expensive copy operations classes should inherit publicly from `nocopy_c` in order to delete copy construction and assignment.

All member functions and non-POD arguments should be const when possible. Structs and classes with a `sizeof` grater than 4 should be bassed as references. Pass objects as pointer only for out arguments, or to signify that an object is optional.


## Available functionality

A non exhaustive list of the available primary classes.


### Utilities

Simplified utility functions as available in C++ stdlib. No memory allocators, no helper structs like `less` when the `<` operator will do.

* `nocopy_c` - Base class for all classes, remove copy functionality.
* `pair_c` - A simple pair.
* `lower_bound`, `binary_search`, `sort` - Good to have functions


### Memory

Dynamic memory allocation and deallocation is expensive, and should be avoided whenever possible. But when done, these classes are to be used. 

* `unique_ptr_c`, `shared_ptr_c` - C++ stdlib equivalents.
* `static_allocator_c` - Helper class for fast alloc/dealloc of statically sized memory blocks.


### Collections

Minimal set of C++ stdlib inspired collections. No allocators to be be used, and space for items are allocated in place.

* `vector_c` - A subset of `std::vector`.
* `forward_list_c` - A subset of `std::forward_list`.
* TODO: `flat_map` - A subset of `std::flat_map`.

TODO: Static size of `0` should be used for defining dynamically sized specializations.


### Streams

Minimal set of not quite stdlib, not boost, streams. The streams uses virtual methods for abstraction.

* `streamn_c` - Abstract baseclass.
    * `fstream_c` - Stream wrapper ontop of a file handle.
    * `strsteram_s` - Stream wrapper ontop of a memory buffer.
    * `swapstream_c` - Stream for byte order swapping on read and write.
    * `iffstream_c` - A stream that simplifies reading and writing of EA IFF 85 files.

TODO: Rewrite all this to be more like boost streams, with filters taking the place of swapstream's functionality. Abstracting IFF file access as a stream was a bad idea in hindsight.


### Machine abstractions

* `machine_c` - Singletong providing access to hardware and software information.
* `timer_c` - Singletons for VBL and system clock timers.
* `mouse_c` - Mouse input.
* TODO: `joystick_c`, `keyboard_c`


### Assets management

Assets are lazy loaded, or can be loaded as sets. Assets can also be unloaded as sets to give memory back.

* `asset_manager_c` - Singleton providing access to assets.
* `asset_c` - Abstract asset baseclass.
    * `image_c` - A bitmap image, can be initialised of IFF ilbm files.
    * `tileset_c` - A helper class for managing tiles or animation frames from an image.
    * `font_c` - A bitmap font, initialize from an image and font def.
    * `audio_c` - A 8-bit PCM sample, initialized from an aif file.
    * `music_c` - An YM sound file, initialized from a sndh file.


### Image and drawing

The drawing operations are using the Atari STe blitter, with a software blitter emulator for macOS host.

* `image_c` - A read only image.
    * An image can optionally be masked, eg. be a sprite.
* `canvas_c` - A context for drawing onto an image.
    * `fill(int ...` to fill a rectangle.
    * `draw(image_c ...` to draw an image
    * `draw_aligned(image_c ...` to draw an image aligned to nearest multiple of 16 horizontally. 
    * `draw(tileset_c ...` to draw a tile from a tileset
    * `draw(font_c ...` to draw text.
* `dirtymap_c` - A helper class for marking areas of an image as dirty, for faster restoration.
* `stencil_c` - A 16x16 1 bit mask to filter drawing operation through.
* `screen_c` - A screen of content, wrapping an image, a canvas and a dirtymap.

TODO: Should screen_c be a subclass of `canvas_c`, possibly even multiple superclasses?


### Audio replay

* `audio_c` - A 12.5kHz PCM audio file.
* `music_c` - A YM sndh file.

TODO: Allow samples at other frequenzies than 12.5kHz. Implement an audiomixer to support playing several sound effects simultaniously, should support pitch changes. Add support to also play MOD-files.


### Game life-cycle

A game is intended to be implemnted as a stack of scenes. Navigating to a new screen such as from the menu to a level involves either pushing a new scene onto the stack, or replacing the top scene. Navigating back is popping the stack, popping the last scenes exits the game.

* `scene_manager_c` - The manager singleton.
    * `push(...`, `pop(...`, `replace(...` to manage the scene stack.
    * `overlay_scene` a scene to draw ontop of all other content, such as a status bar or mouse cursor.
    * `front`, `back`, `clear` three screens, front is being presented, back is being drawn, and clear is used for restoring other screens from their dirtymaps.
* `scene_c` - The abstract scene baseclass.
    * `configuration` the scene configuration, only the palette to use for now.
    * `will_appear` called when scene becomes the top scene and will appear.
        * Implement to draw initial content.
    * `update_clear` updatye the clear screen.
    * `update_back` update the back screen that will be presented next screen swap.
* `transition_c` - A transitions between two scenes, run for push, pop and replace operations.
    
TODO: Support screens of sizes other than 320x200 for hardware scrolling.
