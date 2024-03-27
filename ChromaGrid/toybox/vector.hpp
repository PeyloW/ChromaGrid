//
//  vector.hpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-03-22.
//

#ifndef vector_h
#define vector_h

#include "algorithm.hpp"

namespace toystd {
    
    // Minimal std::vector replacement with static size
    template<class TYPE, int COUNT>
    class vector_c : nocopy_c {
    public:
        inline vector_c() : _size(0) {}
        
        __forceinline TYPE *begin() const { return _data[0].ptr(); }
        __forceinline TYPE *end() const { return _data[_size].ptr(); }
        __forceinline int size() const { return _size; }
        
        inline TYPE& operator[](const int i) const {
            assert(i < _size);
            assert(i >= 0);
            return *_data[i].ptr();
        }
        inline TYPE& front() const {
            assert(_size > 0);
            return *_data[0].ptr();
        }
        inline TYPE& back() const {
            assert(_size > 0);
            return *_data[_size - 1].ptr();
        }
        
        inline void push_back(const TYPE& value) {
            assert(_size < COUNT);
            *_data[_size++].ptr() = value;
        }
        inline void insert(TYPE *pos, const TYPE &value) {
            assert(_size < COUNT && pos >= begin() && pos <= end());
            move_backward(pos, end(), end() + 1);
            *pos = value;
            _size++;
        }
        template<class... Args>
        inline void emplace_back(Args&&... args) {
            assert(_size < COUNT);
            new (_data[_size++].addr()) TYPE(forward<Args>(args)...);
        }
        template<class... Args>
        inline void emplace(TYPE *pos, Args&&... args) {
            assert(_size < COUNT && pos >= begin() && pos <= end());
            move_backward(pos, end(), end() + 1);
            new ((void *)pos) TYPE(forward<Args>(args)...);
            _size++;
        }
        
        inline void erase(TYPE *pos) {
            assert(_size > 0 && pos >= begin() && pos < end());
            destroy_at(pos);
            move(pos + 1, this->end(), pos);
            _size--;
        }
        inline void clear() {
            while (_size) {
                destroy_at(_data[--_size].ptr());
            }
        }
        inline void pop_back() {
            assert(_size > 0);
            destroy_at(_data[--_size].ptr());
        }
        
    private:
        aligned_membuf<TYPE> _data[COUNT];
        int _size;
    };
    
}

#endif /* vector_h */
