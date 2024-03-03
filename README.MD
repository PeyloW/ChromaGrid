# Chroma Grid

An Atari STe game by T.O.Y.S. to be release at Sommarhack 2024.

## Credits

* Main Code: _Fredrik 'PeyloW' Olsson_
* Blitter Code: _Hatari 1.0 as reference_
* Music: _Joakim 'AiO' Ekblad_
* Graphics: ??
* Font: _Damien Guard_
* Concept: Peter 'Eagle' Nyman

### Project Requirement

* GCC-4.6.4 with fastcall support (https://github.com/PeyloW/gcc-4.6.4)
* libcmini-0.47 (https://github.com/freemint/libcmini)

### TODO: List

Legend: __Required__, Expected , _Optional_

* [x] __System__
    * [x] __MacOS Host app harness__
    * [x] __Target machine bootstrap__
    * [x] __VBL timer support__
    * [x] __Mouse interups__
* [ ] __Basic file loading__
    * [x] __IFF Image files, with masks__
        * [x] __Load fonts__
        * [x] _Proportinal fonts_
    * [x] __SNDH music files__
    * [ ] AIF samples
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
        * [ ] Asm and HW optimized
* [ ] __Sound__
    * [x] __Play SNDH music__
        * [x] Change tracks
    * [ ] __Play LCM samples__
        * [ ] _Mix several channels_
* [ ] __Scene management__
    * [x] __Shared resources manager__
    * [x] __A stage manager - push-pop scenes__
        * [x] _Scene transitions_
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
* [ ] __Help scene__
    * [ ] Goal of the game
        * [ ] Tile types
        * [ ] Orb fusing        
    * [ ] _Scoring_
* [ ] __Gameplay scene__
    * Re-use right side background
    * [x] __16x16 tiles, 5x types__
        * [ ] Animations between states
    * [x] __8x8 orbs sprites__
        * [x] Animations between states
    * [ ] __Game logic ;)__
        * [ ] __Count down timer__
    * [ ] _Per level scroll text_
    * [ ] In game music, _one or more tracks_
* [ ] Game over scene
    * [ ] Large Game Over logo
    * [ ] Game over game music 
* [ ] _Highscore screne_
    * [ ] _Name entry_
    * [ ] _High score per level_
* [ ] _Level editor screne_
    * [ ] _Editor logic ;)_
    * [ ] _Test play_
    * [ ] _Save support_
