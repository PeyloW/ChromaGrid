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
