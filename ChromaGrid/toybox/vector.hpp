//
//  vector.hpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-03-22.
//

#ifndef vector_h
#define vector_h

#include "algorithm.hpp"

namespace toybox {
    
    /**
     `vector_c` is a minimal implementation of `std::vector` with a statically
     allocated backiong store, for performance reasons.
     TODO: Treat Count of 0 as a dynamic vector.
     */
    template<class Type, int Count>
    class vector_c : public nocopy_c {
    public:
        typedef Type value_type;
        typedef value_type* pointer;
        typedef const value_type* const_pointer;
        typedef value_type& reference;
        typedef const value_type& const_reference;
        typedef value_type* iterator;
        typedef const value_type* const_iterator;
        
        inline vector_c() : _size(0) {}
        
        __forceinline iterator begin() __pure { return _data[0].ptr(); }
        __forceinline const_iterator begin() const __pure { return _data[0].ptr(); }
        __forceinline iterator end() __pure { return _data[_size].ptr(); }
        __forceinline const_iterator end() const __pure { return _data[_size].ptr(); }
        __forceinline int size() const __pure { return _size; }
        
        inline reference operator[](const int i) __pure {
            assert(i < _size);
            assert(i >= 0);
            return *_data[i].ptr();
        }
        inline const_reference operator[](const int i) const __pure {
            assert(i < _size);
            assert(i >= 0);
            return *_data[i].ptr();
        }
        inline reference front() __pure {
            assert(_size > 0);
            return *_data[0].ptr();
        }
        inline const_reference front() const __pure {
            assert(_size > 0);
            return *_data[0].ptr();
        }
        inline reference back() __pure {
            assert(_size > 0);
            return *_data[_size - 1].ptr();
        }
        inline const_reference back() const __pure {
            assert(_size > 0);
            return *_data[_size - 1].ptr();
        }

        inline void push_back(const_reference value) {
            assert(_size < Count);
            *_data[_size++].ptr() = value;
        }
        inline iterator insert(const_iterator pos, const_reference value) {
            assert(_size < Count && pos >= begin() && pos <= end());
            move_backward(pos, end(), end() + 1);
            *pos = value;
            _size++;
            return ++pos;
        }
        template<class... Args>
        inline reference emplace_back(Args&&... args) {
            assert(_size < Count);
            return *new (_data[_size++].addr()) Type(forward<Args>(args)...);
        }
        template<class... Args>
        inline iterator emplace(Type *pos, Args&&... args) {
            assert(_size < Count && pos >= begin() && pos <= end());
            move_backward(pos, end(), end() + 1);
            new ((void *)pos) Type(forward<Args>(args)...);
            _size++;
            return ++pos;
        }
        
        inline void erase(const_iterator pos) {
            assert(_size > 0 && pos >= begin() && pos < end());
            destroy_at(pos);
            move(pos + 1, this->end(), pos);
            _size--;
            return iterator(pos);
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
        aligned_membuf<Type> _data[Count];
        int _size;
    };
    
}

#endif /* vector_h */
