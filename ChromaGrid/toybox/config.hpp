//
//  config.hpp
//  toybox
//
//  Created by Fredrik on 2024-04-20.
//

// 0 = Not Atari target
// 1 = STfm
// 2 = STe
// 3 = TT
// 4 = Falcon030
#ifndef TOYBOX_TARGET_ATARI
#   define TOYBOX_TARGET_ATARI 2
#endif

// 0 = Not Amiga target
// 1 = OSC
// 2 = ECS
// 3 = AGA
#ifndef TOYBOX_TARGET_AMIGA
#   define TOYBOX_TARGET_AMIGA 0
#endif

#define TOYBOX_ASSET_COUNT 32

#ifndef TOYBOX_LOG_MALLOC
#   define TOYBOX_LOG_MALLOC 0
#endif

#ifndef TOYBOX_IMAGE_SUPPORTS_SAVE
#   ifdef __M68000__
#       define TOYBOX_IMAGE_SUPPORTS_SAVE 0
#   else
#       define TOYBOX_IMAGE_SUPPORTS_SAVE 1
#   endif
#endif

#ifndef TOYBOX_SCREEN_SIZE_MAX
#   define TOYBOX_SCREEN_SIZE_MAX size_s(320, 200)
#endif

#ifndef TOYBOX_SCREEN_SIZE_DEFAULT
#   define TOYBOX_SCREEN_SIZE_DEFAULT size_s(320, 200)
#endif

#ifndef TOYBOX_DEBUG_DIRTYMAP
#   ifdef __M68000__
#       define TOYBOX_DEBUG_DIRTYMAP 0
#   else
#       define TOYBOX_DEBUG_DIRTYMAP 0
#   endif
#endif

#ifndef TOYBOX_DIRTYMAP_TILE_SIZE
#   define TOYBOX_DIRTYMAP_TILE_SIZE size_s(16, 16)
#endif

#ifndef TOYBOX_DEBUG_RESTORE_SCREEN
#   ifdef __M68000__
#       define TOYBOX_DEBUG_RESTORE_SCREEN 0
#   else
#       define TOYBOX_DEBUG_RESTORE_SCREEN 1
#   endif
#endif

#ifndef TOYBOX_RESERVE_A6_GLOBAL
#   define TOYBOX_RESERVE_A6_GLOBAL 1
#endif

#if TOYBOX_RESERVE_A6_GLOBAL
extern "C" {
#   ifdef __M68000__
    register void *g_global_ptr __asm__("a6");
#   else
    extern void *g_global_ptr;
#   endif
}
#endif
