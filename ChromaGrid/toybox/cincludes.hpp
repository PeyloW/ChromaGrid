//
//  cincludes.hpp
//  ToyBox
//
//  Created by Fredrik on 2023-12-30.
//  Copyright © 2023 TOYS. All rights reserved.
//

#ifndef cincludes_h
#define cincludes_h

extern "C" {
    
#define __forceinline __attribute__((__always_inline__))
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
#include <assert.h>
#include <ctype.h>
        
}

#endif /* cincludes_h */