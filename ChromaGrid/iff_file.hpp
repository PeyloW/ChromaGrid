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

typedef uint32_t cgiff_id_t;

#ifdef __M68000__
#   define cghton(v)
#else
#include <type_traits>
template<class Type, typename std::enable_if<sizeof(Type) == 1 && std::is_arithmetic<Type>::value, bool>::type = true>
static inline void cghton(Type &value) { }

template<class Type, typename std::enable_if<sizeof(Type) == 2 && std::is_arithmetic<Type>::value, bool>::type = true>
static void inline cghton(Type &value) { value = htons(value); }

template<class Type, typename std::enable_if<sizeof(Type) == 4 && std::is_arithmetic<Type>::value, bool>::type = true>
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


__forceinline static cgiff_id_t cgiff_id_make(const char *const str) {
    assert(strlen(str) == 4);
    return (uint32_t)str[0]<<24 | (uint32_t)str[1]<<16 | (uint32_t)str[2]<<8 | str[3];
}
__forceinline static void cgiff_id_str(cgiff_id_t id, char buf[5]) {
    buf[0] = id >> 24; buf[1] = id >> 16; buf[2] = id >> 8; buf[3] = id; buf[4] = 0;
}
__forceinline static bool cgiff_id_match(const cgiff_id_t id, const char *const str) {
    if (strcmp(str, "*") != 0) {
        return cgiff_id_make(str) == id;
    }
    return true;
}

static const char *const CGIFF_FORM = "FORM";
static const char *const CGIFF_LIST = "LIST";
static const char *const CGIFF_CAT  = "CAT ";
static const char *const CGIFF_NULL = "    ";

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
    
    bool first(const char *const id, const char *const subtype, cgiff_group_t &group);
    bool next(const cgiff_group_t &in_group, const char *const id, cgiff_chunk_t &chunk);
    bool expand(const cgiff_chunk_t &chunk, cgiff_group_t &group);

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
            for (int i = 0; i < n; i++) {
                cghton(data[i]);
            }
            return true;
        }
        return false;
    }
    bool read(void *data, size_t s, size_t n);
    
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
    bool _owns_file;
};

#endif /* iff_file_hpp */
