//
//  allocator.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-01.
//

#include "cincludes.hpp"
#include "memory.hpp"

extern "C" {
    FILE *log_file() {
        static FILE *log = nullptr;
        if (log == nullptr) {
            log = fopen("log.txt", "w+");
        }
        return log;
    }
}

template<>
toystd::detail::shared_count_t::allocator::type toystd::detail::shared_count_t::allocator::first_block = nullptr;
 
void *operator new (size_t n) {
    return _malloc(n);
}
void* operator new[] (size_t n) {
    return _malloc(n);
}

void operator delete (void* p) noexcept {
    _free(p);
}
void operator delete[] (void* p) noexcept {
    _free(p);
}

#ifdef __M68000__

extern "C" void __cxa_pure_virtual() {
    hard_assert(0);
}

// WARNING: Thes einit guards are NOT threadsafe, never init statics on timer callbacks.
#define GUARD_DONE      0x01
#define GUARD_PENDING   0x02
typedef uint8_t __guard;
extern "C" int __cxa_guard_acquire(__guard* g) {
    if (*g & GUARD_DONE) {
        return 0;
    }
    *g = GUARD_PENDING;
    return 1;
}
extern "C" void __cxa_guard_release(__guard* g) {
    *g = GUARD_DONE;
}
extern "C" void __cxa_guard_abort(__guard* g) {
    *g = 0;
}

#endif

void* operator new (size_t count, void *p) noexcept {
    return p;
}
void* operator new[] (size_t count, void *p) {
    return p;
}
