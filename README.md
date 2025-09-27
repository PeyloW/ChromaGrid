# Chroma Grid

An Atari ST<sup>E</sup> game by T.O.Y.S. released at Sommarhack 2024.

Chroma Grid is a historical preservation project dedicated to preserving a significant piece of digital heritage. By bringing an unfinished game from 1994 to life, this project serves as both a study and an educational resource on development for the Atari ST. 

The Atari ST family of computers was a pioneer in the 16-bit era and played a crucial role in the advent of home computing. Preserving and understanding the capabilities and workings of this technology is essential for comprehending the evolution of computer technology up to the present day, and where it may take us in the future.


## Credits

* Code: _Fredrik 'PeyloW' Olsson_
* Music: _Joakim 'AiO' Ekblad_
* Graphics: Hervé 'Exocet' Piton
* Font: _Damien Guard_
* Concept: Peter 'Eagle' Nyman


### Project Requirement

* GCC-4.6.4 with fastcall support (https://github.com/PeyloW/gcc-4.6.4)
* libcmini-0.47 (https://github.com/freemint/libcmini)


### Project aknowledgements

* Hatari 1.0 as reference for host blitter emulation (https://github.com/hatari/hatari)
* Royalty free sound effects from ZapSplat (https://www.zapsplat.com/)
* Blitter and audio setup code inspired by GODLib (https://github.com/ReservoirGods/GODLIB).


### The Code

Code is split up into three main parts:

* MacOSHost - A macOS host harness capable of running and debugging a simulated version pof the game
* ChromaGrid - The game!
* toybox - The reusable parts that could become many games
    * Minimal replacements for C++ standard library functionality, optimized for speed and space.
    * Primitives for machine, graphics and audio.
    * A scene based game system for managing game creation.

All code must compile with GCC 4.6.4 with c++0x _(Experimental C++11)_ enabled, no standard libraries linked!

 Make no assumption of integer/pointer size. macOS host uses 32 bit integers, target uses 16 bit integers. Whenever possible use explicitly sized types, `int16_t` not `short`.

Use `static_assert` to ensure expected sizes for structs are correct. Asserts are enabled on macOS host, but not Atari target. Asserts are used liberly to ensure correctness.
    
Types uses suffix, variables does not:

* `_t` - POD, plain old type.
* `_e` - Enumeration.
* `_s` - Simple structs
    * Must never implement constructors or destructors.
    * For direct access to all members.
* `_c` - Classes
    * Classes **must** support move semantics.
    * **Never** assume copy semantics are available.
        * Most classes explicitly forbig copy semntions by use of `nocopy_c` subclassing.


#### Known libcmini limitations

libcmini-0.47 used is an older version from 2017, and has a few known issues. It is not being updated as to avoid disruptions before Sommarhack 2024.

* `fread` and `fwrite` does not return number of items, but total bytes on success.
* `rand()` does not respect `RAND_MAX` with `-mshort`, and can return negative values.
* `strncmp()` is just very buggy.


### File Format

#### `levels.dat` - An EA IFF 85 format

Same format is used for builtin levels and user levels. User levels may contain a maximum of ten level forms.

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
If the number of scores chunks are less than available built in levels the  remainders are all zero. Having more score chunks than evailable built in levels is an error :/. `f16check` is a Fletcher16 checksum of the level recipies  header+tiles, not text.

```
// List of ChromaGrid Level Scores
LIST # { CGLR       
    // Zero or more ChromaGrid Level Result chunks
    CGLR # {
        uword score
        ubyte[2] orbs
        uword time
        uword moves
        uword f16check
    } *
}
```
