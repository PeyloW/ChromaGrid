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
extern void* operator new (size_t count, void *p) noexcept;

namespace toystd {
    
    typedef decltype(nullptr) nullptr_t;

    // Base class enforcing no copy constructor or assignment.
    class cgnocopy_c {
    protected:
        __forceinline cgnocopy_c() {}
        cgnocopy_c(const cgnocopy_c&) = delete;
        cgnocopy_c& operator=(const cgnocopy_c&) = delete;
    };

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

}

#endif /* stdlib_h */
