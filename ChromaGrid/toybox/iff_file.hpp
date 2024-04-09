//
//  iff_file.hpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-04.
//

#ifndef iff_file_hpp
#define iff_file_hpp

#include "cincludes.hpp"
#include "types.hpp"
#include "type_traits.hpp"

namespace toybox {
    
    using namespace toystd;
    
    typedef uint32_t iff_id_t;
    
#ifdef __M68000__
#   define hton(v)
#else

    template<class Type, typename enable_if<sizeof(Type) == 1 && is_arithmetic<Type>::value, bool>::type = true>
    static inline void hton(Type &value) { }
    
    template<class Type, typename enable_if<sizeof(Type) == 2 && is_arithmetic<Type>::value, bool>::type = true>
    static void inline hton(Type &value) { value = htons(value); }
    
    template<class Type, typename enable_if<sizeof(Type) == 4 && is_arithmetic<Type>::value, bool>::type = true>
    static void inline hton(Type &value) { value = htonl(value); }
    
    template<class Type, size_t Count>
    static void inline hton(Type (&array)[Count]) {
        for (auto &value : array) {
            hton(&value);
        }
    }
    
    static inline void hton(size_s &size) {
        hton(size.width);
        hton(size.height);
    }
    static inline void hton(point_s &point) {
        hton(point.x);
        hton(point.y);
    }
    
#endif
    
    
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
    
    
    class iff_file_c : public nocopy_c {
    public:
        iff_file_c(FILE *file, bool write = false);
        iff_file_c(const char *path, const char *mode = "r");
        ~iff_file_c();
        
        template<class Commands>
        __forceinline void with_hard_asserts(bool asserts, Commands commands) {
            auto old_state = _hard_assert;
            _hard_assert = asserts;
            commands();
            _hard_assert = old_state;
        };
        
        long get_pos() const;
#ifndef __M68000__
        bool set_pos(long pos) const;
#endif
        
        bool first(const char *const id, iff_chunk_s &chunk);
        bool first(const char *const id, const char *const subtype, iff_group_s &group);
        bool next(const iff_group_s &in_group, const char *const id, iff_chunk_s &chunk);
        bool expand(const iff_chunk_s &chunk, iff_group_s &group);
        
        bool reset(const iff_chunk_s &chunk);
        bool skip(const iff_chunk_s &chunk);
        bool align();
        
        template<typename T>
        bool read(T &value) {
            if (read(&value, sizeof(T), 1)) {
                hton(value);
                return true;
            }
            return false;
        }
        template<typename T, size_t C>
        bool read(T (&value)[C]) {
            if (read(&value, sizeof(T), C)) {
                hton(value);
                return true;
            }
            return false;
        }
        template<typename T>
        bool read(T *data, size_t n) {
            if (read(data, sizeof(T), n)) {
                for (int i = 0; i < n; i++) hton(data[i]);
                return true;
            }
            return false;
        }
        bool read(void *data, size_t s, size_t n);
        
        bool begin(iff_chunk_s &chunk, const char *const id);
        bool end(iff_chunk_s &chunk);
        
        template<typename T>
        bool write(T &value) {
            hton(value);
            bool r = write((void *)&value, sizeof(T), 1);
            hton(value);
            return r;
        }
        template<typename T, size_t C>
        bool write(T (&value)[C]) {
            hton(value);
            bool r = write((void *)&value, sizeof(T), C);
            hton(value);
            return r;
        }
        template<typename T>
        bool write(T *data, size_t n) {
            for (int i = 0; i < n; i++) hton(data[i]);
            bool r = write(data, sizeof(T), n);
            for (int i = 0; i < n; i++) hton(data[i]);
            return r;
        }
        bool write(void *data, size_t s, size_t n);
        
    private:
        bool read(iff_group_s &group);
        bool read(iff_chunk_s &chunk);
        
#define IFF_MAX_NESTING_DEPTH 8
        struct chunk_start_s {
            iff_id_t id;
            long offset;
            bool is_group;
        };
        FILE *_file;
        bool _hard_assert;
        bool _for_writing;
        bool _owns_file;
    };
    
}

#endif /* iff_file_hpp */
