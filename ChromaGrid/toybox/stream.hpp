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

namespace toystd {
    
    class stream_c : public nocopy_c {
    public:
        typedef enum __packed {
            beg = SEEK_SET,
            cur = SEEK_CUR,
            end = SEEK_END
        } seekdir_e;
        
        stream_c();
        virtual ~stream_c() {}
            
        virtual void set_assert_on_error(bool assert);

        virtual bool good() const __pure;
        virtual ptrdiff_t tell() const __pure = 0;
        virtual ptrdiff_t seek(ptrdiff_t pos, seekdir_e way) = 0;

        virtual bool read(uint8_t *buf, int count = 1) = 0;
        virtual bool read(uint16_t *buf, int count = 1);
        virtual bool read(uint32_t *buf, int count = 1);
        virtual bool read(void *buf, const char *layout);
        template<typename T, typename enable_if<is_class<T>::value, bool>::type = true>
        bool read(T *buf) { return read((void *)buf, struct_layout<T>::value); };
        __forceinline bool read(int8_t *buf, int count = 1) { return read((uint8_t*)buf, count); };
        __forceinline bool read(int16_t *buf, int count = 1) { return read((uint16_t*)buf, count); };
        __forceinline bool read(int32_t *buf, int count = 1) { return read((uint32_t*)buf, count); };

        virtual bool write(const uint8_t *buf, int count = 1) = 0;
        virtual bool write(const uint16_t *buf, int count = 1);
        virtual bool write(const uint32_t *buf, int count = 1);
        virtual bool write(const void *buf, const char *layout);
        template<typename T, typename enable_if<is_class<T>::value, bool>::type = true>
        bool write(const T *buf) { return write((void *)buf, struct_layout<T>::value); };
        __forceinline bool write(const int8_t *buf, int count = 1) { return write((uint8_t*)buf, count); };
        __forceinline bool write(const int16_t *buf, int count = 1) { return write((uint16_t*)buf, count); };
        __forceinline bool write(const int32_t *buf, int count = 1) { return write((uint32_t*)buf, count); };
        
    protected:
        bool assert_on_error() const __pure { return _assert_on_error; }
        bool _assert_on_error;
    };
    
    class hton_stream_c : public stream_c {
    public:
        hton_stream_c(stream_c *stream);
        hton_stream_c(stream_c &stream);
        virtual ~hton_stream_c();

        virtual void set_assert_on_error(bool assert);

        virtual bool good() const __pure;
        virtual ptrdiff_t tell() const __pure;
        virtual ptrdiff_t seek(ptrdiff_t pos, seekdir_e way);

        using stream_c::read;
        virtual bool read(uint8_t *buf, int count = 1);
        virtual bool read(uint16_t *buf, int count = 1);
        virtual bool read(uint32_t *buf, int count = 1);

        using stream_c::write;
        virtual bool write(const uint8_t *buf, int count = 1);
        virtual bool write(const uint16_t *buf, int count = 1);
        virtual bool write(const uint32_t *buf, int count = 1);
    private:
        stream_c *_owned_stream;
        stream_c &_stream;
    };
    
    
    class fstream_c : public stream_c {
    public:
        typedef enum __packed {
            input = 1 << 0,
            output = 1 << 1,
            append = 1 << 2
        } openmode_e;

        fstream_c(const char *path, openmode_e mode = input);
        virtual ~fstream_c();
        
        openmode_e mode() const __pure { return _mode; }
        bool is_open() const __pure { return _file != nullptr; }
        bool open();
        bool close();

        virtual bool good() const __pure;
        virtual ptrdiff_t tell() const __pure;
        virtual ptrdiff_t seek(ptrdiff_t pos, seekdir_e way);

        using stream_c::read;
        virtual bool read(uint8_t *buf, int count = 1);
        using stream_c::write;
        virtual bool write(const uint8_t *buf, int count = 1);
        
    private:
        const char *_path;
        openmode_e _mode;
        FILE *_file;
    };
    
    __forceinline __pure static fstream_c::openmode_e operator|(fstream_c::openmode_e a, fstream_c::openmode_e b) {
        return (fstream_c::openmode_e)((uint8_t)a | (uint8_t)b);
    }
    
}

#endif /* stream_hpp */
