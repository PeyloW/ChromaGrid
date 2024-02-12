//
//  allocator.hpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-01.
//

#ifndef allocator_h
#define allocator_h

#include "cincludes.hpp"

class allocator_imp_base {
public:
    typedef size_t size_type;
    static const size_type MIN_BLOCK_SIZE = 8;
    static const size_type MAX_BLOCK_SIZE = 4096;
    static const size_type BLOCK_SIZE_COUNT = 11;
#ifndef __M68000__
    size_type alloc_count;
    size_type max_alloc_count;
#endif
    
    template <size_type S = MIN_BLOCK_SIZE>
    constexpr static int alloctor_index(const size_type n, int i = 0) {
        return (S <= MAX_BLOCK_SIZE && S < n) ? alloctor_index<S * 2>(n, i + 1) : i;
    }
    
    virtual void *allocate(const size_type n) = 0;
    virtual void deallocate(void *p) = 0;
};


template <size_t SIZE>
class allocator_imp : public allocator_imp_base {
    static const size_type MAX_BUFFER_SIZE = 8192;
    static const size_type MAX_BLOCK_COUNT = MAX_BUFFER_SIZE / SIZE;
    static const size_type MIN_BLOCK_COUNT = 2;
    static_assert(MAX_BLOCK_COUNT >= MIN_BLOCK_COUNT, "");
    
    struct block_t {
        block_t *next;
    };

    struct buffer_t {
        static const size_type BLOCK_SIZE = SIZE > sizeof(block_t) ? SIZE : sizeof(block_t);
        uint8_t data[BLOCK_SIZE * MAX_BLOCK_COUNT];
        void *get_block(size_type index) {
            return reinterpret_cast<void *>(&data[MAX_BLOCK_COUNT * index]);
        }
    };
    
    block_t *first_free_block;
    buffer_t *first_buffer;
    size_type available_blocks;

public:
    allocator_imp() : first_free_block(nullptr), first_buffer(nullptr), available_blocks(0) {}

    virtual void *allocate(const size_type n) {
#ifndef __M68000__
        assert(n <= SIZE);
        alloc_count++;
        max_alloc_count = MAX(max_alloc_count, alloc_count);
#endif
        if (first_free_block) {
            block_t *block = first_free_block;
            first_free_block = block->next;
            return block;
        }

        if (available_blocks == 0) {
            first_buffer = new (malloc(sizeof(buffer_t))) buffer_t();
            available_blocks = MAX_BLOCK_COUNT;
        }

        return first_buffer->get_block(--available_blocks);
    }
    
    virtual void deallocate(void *p) {
#ifndef __M68000__
        assert(alloc_count > 0);
        alloc_count--;
#endif
        block_t *block = reinterpret_cast<block_t *>(p);
        block->next = first_free_block;
        first_free_block = block;
    }
};

template <>
class allocator_imp<0> : public allocator_imp_base {
public:
    virtual void *allocate(const size_type n) {
#ifndef __M68000__
        assert(n > MAX_BLOCK_SIZE);
        alloc_count++;
        max_alloc_count = MAX(max_alloc_count, alloc_count);
#endif
        return malloc(n);
    }
    virtual void deallocate(void *p) {
#ifndef __M68000__
        assert(alloc_count > 0);
        alloc_count--;
#endif
        free(p);
    }
};

static allocator_imp_base *allocator_imps[allocator_imp_base::BLOCK_SIZE_COUNT] = {
    new allocator_imp<8>(),
    new allocator_imp<16>(),
    new allocator_imp<32>(),
    new allocator_imp<64>(),
    new allocator_imp<128>(),
    new allocator_imp<256>(),
    new allocator_imp<512>(),
    new allocator_imp<1024>(),
    new allocator_imp<2048>(),
    new allocator_imp<4096>(),
    new allocator_imp<0>(),
};

template <class T>
class allocator {
public:
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef T *pointer;
    typedef const T *const_pointer;
    typedef T &reference;
    typedef const T &const_reference;
    typedef T value_type;
    
    static const size_type SIZE_OF_TYPE = sizeof(T);
    
    template <class U>
    struct rebind {
        typedef allocator<U> other;
    };
    
    pointer allocate(size_type n, const void *hint = 0) {
        int i;
        size_type s;
        if (n == 1) {
            s = SIZE_OF_TYPE;
            i = allocator_imp_base::alloctor_index(SIZE_OF_TYPE);
        } else {
            s = sizeof(T) * n;
            i = allocator_imp_base::alloctor_index(s);
        }
        return reinterpret_cast<pointer>(allocator_imps[i]->allocate(s));
    }
    
    void deallocate(pointer p, size_type n) {
        int i;
        size_type s;
        if (n == 1) {
            s = SIZE_OF_TYPE;
            i = allocator_imp_base::alloctor_index(SIZE_OF_TYPE);
        } else {
            s = sizeof(T) * n;
            i = allocator_imp_base::alloctor_index(s);
        }
        return allocator_imps[i]->deallocate(p);
    }
    
    void construct(pointer p, const_reference val) {
        new (p) T(val);
    }
    
    void destroy(pointer p) {
        p->~T();
    }

};

static void *allocate(const size_t n) {
    const size_t s = n + sizeof(size_t);
    const auto imp = allocator_imps[allocator_imp_base::alloctor_index(s)];
    size_t *rp = reinterpret_cast<size_t *>(imp->allocate(s));
    *rp++ = s;
    return rp;
}

static void deallocate(void *p) {
    size_t *rp = reinterpret_cast<size_t *>(p);
    const size_t s = *(--rp);
    const auto imp = allocator_imps[allocator_imp_base::alloctor_index(s)];
    imp->deallocate(reinterpret_cast<uint8_t *>(rp));
}


#endif /* allocator_h */
