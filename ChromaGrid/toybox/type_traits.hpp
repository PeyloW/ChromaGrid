//
//  type_traits.hpp
//  CodingCpp
//
//  Created by Fredrik Olsson on 3/22/24.
//

#ifndef type_traits_h
#define type_traits_h

extern void* operator new (size_t count, void *p) noexcept;

namespace toystd {
    
    template<bool B, typename T = void> struct enable_if {};
    template<typename T> struct enable_if<true, T> { typedef T type; };
    
    template<class T, T v>
    struct integral_constant {
        static constexpr T value = v;
    };
    template<bool B>
    struct bool_constant : public integral_constant<bool, B> {};
    typedef bool_constant<false> false_type;
    typedef bool_constant<true> true_type;
    
    template<typename T> struct is_integral : public false_type {};
    template<> struct is_integral<bool> : public true_type {};
    template<> struct is_integral<int8_t> : public true_type {};
    template<> struct is_integral<uint8_t> : public true_type {};
    template<> struct is_integral<int16_t> : public true_type {};
    template<> struct is_integral<uint16_t> : public true_type {};
    template<> struct is_integral<int32_t> : public true_type {};
    template<> struct is_integral<uint32_t> : public true_type {};
    
    template<typename T> struct is_floating_point : public false_type {};
    template<> struct is_floating_point<float> : public true_type {};
    template<> struct is_floating_point<double> : public true_type {};
    
    template<typename T> struct is_arithmetic : public bool_constant<is_integral<T>::value || is_floating_point<T>::value> {};
    
    
    template<class T> struct is_reference : false_type {};
    template<class T> struct is_reference<T&> : true_type {};
    template<class T> struct is_reference<T&&> : true_type {};
    
    template<class T> struct remove_const { typedef T type; };
    template<class T> struct remove_const<const T> { typedef T type; };
    
    template<class T> struct remove_reference { typedef T type; };
    template<class T> struct remove_reference<T&> { typedef T type; };
    template<class T> struct remove_reference<T&&> { typedef T type; };
    
    namespace detail {
        template<typename T, typename U = T&&> U declval_imp(int);
        template<typename T> T declval_imp(long);
    }
    template<typename T> auto declval() noexcept -> decltype(detail::declval_imp<T>(0));
    
    namespace detail {
        struct is_constructible_imp
        {
            template<typename T, typename = decltype(declval<T&>().~T())>
            static true_type test(int);
            template<typename>
            static false_type test(...);
        };
        struct is_destructible_imp
        {
            template<typename T, typename = decltype(declval<T&>().~T())>
            static true_type test(int);
            template<typename>
            static false_type test(...);
        };
    }
    
    template<typename T, typename... Args> struct is_constructible : public bool_constant<sizeof(detail::is_constructible_imp::test<T>(0)) == sizeof(true_type)> {};
    template<typename T> struct is_trivially_constructible : public bool_constant<__has_trivial_constructor(T)> {};
    template<typename T> struct is_default_constructible : public bool_constant<is_constructible<T>::value> {};
    template<typename T> struct is_copy_constructible : public is_constructible<T, const T&> {};
    template<typename T> struct is_move_constructible : public is_constructible<T, T&&> {};
    
    template<typename T> struct is_destructible : public bool_constant<sizeof(detail::is_destructible_imp::test<T>(0)) == sizeof(true_type)> {};
    template<typename T> struct is_trivially_destructible : bool_constant<is_destructible<T>::value && __has_trivial_destructor(T)> {};
    
}

#endif /* type_traits_h */