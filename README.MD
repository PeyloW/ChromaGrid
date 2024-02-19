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

* [x] System
    * [x] MacOS Host app harness
    * [x] Target machine bootstrap
    * [x] VBL timer support
    * [x] Mouse interups
* [ ] Basic file loading
    * [x] IFF Image files, with masks
        * [ ] Load fonts
    * [x] SNDH music files
    * [ ] AIF samples
* [ ] Graphics
    * [x] Display physical screen
        * [x] _STe HW scrolling_
    * [x] Set & Get pixel
        * [ ] Asm optimized
        * [ ] _Color replacement_
    * [x] Blit word aligned grapcs
        * [x] Asm and HW optimized
    * [x] Blit any alignment with clipping
        * [x] With optional masking
        * [x] Asm and HW optimized
    * [ ] Draw text
        * [ ] Optimized numerics
    * [ ] Add optional dirty mask to all draw operations
        * [ ] Restore from dirty mask
        * [ ] Asm and HW optimized
* [ ] Sound
    * [x] Play SNDH music
        * [x] Change tracks
    * [ ] Play LCM samples
        * [ ] _Mix several channels_
* [ ] Scene management
    * [ ] Shared resources manager
    * [ ] A stage manager - push-pop scenes
        * [ ] _Scene transitions_
    * [ ] Generic scene
* [ ] Introduction scene
    * [ ] T.O.Y.S. logo
    * [ ] Large Chroma Grid logo
    * [ ] _Intricate background_
    * [ ] Main music _(Thalion intro rip-off?)_
* [ ] Main menu screne
    * [x] 12x16 muse cursor
    * [ ] Right side bar background 128px wide
        * [ ] Small Chroma Grid Logo
        * [ ] Button shapes, on and off
    * [ ] Level selection grid, center
* [ ] Gameplay screne
    * Re-use right side background
    * [ ] 16x16 tiles, 5x types
        * [ ] Animations between states
    * [ ] 8x8 orbs sprites
        * [ ] Animations between states
    * [ ] Game logic ;)
        * [ ] Count down timer
    * [ ] _Per level scroll text_
    * [ ] In game music, _one or more tracks_
* [ ] Game over screne
    * [ ] Large Game Over logo
    * [ ] Game over game music 
* [ ] _Highscore screne
    * [ ] _Name entry_
    * [ ] _High score per level_
* [ ] _Level editor screne_
    * [ ] _Editor logic ;)_
    * [ ] _Test play_
    * [ ] _Save support_
