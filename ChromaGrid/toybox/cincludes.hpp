//
//  cincludes.hpp
//  ToyBox
//
//  Created by Fredrik on 2023-12-30.
//  Copyright © 2023 TOYS. All rights reserved.
//

#ifndef cincludes_h
#define cincludes_h

#include "config.hpp"

extern "C" {
    
#define __pure __attribute__ ((pure))
#define __forceinline __attribute__((__always_inline__))
#define __neverinline __attribute__((noinline))
#define __packed __attribute__((packed))
#define __packed_struct __attribute__((packed, aligned(2)))

#ifndef MAX
#   define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#   define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#endif

#ifndef ABS
#   define ABS(X) (((X) < 0) ? -(X) : (X))
#endif
      
#ifdef __M68000__
#define hard_assert(expr)\
    ((void)((expr)||(fprintf(stderr, \
    "\nHard assert [%d]: (%s), in %s: %d\n", errno,\
     #expr, __FILE__, __LINE__ ),\
     fgetc(stdin))))
#else
#define hard_assert assert
#endif

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <ctype.h>
#include <time.h>
}

extern "C" {

    FILE *log_file();
#if TOYBOX_LOG_MALLOC
    extern FILE *g_malloc_log;
    static void *_malloc(size_t n) {
        auto b = (Malloc(-1));
        auto p = malloc(n);
        auto a = (Malloc(-1));
        fprintf(log_file(), "Malloc %ld [%ld -> %ld].\n", n, b, a);
        hard_assert(p != nullptr);
        return p;
    }
    
    static void *_calloc(size_t c, size_t n) {
        auto b = (Malloc(-1));
        auto p = calloc(c, n);
        auto a = (Malloc(-1));
        fprintf(log_file(), "Calloc %ld [%ld -> %ld].\n", (c * n), b, a);
        hard_assert(p != nullptr);
        return p;
    }
    static void *_free(void *p) {
        auto b = (Malloc(-1));
        free(p);
        auto a = (Malloc(-1));
        fprintf(log_file(), "Free [%ld -> %ld].\n", b, a);
    }
#else
#   define _malloc malloc
#   define _calloc calloc
#   define _free free
#endif
}

// Required for inplace new
extern void* operator new (size_t count, void *p) noexcept;
namespace toystd {
    typedef decltype(nullptr) nullptr_t;
}

#endif /* cincludes_h */
