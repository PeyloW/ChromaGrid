//
//  util_stream.hpp
//  toybox
//
//  Created by Fredrik on 2024-05-19.
//

#ifndef util_stream_hpp
#define util_stream_hpp

#include "stream.hpp"

namespace toybox {
        
    /// The dev/null of streams
    class nullstream_c : public stream_c {
    public:
        nullstream_c();
        virtual ~nullstream_c() {};
        
        virtual ptrdiff_t tell() const __pure;
        virtual ptrdiff_t seek(ptrdiff_t pos, seekdir_e way);

        virtual size_t read(uint8_t *buf, size_t count = 1);
        virtual size_t write(const uint8_t *buf, size_t count = 1);

    };
    
}

#endif /* util_stream_hpp */
