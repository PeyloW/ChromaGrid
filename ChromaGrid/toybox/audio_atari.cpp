//
//  audio.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-18.
//

#include "audio.hpp"
#include "system_helpers.hpp"

#if !TOYBOX_TARGET_ATARI
#   error "For Atari target only"
#endif

using namespace toybox;


// libcmini (version used) has a buggy strncmp :(
static int _strncmp(const char *s1, const char *s2, size_t max)
{
    int cmp = 0;
    while (cmp == 0 && max && *s1) {
        cmp = *(unsigned char *)s1 - *(unsigned char *)s2;
        s1++; s2++;
        max--;
    }
    return cmp;
}

ymmusic_c::ymmusic_c(const char *path) {
    fstream_c file(path);
    hard_assert(file.good());
    file.seek(0, stream_c::end);
    size_t size = file.tell();
    file.seek(0, toybox::fstream_c::beg);
    
    _sndh.reset((uint8_t *)_malloc(size));
    _length = size;
    size_t read = file.read(_sndh.get(), size);
    assert(read == size);
    assert(memcmp(_sndh + 12, "SNDH", 4) == 0);
    _title = nullptr;
    _composer = nullptr;
    _track_count = 1;
    _freq = 50;
    char *header_str = (char *)(_sndh + 16);
    while (_strncmp(header_str, "HDNS", 4) != 0 && ((uint8_t*)header_str < _sndh + 200)) {
        int len = (int)strlen(header_str);
         if (len > 0) {
            if (len > 100) {
                break;
            }
            if (_strncmp(header_str, "TITL", 4) == 0) {
                _title = header_str + 4;
            } else if (_strncmp(header_str, "COMM", 4) == 0) {
                _composer = header_str + 4;
            } else if (strncmp(header_str, "##", 2) == 0) {
                _track_count = atoi(header_str + 2);
            } else if (_strncmp(header_str, "TA", 2) == 0 ||
                       _strncmp(header_str, "TB", 2) == 0 ||
                       _strncmp(header_str, "TC", 2) == 0 ||
                       _strncmp(header_str, "TD", 2) == 0 ||
                       _strncmp(header_str, "!V", 2) == 0) {
                _freq = atoi(header_str + 2);
                assert(_freq != 0);
            }
        }
        header_str += len;
        while (*++header_str == 0);
    }
#ifdef __M68000__
    codegen_s::make_trampoline(_music_init_code, _sndh + 0, false);
    codegen_s::make_trampoline(_music_exit_code, _sndh + 4, false);
    codegen_s::make_trampoline(_music_play_code, _sndh + 8, false);
#endif
}
