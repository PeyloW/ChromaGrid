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

* toybox 1.1


### Project aknowledgements

* Hatari 1.0 as reference for host blitter emulation (https://github.com/hatari/hatari)
* Royalty free sound effects from ZapSplat (https://www.zapsplat.com/)
* Blitter and audio setup code inspired by GODLib (https://github.com/ReservoirGods/GODLIB).


### The Code

Code is split up into three main parts:

* ChromaGrid - The game! This repository
* toybox - The reusable parts that could become many games
    * Minimal replacements for C++ standard library functionality, optimized for speed and space.
    * Primitives for machine, graphics and audio.
    * A scene based game system for managing game creation.
    

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
