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
    
    typedef uint32_t cgiff_id_t;
    
#ifdef __M68000__
#   define cghton(v)
#else

    template<class Type, typename enable_if<sizeof(Type) == 1 && is_arithmetic<Type>::value, bool>::type = true>
    static inline void cghton(Type &value) { }
    
    template<class Type, typename enable_if<sizeof(Type) == 2 && is_arithmetic<Type>::value, bool>::type = true>
    static void inline cghton(Type &value) { value = htons(value); }
    
    template<class Type, typename enable_if<sizeof(Type) == 4 && is_arithmetic<Type>::value, bool>::type = true>
    static void inline cghton(Type &value) { value = htonl(value); }
    
    template<class Type, size_t Count>
    static void inline cghton(Type (&array)[Count]) {
        for (auto &value : array) {
            cghton(&value);
        }
    }
    
    static inline void cghton(cgsize_t &size) {
        cghton(size.width);
        cghton(size.height);
    }
    static inline void cghton(cgpoint_t &point) {
        cghton(point.x);
        cghton(point.y);
    }
    
#endif
    
    
#ifdef __M68000__
    __forceinline static constexpr cgiff_id_t cgiff_id_make(const char *const str) {
        return (uint32_t)str[0]<<24 | (uint32_t)str[1]<<16 | (uint32_t)str[2]<<8 | str[3];
    }
#else
    __forceinline static const cgiff_id_t cgiff_id_make(const char *const str) {
        assert(strlen(str) == 4);
        return (uint32_t)str[0]<<24 | (uint32_t)str[1]<<16 | (uint32_t)str[2]<<8 | str[3];
    }
#endif
    __forceinline static void cgiff_id_str(cgiff_id_t id, char buf[5]) {
        buf[0] = id >> 24; buf[1] = id >> 16; buf[2] = id >> 8; buf[3] = id; buf[4] = 0;
    }
    __forceinline static bool cgiff_id_match(const cgiff_id_t id, const char *const str) {
        if (strcmp(str, "*") != 0) {
            return cgiff_id_make(str) == id;
        }
        return true;
    }
    
    
#ifdef __M68000__
#   define CGDEFINE_ID(ID) \
static constexpr const char * CGIFF_ ## ID = #ID; \
static constexpr cgiff_id_t CGIFF_ ## ID ## _ID = cgiff_id_make(CGIFF_ ## ID)
#   define CGDEFINE_ID_EX(ID, STR) \
static constexpr char * CGIFF_ ## ID = STR; \
static constexpr cgiff_id_t CGIFF_ ## ID ## _ID = cgiff_id_make(CGIFF_ ## ID)
#else
#   define CGDEFINE_ID(ID) \
static const char *const CGIFF_ ## ID = #ID; \
static cgiff_id_t CGIFF_ ## ID ## _ID = cgiff_id_make(CGIFF_ ## ID)
#   define CGDEFINE_ID_EX(ID, STR) \
static const char *const CGIFF_ ## ID = STR; \
static const cgiff_id_t CGIFF_ ## ID ## _ID = cgiff_id_make(CGIFF_ ## ID)
#endif
    
    CGDEFINE_ID (FORM);
    //static constexpr const char * CGIFF_FORM = "FORM";
    //static constexpr cgiff_id_t CGIFF_FORM_ID = cgiff_id_make(CGIFF_FORM);
    
    CGDEFINE_ID (LIST);
    CGDEFINE_ID_EX (CAT, "CAT ");
    CGDEFINE_ID_EX (NULL, "    ");
    CGDEFINE_ID (TEXT);
    CGDEFINE_ID (NAME);
    
    struct cgiff_chunk_t {
        long offset;
        cgiff_id_t id;
        uint32_t size;
    };
    
    struct cgiff_group_t : public cgiff_chunk_t {
        cgiff_id_t subtype;
    };
    
    
    class cgiff_file_c : private cgnocopy_c {
    public:
        cgiff_file_c(FILE *file);
        cgiff_file_c(const char *path, const char *mode = "r");
        ~cgiff_file_c();
        
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
        
        bool first(const char *const id, cgiff_chunk_t &chunk);
        bool first(const char *const id, const char *const subtype, cgiff_group_t &group);
        bool next(const cgiff_group_t &in_group, const char *const id, cgiff_chunk_t &chunk);
        bool expand(const cgiff_chunk_t &chunk, cgiff_group_t &group);
        
        bool reset(const cgiff_chunk_t &chunk);
        bool skip(const cgiff_chunk_t &chunk);
        bool align();
        
        template<typename T>
        bool read(T &value) {
            if (read(&value, sizeof(T), 1)) {
                cghton(value);
                return true;
            }
            return false;
        }
        template<typename T, size_t C>
        bool read(T (&value)[C]) {
            if (read(&value, sizeof(T), C)) {
                cghton(value);
                return true;
            }
            return false;
        }
        template<typename T>
        bool read(T *data, size_t n) {
            if (read(data, sizeof(T), n)) {
                for (int i = 0; i < n; i++) cghton(data[i]);
                return true;
            }
            return false;
        }
        bool read(void *data, size_t s, size_t n);
        
        bool begin(cgiff_chunk_t &chunk, const char *const id);
        bool end(cgiff_chunk_t &chunk);
        
        template<typename T>
        bool write(T &value) {
            cghton(value);
            bool r = write((void *)&value, sizeof(T), 1);
            cghton(value);
            return r;
        }
        template<typename T, size_t C>
        bool write(T (&value)[C]) {
            cghton(value);
            bool r = write((void *)&value, sizeof(T), C);
            cghton(value);
            return r;
        }
        template<typename T>
        bool write(T *data, size_t n) {
            for (int i = 0; i < n; i++) cghton(data[i]);
            bool r = write(data, sizeof(T), n);
            for (int i = 0; i < n; i++) cghton(data[i]);
            return r;
        }
        bool write(void *data, size_t s, size_t n);
        
    private:
        bool read(cgiff_group_t &group);
        bool read(cgiff_chunk_t &chunk);
        
#define CGIFF_MAX_NESTING_DEPTH 8
        struct chunk_start_t {
            cgiff_id_t id;
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