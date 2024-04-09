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
        inline void insert(const_iterator pos, const_reference value) {
            assert(_size < Count && pos >= begin() && pos <= end());
            move_backward(pos, end(), end() + 1);
            *pos = value;
            _size++;
        }
        template<class... Args>
        inline void emplace_back(Args&&... args) {
            assert(_size < Count);
            new (_data[_size++].addr()) Type(forward<Args>(args)...);
        }
        template<class... Args>
        inline void emplace(Type *pos, Args&&... args) {
            assert(_size < Count && pos >= begin() && pos <= end());
            move_backward(pos, end(), end() + 1);
            new ((void *)pos) Type(forward<Args>(args)...);
            _size++;
        }
        
        inline void erase(const_iterator pos) {
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
        aligned_membuf<Type> _data[Count];
        int _size;
    };
    
}

#endif /* vector_h */
