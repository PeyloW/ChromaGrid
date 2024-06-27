//
//  utility.hpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-03-22.
//

#ifndef utility_h
#define utility_h

#include "type_traits.hpp"

namespace toybox {
    
    /*
     This file containes a minimal set of cuntionality from C++ stdlib.
     */

#ifdef __M68000__
#   define hton(v)
#else
    template<class Type, typename enable_if<sizeof(Type) == 1 && is_arithmetic<Type>::value, bool>::type = true>
    static inline void hton(Type &value) { }
    
    template<class Type, typename enable_if<sizeof(Type) == 2 && is_arithmetic<Type>::value, bool>::type = true>
    static void inline hton(Type &value) { value = htons(value); }
    
    template<class Type, typename enable_if<sizeof(Type) == 4 && is_arithmetic<Type>::value, bool>::type = true>
    static void inline hton(Type &value) { value = htonl(value); }
    
    template<class Type, size_t Count>
    static void inline hton(Type (&array)[Count]) {
        for (auto &value : array) {
            hton(&value);
        }
    }
#endif
    
    static inline int sqrt(int x) {
        if (x == 0 || x == 1) {
            return x;
        } else {
            int i = 1, result = 1;
            while (result <= x) {
                i++;
                result = i * i;
            }
            return i - 1;
        }
    }

    static inline uint16_t fletcher16(uint8_t *data, size_t count, uint16_t start = 0) {
        uint16_t sum1 = start & 0xff;
        uint16_t sum2 = (start >> 8) & 0xff;
        while (count--) {
           sum1 = (sum1 + *data++) % 255;
           sum2 = (sum2 + sum1) % 255;
        }
        return (sum2 << 8) | sum1;
    }
    
    template<class C> inline auto begin(C& c) -> decltype(c.begin()) { return c.begin(); };
    template<class C> inline auto begin(const C& c) -> decltype(c.begin()) { return c.begin(); };
    template<class T, size_t N> inline T* begin(T (&array)[N]) { return &array[0]; };
    template<class C> inline auto end(C& c) -> decltype(c.end()) { return c.end(); };
    template<class C> inline auto end(const C& c) -> decltype(c.end()) { return c.end(); };
    template<class T, size_t N> inline T* end(T (&array)[N]) { return &array[N]; };

    template<typename T> constexpr T&& forward(typename remove_reference<T>::type& t) noexcept {
        return static_cast<T&&>(t);
    }
    template<typename T> constexpr T&& forward(typename remove_reference<T>::type&& t) noexcept {
        return static_cast<T&&>(t);
    }
    template<typename T> constexpr typename remove_reference<T>::type&& move(T&& t) noexcept {
        return static_cast<typename remove_reference<T>::type&&>(t);
    }
    
    template<typename T>
    __forceinline void swap(T& a, T& b) {
        T t = move(a);
        a = move(b);
        b = move(t);
    }

    template< class T, class... Args >
    constexpr T* construct_at(T* p, Args&&... args) {
        return new ((void *)p) T(forward<Args>(args)...);
    }
    template<class T> void destroy_at(T* p) { p->~T(); }

    
    // Base class enforcing no copy constructor or assignment.
    class nocopy_c {
    public:
        bool operator==(const nocopy_c &other) const {
            return this == &other;
        }
    protected:
        __forceinline nocopy_c() {}
        nocopy_c(const nocopy_c&) = delete;
        nocopy_c& operator=(const nocopy_c&) = delete;
    };

    template<class T1, class T2>
    class pair_c : nocopy_c {
    public:
        pair_c(const T1 &f, const T2 &s) : first(f), second(s) {}
        T1 first;
        T2 second;
    };

    template< class T1, class T2 >
    pair_c<T1, T1> make_pair( T1&& f, T2&& s) { return pair_c<T1, T2>(forward<T1>(f), forward<T2>(s)); }

}

#endif /* utility_h */
