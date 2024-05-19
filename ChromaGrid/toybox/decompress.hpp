//
//  decompress.hpp
//  toybox
//
//  Created by Fredrik on 2024-05-17.
//

#ifndef deflate_hpp
#define deflate_hpp

#include "cincludes.hpp"

namespace toystd {

    size_t decompress_deflate(uint8_t *dest, size_t dest_length, const uint8_t *source, size_t source_length);

}

#endif
