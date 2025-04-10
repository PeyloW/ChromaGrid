//
//  static_allocator.hpp
//  ChromaGrid
//
//  Created by Fredrik Olsson on 2024-03-24.
//

#ifndef static_allocator_h
#define static_allocator_h

#include "algorithm.hpp"

namespace toybox {

/**
 A `static_alloctor_c` is a vaguelly related to `std::allocator`, but always
 allocates single blocks of a static size.
 This exists for performance, as `malloc` can be expensive.
 */
template <class T, size_t Count>
class static_allocator_c {
    struct block_t;
public:
    static const size_t alloc_size = sizeof(T);
    typedef block_t *type;
    static void *allocate() {
        if (first_block == nullptr) init_blocks();
        assert(first_block);
#ifndef __M68000__
        _alloc_count++;
        _max_alloc_count = MAX(_max_alloc_count, _alloc_count);
#endif
        auto ptr = reinterpret_cast<T*>(&first_block->data[0]);
        first_block = first_block->next;
        return ptr;
    };
    static void deallocate(void *ptr) {
#ifndef __M68000__
        _alloc_count--;
#endif
        block_t *block = reinterpret_cast<block_t *>((void **)ptr - 1);
        block->next = first_block;
        first_block = block;
    }
#ifndef __M68000__
    static int max_alloc_count() { return _max_alloc_count; }
#endif
private:
    struct block_t {
        block_t *next;
        uint8_t data[alloc_size];
    };
#ifndef __M68000__
    inline static int _alloc_count = 0;
    inline static int _max_alloc_count = 0;
#endif
    static block_t *first_block;
    static void init_blocks() {
        first_block = reinterpret_cast<block_t *>(_malloc(sizeof(block_t) * Count));
        for (int i = 0; i < Count - 1; i++) {
            first_block[i].next = &first_block[i + 1];
        }
        first_block[Count - 1].next = nullptr;
    }
};

}

#endif /* static_allocator_h */
