# Chroma Grid

An Atari STe game by T.O.Y.S. to be release at Sommarhack 2024.

## Credits

* Main Code: _Fredrik 'PeyloW' Olsson_
* Blitter Code: _Leon 'Mr. Pink' O'Reilly_
* Music: _Joakim 'AiO' Ekblad_
* Graphics: ??

### Project Requirement

* GCC-4.6.4 with fastcall support (https://github.com/PeyloW/gcc-4.6.4)
* libcmini-0.47 (https://github.com/freemint/libcmini)

### TODO: List

Legend:
* __Required__
* Expected
* _Optional_

* [x] __System__
    * [x] __MacOS Host app harness__
    * [x] __Target machine bootstrap__
    * [x] __VBL timer support__
    * [x] __Mouse interups__
* [ ] __Basic file loading__
    * [x] __IFF Image files, with masks__
        * [ ] __Load fonts__
    * [x] __SNDH music files__
    * [ ] AIF samples
* [ ] __Graphics__
    * [x] __Display physical screen__
        * [x] _STe HW scrolling_
    * [x] __Set & Get pixel__
        * [ ] Asm optimized
        * [ ] _Color replacement_
    * [x] __Blit word aligned grapcs__
        * [x] Asm and HW optimized
    * [x] __Blit any alignment with clipping__
        * [x] __With optional masking__
        * [x] Asm and HW optimized
    * [ ] __Draw text__
        * [ ] Optimized numerics
    * [ ] Add optional dirty mask to all draw operations
        * [ ] Restore from dirty mask
        * [ ] Asm and HW optimized
* [ ] __Sound__
    * [x] __Play SNDH music__
        * [x] Change tracks
    * [ ] __Play LCM samples__
        * [ ] _Mix several channels_
* [ ] __Scene management__
    * [ ] __Shared resources manager__
    * [ ] __A stage manager - push-pop scenes__
        * [ ] _Scene transitions_
    * [ ] Generic scene
* [ ] Introduction scene
    * [ ] T.O.Y.S. logo
    * [ ] Large Chroma Grid logo
    * [ ] _Intricate background_
    * [ ] Main music _(Thalion intro rip-off?)_
* [ ] __Main menu screne__
    * [x] __12x16 muse cursor__
    * [ ] __Right side bar background 128px wide__
        * [ ] __Small Chroma Grid Logo__
        * [ ] __Button shapes, on and off__
    * [ ] Level selection grid, center
* [ ] __Gameplay screne__
    * Re-use right side background
    * [ ] __16x16 tiles, 5x types__
        * [ ] Animations between states
    * [ ] __8x8 orbs sprites__
        * [ ] Animations between states
    * [ ] __Game logic ;)__
        * [ ] __Count down timer__
    * [ ] _Per level scroll text_
    * [ ] In game music, _one or more tracks_
* [ ] Game over screne
    * [ ] Large Game Over logo
    * [ ] Game over game music 
* [ ] _Highscore screne_
    * [ ] _Name entry_
    * [ ] _High score per level_
* [ ] _Level editor screne_
    * [ ] _Editor logic ;)_
    * [ ] _Test play_
    * [ ] _Save support_