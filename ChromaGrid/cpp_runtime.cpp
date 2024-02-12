//
//  allocator.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-01.
//

#include "allocator.hpp"

#ifdef __M68000__

void *operator new (size_t n) {
    return allocate(n);
}
void* operator new[] (size_t n) {
    return allocate(n);
}

void operator delete (void* p) noexcept {
    deallocate(p);
}
void operator delete[] (void* p) noexcept {
    deallocate(p);
}

extern "C" void __cxa_pure_virtual() {
    while (1);
}

#endif

void* operator new (size_t count, void *p) {
    return p;
}
void* operator new[] (size_t count, void *p) {
    return p;
}
