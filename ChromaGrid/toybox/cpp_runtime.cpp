//
//  allocator.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-01.
//

#include "cincludes.hpp"

#ifdef __M68000__

void *operator new (size_t n) {
    return malloc(n);
}
void* operator new[] (size_t n) {
    return malloc(n);
}

void operator delete (void* p) noexcept {
    free(p);
}
void operator delete[] (void* p) noexcept {
    free(p);
}

extern "C" void __cxa_pure_virtual() {
    while (1);
}

#endif

void* operator new (size_t count, void *p) noexcept {
    return p;
}
void* operator new[] (size_t count, void *p) {
    return p;
}
