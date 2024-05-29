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
    
    /// A byte order swapping stream
    class swapstream_c : public stream_c {
    public:
        swapstream_c(stream_c *stream);
        virtual ~swapstream_c() {};

        virtual void set_assert_on_error(bool assert);

        virtual bool good() const __pure;
        virtual ptrdiff_t tell() const __pure;
        virtual ptrdiff_t seek(ptrdiff_t pos, seekdir_e way);
        virtual bool flush();

        using stream_c::read;
        virtual size_t read(uint8_t *buf, size_t count = 1);
        virtual size_t read(uint16_t *buf, size_t count = 1);
        virtual size_t read(uint32_t *buf, size_t count = 1);

        using stream_c::write;
        virtual size_t write(const uint8_t *buf, size_t count = 1);
        virtual size_t write(const uint16_t *buf, size_t count = 1);
        virtual size_t write(const uint32_t *buf, size_t count = 1);
    private:
        unique_ptr_c<stream_c> _stream;
    };
   
    /// A buffered wrapper for a stream
    /// NOT RECOMENDED FOR USE YET
    /*
    class bufstream_c : public stream_c {
    public:
        bufstream_c(stream_c *stream, size_t len);
        virtual ~bufstream_c() {};

        size_t length() const { return _len; }
        
        virtual void set_assert_on_error(bool assert);

        virtual bool good() const __pure;
        virtual ptrdiff_t tell() const __pure;
        virtual ptrdiff_t seek(ptrdiff_t pos, seekdir_e way);
        virtual bool flush();

        using stream_c::read;
        virtual size_t read(uint8_t *buf, size_t count = 1);

        using stream_c::write;
        virtual size_t write(const uint8_t *buf, size_t count = 1);
    protected:
        virtual size_t underflow(uint8_t *buf, size_t len);
        virtual size_t overflow(const uint8_t *buf, size_t len);
    private:
        unique_ptr_c<stream_c> _stream;
        unique_ptr_c<uint8_t> _buffer;
        const size_t _len;
        size_t _pos;
        size_t _max;
        bool _mutated;
    };
     */
    
}

#endif /* util_stream_hpp */
