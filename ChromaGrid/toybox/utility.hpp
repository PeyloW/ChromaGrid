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
    
    void hton_struct(void *ptr, const char *layout);
    template<typename T, typename enable_if<is_class<T>::value, bool>::type = true>
    static void inline hton(T &value) {
        hton_struct(&value, struct_layout<T>::value);
    }

    template<class Type>
    static void inline hton(Type *buf, size_t count) {
        while (count--) {
            hton(*buf);
            buf++;
        }
    }
    template<class Type, size_t Count>
    static void inline hton(Type (&array)[Count]) {
        hton(&array[0], Count);
    }
#endif
    
    static inline __pure int sqrt(int x) {
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

    /**
     Blue noise random number series with 256 index repeat. 
     Number are in range 0..63 inclusive.
     */
    static inline __pure int brand(int idx) {
        const static uint8_t s_blue[256] = {
            10,  2, 17, 23, 10, 34,  4, 28, 37,  2, 19,  7,  3,  1,  5, 62,
             0,  8, 55, 46,  1, 61, 19,  0,  1,  9, 45, 25, 34, 16,  0, 24,
            13, 28,  5,  0,  3, 26, 12,  6, 42, 14, 59,  0, 11,  2, 50, 42,
             1, 37, 20, 14, 32,  8,  2, 53, 22,  3,  0, 29,  4, 56, 18,  3,
             0, 57,  2,  6, 50,  0, 38,  0, 31,  8, 17, 39,  1,  6, 32,  9,
            47, 23, 10,  1, 42, 21, 16,  4, 58,  5, 48,  2, 13, 21,  0, 15,
             5, 35,  0, 63,  4,  1, 28, 11,  0,  1, 25,  9, 62, 27, 41,  2,
             8, 29, 18, 12, 26,  7, 53, 14, 43, 19, 36,  0,  4,  0, 54,  1,
            51,  0,  3,  0, 39,  2,  0, 34,  6,  3, 10, 46, 16,  6, 12, 20,
            40, 14, 57, 22, 47, 10,  1, 59, 24,  0, 52, 30,  2, 36, 26,  0,
             9, 32,  2,  6, 16, 31, 20,  4,  1, 15,  7, 21,  1, 60,  7,  3,
             0,  4, 45,  1,  0,  3, 49, 12, 35, 55,  0,  4, 13,  0, 49, 17,
            58, 27, 11, 36, 61,  8, 18,  0, 40, 27,  9, 44, 33,  5, 38, 23,
             0,  7, 19,  0, 24, 29,  2,  0,  6,  3,  1, 17, 25,  2, 11,  1,
            43, 52,  3, 13,  5, 54, 44, 11, 22, 63, 31,  0, 56,  8,  0, 15,
             4, 33,  0, 41,  1,  7,  0, 15, 51,  5,  0, 12, 48, 39, 21, 30
        };
        return s_blue[idx & 0xff];
    };
    
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
        return new (static_cast<void *>(p)) T(forward<Args>(args)...);
    }
    template<class T> inline  void destroy_at(T* p) { p->~T(); }

    
    // Base class enforcing no copy constructor or assignment.
    class nocopy_c {
    public:
        inline  bool operator==(const nocopy_c &other) const {
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
    inline pair_c<T1, T1> make_pair( T1&& f, T2&& s) { return pair_c<T1, T2>(forward<T1>(f), forward<T2>(s)); }

}

#endif /* utility_h */
