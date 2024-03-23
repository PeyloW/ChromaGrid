//
//  utility.hpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-03-22.
//

#ifndef utility_h
#define utility_h

namespace toystd {
    
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
    
}

#endif /* utility_h */
