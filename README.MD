# Chroma Grid

An Atari STe game by T.O.Y.S. to be release at Sommarhack 2024.

## Credits

* Main Code: _Fredrik 'PeyloW' Olsson_
* Blitter Code: _Hatari 1.0 as reference_
* Music: _Joakim 'AiO' Ekblad_
* Graphics: Hervé 'Exocet' Piton
* Font: _Damien Guard_
* Concept: Peter 'Eagle' Nyman

### Project Requirement

* GCC-4.6.4 with fastcall support (https://github.com/PeyloW/gcc-4.6.4)
* libcmini-0.47 (https://github.com/freemint/libcmini)
* libpng (http://www.libpng.org)

### TODO: List

Legend: __Required__, Expected , _Optional_

* [x] __System__
    * [x] __MacOS Host app harness__
    * [x] __Target machine bootstrap__
    * [x] __VBL timer support__
    * [x] __Timer-C support__
        * [x] Other frequencies than 200hz
    * [x] __Mouse interups__
* [x] __Basic file loading__
    * [x] __ILBM Image files, with masks__
        * [x] __Load fonts__
        * [x] _Proportinal fonts_
    * [x] __SNDH music files__
    * [x] AIFF samples
* [ ] __Graphics__
    * [x] __Display physical screen__
        * [x] _STe HW scrolling_
        * [x] __NEW: Double buffered__
    * [x] __Set & Get pixel__
        * [ ] Asm optimized
        * [x] _Color replacement_
    * [x] __Blit word aligned grapcs__
        * [x] Asm and HW optimized
    * [x] __Blit any alignment with clipping__
        * [x] __With optional masking__
        * [x] Asm and HW optimized
            * [x] ~~__BUG:__ Left edge clipping skews graphics~~
            * [x] ~~__BUG:__ Host blitter emu shifts wrong for 8+16 alignment~~
    * [x] __Draw text__
        * [ ] Optimized numerics
        * [x] _Proportional fonts_
    * [x] Draw 3 patches
    * [x] Add optional dirty mask to all draw operations
        * [x] Restore from dirty mask
        * [x] _Asm and HW optimized_
* [ ] __Sound__
    * [x] __Play SNDH music__
        * [x] Change tracks
        * [x] Parse headers
        * [x] Support non-static replay frequencies
    * [x] __Play LCM samples__
        * [ ] _Mix several channels_
* [ ] __Scene management__
    * [x] __Shared resources manager__
    * [x] __A stage manager - push-pop scenes__
        * [x] **Scene transitions**
        * [ ] Transition through black
    * [x] Generic scene
* [ ] Introduction scene
    * [ ] T.O.Y.S. logo
    * [ ] Large Chroma Grid logo
    * [ ] _Intricate background_
    * [ ] Main music _(Thalion intro rip-off?)_
* [ ] __Main menu screne__
    * [x] __12x16 mouse cursor__
    * [x] __Right side bar background 128px wide__
        * [ ] __Small Chroma Grid Logo__
        * [x] __Button shapes, on and off__
    * [ ] Level selection grid, center
* [x] __Credits scene__
    * [x] Credits
    * [x] Acknowledgements
    * [ ] Dedications
    * [ ] Greetings
* [ ] __Help scene__
    * [ ] Goal of the game
        * [ ] Tile types
        * [ ] Orb fusing    
    * [ ] _Editing levels_
    * [ ] _Scoring_
* [ ] __Gameplay scene__
    * Re-use right side background
    * [x] __16x16 tiles, 5x types__
        * [x] Animations between states
    * [x] __8x8 orbs sprites__
        * [x] Animations between states
    * [x] __Game logic ;)__
        * [x] __Count down timer__
        * [x] Count moves
    * [ ] _Per level scroll text_
    * [ ] In game music, _one or more tracks_
* [ ] Game over scene
    * [x] Scoring summary
    * [ ] Large success logo
    * [ ] Large failure logo
    * [ ] Game over game music 
* [x] _Highscore screne_
    * [x] Per level scores
        * [x] Load/Save scores
    * [ ] ~~_Name entry_~~
    * [x] _High score per level_
* [x] Level editor screne
    * [x] _Editor logic ;)_
    * [x] Edit times
    * [x] Edit orb counts
    * [x] _Test play_
    * [x] Load/Save support
    * [ ] _Check for invalid states_


### The Code

Code is split up into three main parts:

* MacOSHost - A macOS harness capable of running and debugging a simulated version pof the game
* ChromaGrid - The game!
* toybox - The reusable parts that could become many games
    * toystd - Minimal replacements for C++ standard library functionality, optimized for speed and space.

All code must compile with GCC 4.6.4 with c++0x _(Experimental C++11)_ enabled, no standard libraries linked.

 Make no assumption of integer/pointer size. macOS host uses 32 bit integers, target uses 16 bit integers. Whenever possible use explicitly sized types, `int16_t` not `short`.

Use static_asserts to ensure expected sizes for structs are correct. Asserts are enabled on macOS host, but not Atari target. Asserts are used liberly to ensure correctness.
    
Types uses suffix, variables does not:

* `_t` - POD, plain old type.
* `_e` - Enumeration.
* `_s` - Simple structs
    * Must never implement constructors or destructors.
    * For direct access to all members.
* `_c` - Classes
    * Classes __must__ support move semantics.
    * Never assume copy semantics are available.

#### Known libcmini limitations

libcmini-0.47 used is an older version form 2017, and has a few known issues. It is not being updated as to avoid disruptions before Sommarhack 2024.

* `fread` and `fwrite` does not return number of items, but total bytes on success.
* `rand()` does not respect `RAND_MAX` with -mshort, and can return negative values.
* `strncmp()` is just very buggy.

### File Format

#### `levels.dat` - An EA IFF 85 format

Same format is used for builtin levels and user levels. User levels may contain
a maximum of ten level forms.

```
// List of ChromaGrid LeVels
LIST # { CGLV       
    // Zero or more ChromaGrid LeVel forms
    FORM # { CGLV
        // Required LeVel HeaDer
        LVHD # {
            ubyte width
            ubyte height
            ubyte[2] orbs
            uword time
        }
        // Optional text
        TEXT # {
            string
        } ?
        // One or more Tile STateS, count defined by header width * height
        TSTS # {
            {
                ubyte type
                ubyte target
                ubyte current
                ubyte orb
            } +
        }
    } *
}
```

#### `scores.dat` - An EA IFF 85 format
If the number of scores chunks are less than available built in levels the 
remainders are all zero. Having more score chunks than evailable built in levels
is an error :/.

```
// List of ChromaGrid Level Scores
LIST # { CGLR       
    // Zero or more ChromaGrid Level Result chunks
    CGLR # {
        uword score
        ubyte[2] orbs
        uword time
        uword moves
    } *
}
```
