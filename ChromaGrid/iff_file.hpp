//
//  iff_file.hpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-04.
//

#ifndef iff_file_hpp
#define iff_file_hpp

#include "cincludes.hpp"

typedef uint32_t cgiff_id_t;

#ifdef __M68000__
static uint16_t cg_htons(uint16_t v) { return v; }
static uint32_t cg_htons(uint32_t v) { return v; }
#else
uint16_t cg_htons(uint16_t v);
uint32_t cg_htons(uint32_t v);
#endif

__forceinline static cgiff_id_t cgiff_id_make(const char *str) {
    assert(strlen(str) == 4);
#ifdef __M68000__
    return (uint32_t)str[0]<<24 | (uint32_t)str[1]<<16 | (uint32_t)str[2]<<8 | str[3];
#else
    return (uint32_t)str[3]<<24 | (uint32_t)str[2]<<16 | (uint32_t)str[1]<<8 | str[0];
#endif
}
__forceinline static bool cgiff_id_equals(const cgiff_id_t id, const char *str) {
    return cgiff_id_make(str) == id;
}

typedef struct {
    cgiff_id_t id;
    uint32_t size;
} cgiff_chunk_t;

typedef struct {
    cgiff_chunk_t chunk;
    cgiff_id_t subtype;
} cgiff_header_t;

class cgiff_file_c {
public:
    cgiff_file_c(FILE *file);
    cgiff_file_c(const char *path);
    ~cgiff_file_c();
  
    bool read(cgiff_header_t *header);
  
    bool find(const cgiff_id_t id, cgiff_chunk_t *chunk);
    bool read(cgiff_chunk_t *chunk);
    
    template<typename T>
    bool read(T *data, size_t n) {
        return read(data, sizeof(T), n);
    }
    bool read(void *data, size_t s, size_t n);
    
private:
    FILE *_file;
    bool _owns_file;
};

#endif /* iff_file_hpp */
