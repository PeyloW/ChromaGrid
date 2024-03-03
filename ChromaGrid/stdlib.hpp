//
//  stdlib.hpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-03-03.
//

#ifndef stdlib_h
#define stdlib_h

#include "cincludes.hpp"

// Required for inplace new
extern void* operator new (size_t count, void *p);

// Require for range for loops
/*
template<class C> __forceinline auto begin(C &c) -> decltype(c.begin()) { return c.begin(); };
template<class C> __forceinline auto begin(const C &c) -> decltype(c.begin()) { return c.begin(); };
template<class C> __forceinline auto end(C &c) -> decltype(c.begin()) { return c.begin(); };
template<class C> __forceinline auto end(const C &c) -> decltype(c.begin()) { return c.begin(); };
*/

// Required for argument forwarding for emplace
template<typename T> struct cgremove_reference      { typedef T type; };
template<typename T> struct cgremove_reference<T&>  { typedef T type; };
template<typename T> struct cgremove_reference<T&&> { typedef T type; };
template <class T>
__forceinline T&& cgforward(typename cgremove_reference<T>::type& t) {
    return static_cast<T&&>(t);
}
template <class T>
__forceinline T&& cgforward(typename cgremove_reference<T>::type&& t) {
    return static_cast<T&&>(t);
}

// Base class enforcing no copy constructor or assignment.
class cgnocopy_c {
protected:
    __forceinline cgnocopy_c() {}
    cgnocopy_c(const cgnocopy_c&) = delete;
    cgnocopy_c& operator=(const cgnocopy_c&) = delete;
};

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


#endif /* stdlib_h */
