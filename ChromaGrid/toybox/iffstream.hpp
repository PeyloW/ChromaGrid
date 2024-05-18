//
//  iffstream.hpp
//  toybox
//
//  Created by Fredrik on 2024-04-15.
//

#ifndef iffstream_hpp
#define iffstream_hpp

#include "stream.hpp"

namespace toystd {

    typedef uint32_t iff_id_t;

#ifdef __M68000__
    __forceinline static constexpr iff_id_t iff_id_make(const char *const str) {
        return (uint32_t)str[0]<<24 | (uint32_t)str[1]<<16 | (uint32_t)str[2]<<8 | str[3];
    }
#else
    __forceinline static const iff_id_t iff_id_make(const char *const str) {
        assert(strlen(str) == 4);
        return (uint32_t)str[0]<<24 | (uint32_t)str[1]<<16 | (uint32_t)str[2]<<8 | str[3];
    }
#endif
    __forceinline static void iff_id_str(iff_id_t id, char buf[5]) {
        buf[0] = id >> 24; buf[1] = id >> 16; buf[2] = id >> 8; buf[3] = id; buf[4] = 0;
    }
    __forceinline static bool iff_id_match(const iff_id_t id, const char *const str) {
        if (strcmp(str, "*") != 0) {
            return iff_id_make(str) == id;
        }
        return true;
    }
    
    
#ifdef __M68000__
#   define DEFINE_IFF_ID(ID) \
static constexpr const char * IFF_ ## ID = #ID; \
static constexpr iff_id_t IFF_ ## ID ## _ID = iff_id_make(IFF_ ## ID)
#   define DEFINE_IFF_ID_EX(ID, STR) \
static constexpr char * IFF_ ## ID = STR; \
static constexpr iff_id_t IFF_ ## ID ## _ID = iff_id_make(IFF_ ## ID)
#else
#   define DEFINE_IFF_ID(ID) \
static const char *const IFF_ ## ID = #ID; \
static iff_id_t IFF_ ## ID ## _ID = iff_id_make(IFF_ ## ID)
#   define DEFINE_IFF_ID_EX(ID, STR) \
static const char *const IFF_ ## ID = STR; \
static const iff_id_t IFF_ ## ID ## _ID = iff_id_make(IFF_ ## ID)
#endif
    
    DEFINE_IFF_ID (FORM);
    //static constexpr const char * IFF_FORM = "FORM";
    //static constexpr iff_id_t IFF_FORM_ID = iff_id_make(IFF_FORM);
    
    DEFINE_IFF_ID (LIST);
    DEFINE_IFF_ID_EX (CAT, "CAT ");
    DEFINE_IFF_ID_EX (NULL, "    ");
    DEFINE_IFF_ID (TEXT);
    DEFINE_IFF_ID (NAME);
    
    struct iff_chunk_s {
        long offset;
        iff_id_t id;
        uint32_t size;
    };
    
    struct iff_group_s : public iff_chunk_s {
        iff_id_t subtype;
    };
    
    class iffstream_c : public stream_c {
    public:
        iffstream_c(stream_c *stream);
        iffstream_c(const char *path, fstream_c::openmode_e mode = fstream_c::input);
        ~iffstream_c() = default;
                
        virtual void set_assert_on_error(bool assert);

        virtual bool good() const __pure;
        virtual ptrdiff_t tell() const __pure;
        virtual ptrdiff_t seek(ptrdiff_t pos, seekdir_e way);

        bool first(const char *const id, iff_chunk_s &chunk);
        bool first(const char *const id, const char *const subtype, iff_group_s &group);
        bool next(const iff_group_s &in_group, const char *const id, iff_chunk_s &chunk);
        bool expand(const iff_chunk_s &chunk, iff_group_s &group);
        
        bool reset(const iff_chunk_s &chunk);
        bool skip(const iff_chunk_s &chunk);
        bool align();
        
        bool begin(iff_chunk_s &chunk, const char *const id);
        bool end(iff_chunk_s &chunk);
        
        using stream_c::read;
        virtual size_t read(uint8_t *buf, size_t count = 1);
        virtual size_t read(uint16_t *buf, size_t count = 1);
        virtual size_t read(uint32_t *buf, size_t count = 1);
        
        using stream_c::write;
        virtual size_t write(const uint8_t *buf, size_t count = 1);
        virtual size_t write(const uint16_t *buf, size_t count = 1);
        virtual size_t write(const uint32_t *buf, size_t count = 1);

    private:
        bool read(iff_group_s &group);
        bool read(iff_chunk_s &chunk);

        unique_ptr_c<stream_c> _stream;

#define IFF_MAX_NESTING_DEPTH 8
        struct chunk_start_s {
            iff_id_t id;
            long offset;
            bool is_group;
        };
    };
    
}


#endif /* iffstream_hpp */
