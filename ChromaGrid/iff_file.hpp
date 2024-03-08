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


__forceinline static cgiff_id_t cgiff_id_make(const char *str) {
    assert(strlen(str) == 4);
    return (uint32_t)str[0]<<24 | (uint32_t)str[1]<<16 | (uint32_t)str[2]<<8 | str[3];
}
__forceinline static bool cgiff_id_equals(const cgiff_id_t id, const char *str) {
    return cgiff_id_make(str) == id;
}

static const cgiff_id_t CGIFF_FORM = cgiff_id_make("FORM");
static const cgiff_id_t CGIFF_LIST = cgiff_id_make("LIST");
static const cgiff_id_t CGIFF_CAT = cgiff_id_make("CAT ");
static const cgiff_id_t CGIFF_NULL = cgiff_id_make("NULL");

typedef struct {
    cgiff_id_t id;
    uint32_t size;
} cgiff_chunk_t;

typedef struct {
    cgiff_chunk_t chunk;
    cgiff_id_t subtype;
} cgiff_group_t;

#ifndef __M68000__
static inline void cghton(cgiff_chunk_t &chunk) {
    cghton(chunk.id);
    cghton(chunk.size);
}
static inline void cghton(cgiff_group_t &head) {
    cghton(head.chunk);
    cghton(head.subtype);
}
#endif

class cgiff_file_c : private cgnocopy_c {
public:
    cgiff_file_c(FILE *file);
    cgiff_file_c(const char *path);
    ~cgiff_file_c();
    
    bool find(const cgiff_id_t id, const cgiff_id_t type, cgiff_group_t &group);
    bool find(const cgiff_id_t id, cgiff_chunk_t &chunk);
    bool expand(const cgiff_chunk_t &chunk, cgiff_group_t &group);

    bool skip(const cgiff_group_t &header);
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
#define CGIFF_MAX_NESTING_DEPTH 8
    struct chunk_start_t {
        cgiff_id_t id;
        long offset;
        bool is_group;
    };
    FILE *_file;
    bool _owns_file;
    cgvector_c<chunk_start_t, CGIFF_MAX_NESTING_DEPTH> _chunk_starts;
};

#endif /* iff_file_hpp */
