//
//  stream.hpp
//  toybox
//
//  Created by Fredrik on 2024-04-15.
//

#ifndef stream_hpp
#define stream_hpp

#include "cincludes.hpp"
#include "utility.hpp"
#include "memory.hpp"

namespace toystd {
    
    class stream_c : public nocopy_c {
    public:
        typedef enum __packed {
            beg = SEEK_SET,
            cur = SEEK_CUR,
            end = SEEK_END
        } seekdir_e;
        typedef stream_c&(*manipulator_f)(stream_c&);
        
        stream_c();
        virtual ~stream_c() {}
            
        virtual void set_assert_on_error(bool assert);

        virtual bool good() const __pure;
        virtual ptrdiff_t tell() const __pure = 0;
        virtual ptrdiff_t seek(ptrdiff_t pos, seekdir_e way) = 0;
        virtual bool flush();

        virtual size_t read(uint8_t *buf, size_t count = 1) = 0;
        virtual size_t read(uint16_t *buf, size_t count = 1);
        virtual size_t read(uint32_t *buf, size_t count = 1);
        virtual bool read(void *buf, const char *layout);
        template<typename T, typename enable_if<is_class<T>::value, bool>::type = true>
        bool read(T *buf) { return read((void *)buf, struct_layout<T>::value); };
        __forceinline size_t read(int8_t *buf, size_t count = 1) { return read((uint8_t*)buf, count); };
        __forceinline size_t read(int16_t *buf, size_t count = 1) { return read((uint16_t*)buf, count); };
        __forceinline size_t read(int32_t *buf, size_t count = 1) { return read((uint32_t*)buf, count); };

        virtual size_t write(const uint8_t *buf, size_t count = 1) = 0;
        virtual size_t write(const uint16_t *buf, size_t count = 1);
        virtual size_t write(const uint32_t *buf, size_t count = 1);
        virtual bool write(const void *buf, const char *layout);
        template<typename T, typename enable_if<is_class<T>::value, bool>::type = true>
        bool write(const T *buf) { return write((void *)buf, struct_layout<T>::value); };
        __forceinline size_t write(const int8_t *buf, size_t count = 1) { return write((uint8_t*)buf, count); };
        __forceinline size_t write(const int16_t *buf, size_t count = 1) { return write((uint16_t*)buf, count); };
        __forceinline size_t write(const int32_t *buf, size_t count = 1) { return write((uint32_t*)buf, count); };
        
        int width() const { return _width; }
        int width(int w) { int t = _width; _width = w; return t; }
        char fill() const { return _fill; }
        char fill(char d) { int t = _fill; _fill = d; return t; }

        stream_c &operator<<(manipulator_f m);
        stream_c &operator<<(const char *str);
        stream_c &operator<<(const char c);
        stream_c &operator<<(const unsigned char c);
        stream_c &operator<<(const int16_t i);
        stream_c &operator<<(const uint16_t i);
        stream_c &operator<<(const int32_t i);
        stream_c &operator<<(const uint32_t i);

    protected:
        bool assert_on_error() const __pure { return _assert_on_error; }
        bool _assert_on_error;
        int _width;
        char _fill;
    };
    
    extern stream_c& tbin;
    extern stream_c& tbout;
    extern stream_c& tberr;

    extern stream_c::manipulator_f endl;
    extern stream_c::manipulator_f ends;
    extern stream_c::manipulator_f flush;

    namespace detail {
        struct setw_s { int w; };
        static stream_c& operator<<(stream_c &s, const setw_s &m) { s.width(m.w); return s; }
        struct setfill_s { char c; };
        static stream_c& operator<<(stream_c &s, const setfill_s &m) { s.fill(m.c); return s; }
    }
    
    static const detail::setw_s setw(int w) { return (detail::setw_s){ w }; };
    static const detail::setfill_s setfill(char c) { return (detail::setfill_s){ c }; };

    class swap_stream_c : public stream_c {
    public:
        swap_stream_c(stream_c *stream);
        virtual ~swap_stream_c() {};

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
    
    
    class fstream_c : public stream_c {
    public:
        typedef enum __packed {
            input = 1 << 0,
            output = 1 << 1,
            append = 1 << 2
        } openmode_e;

        fstream_c(FILE *file);
        fstream_c(const char *path, openmode_e mode = input);
        virtual ~fstream_c();
        
        openmode_e mode() const __pure { return _mode; }
        bool is_open() const __pure { return _file != nullptr; }
        bool open();
        bool close();

        virtual bool good() const __pure;
        virtual ptrdiff_t tell() const __pure;
        virtual ptrdiff_t seek(ptrdiff_t pos, seekdir_e way);
        virtual bool flush();

        using stream_c::read;
        virtual size_t read(uint8_t *buf, size_t count = 1);
        using stream_c::write;
        virtual size_t write(const uint8_t *buf, size_t count = 1);
        
    private:
        const char *_path;
        openmode_e _mode;
        FILE *_file;
    };
    
    __forceinline __pure static fstream_c::openmode_e operator|(fstream_c::openmode_e a, fstream_c::openmode_e b) {
        return (fstream_c::openmode_e)((uint8_t)a | (uint8_t)b);
    }
    
    class strstream_c : public stream_c {
    public:
        strstream_c(size_t len);
        strstream_c(char *buf, size_t len);
        virtual ~strstream_c() {};
        
        void reset() { _pos = 0; }
        char* str() { return _buf; };
        
        virtual ptrdiff_t tell() const __pure;
        virtual ptrdiff_t seek(ptrdiff_t pos, seekdir_e way);

        using stream_c::read;
        virtual size_t read(uint8_t *buf, size_t count = 1);
        using stream_c::write;
        virtual size_t write(const uint8_t *buf, size_t count = 1);
        
    private:
        unique_ptr_c<char> _owned_buf;
        char *const _buf;
        const size_t _len;
        size_t _pos;
        size_t _max;
    };
    
    /*
    class bufstream_c : public stream_c {
    public:
        bufstream_c(size_t len, stream_c *stream);
        virtual ~bufstream_c() {};

        size_t length() const { return _len; }
        
        virtual void set_assert_on_error(bool assert);

        virtual bool good() const __pure;
        virtual ptrdiff_t tell() const __pure;
        virtual ptrdiff_t seek(ptrdiff_t pos, seekdir_e way);
        virtual bool flush();

        using stream_c::read;
        virtual bool read(uint8_t *buf, size_t count = 1);

        using stream_c::write;
        virtual bool write(const uint8_t *buf, size_t count = 1);
    protected:
        virtual void underflow();
        virtual void overflow(uint8_t *buf, int len);
    private:
        unique_ptr_c<stream_c> _stream;
        unique_ptr_c<uint8_t> _buffer;
        const size_t _len;
        size_t _pos;
        size_t _max;
    };
    */
}

#endif /* stream_hpp */
