//
//  vector.hpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-03-22.
//

#ifndef vector_h
#define vector_h

#include "stdlib.hpp"

namespace toystd {
    
    // Minimal std::vector replacement with static size
    template<class TYPE, int COUNT>
    class cgvector_c : cgnocopy_c {
    public:
        inline cgvector_c() : _size(0) {}
        
        __forceinline TYPE *begin() const { return (TYPE *)&_buffer[0]; }
        __forceinline TYPE *end() const { return begin() + _size; }
        __forceinline int size() const { return _size; }
        
        inline TYPE& operator[](const int i) const {
            assert(i < _size);
            assert(i >= 0);
            return begin()[i];
        }
        inline TYPE& front() const {
            assert(_size > 0);
            return *begin();
        }
        inline TYPE& back() const {
            assert(_size > 0);
            return *(end() - 1);
        }
        
        inline void push_back() {
            assert(_size < COUNT);
            memset(begin() + _size++, 0, sizeof(TYPE));
        }
        inline void push_back(const TYPE& value) {
            assert(_size < COUNT);
            begin()[_size++] = value;
        }
        template<class... Args>
        inline void emplace_back(Args&&... args) {
            assert(_size < COUNT);
            new (begin() + _size++) TYPE(cgforward<Args>(args)...);
        }
        
        inline void clear() {
            _size = 0;
        }
        inline void pop_back() {
            assert(_size > 0);
            _size--;
        }
        
    private:
        uint8_t _buffer[sizeof(TYPE) * COUNT];
        int _size;
    };
    
}

#endif /* vector_h */
