//
//  iff_file.hpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-04.
//

#ifndef iff_file_hpp
#define iff_file_hpp

#include "cincludes.hpp"

typedef union {
    uint32_t value;
    uint8_t bytes[4];
} cgiff_id_t;

#ifdef __M68000__
static uint16_t cg_htons(uint16_t v) { return v; }
static uint32_t cg_htons(uint32_t v) { return v; }
#else
uint16_t cg_htons(uint16_t v);
uint32_t cg_htons(uint32_t v);
#endif

static inline bool cgiff_id_equals(const cgiff_id_t &id, const char *str) {
    assert(strlen(str) == 4);
    return memcmp(&id, str, 4) == 0;
}

typedef struct {
    cgiff_id_t id;
    uint32_t size;
} cgiff_chunk_t;

typedef struct {
    cgiff_chunk_t chunk;
    cgiff_id_t subtype;
} cgiff_header_t;

class cgiff_file {
public:
    cgiff_file(FILE *file);
    cgiff_file(const char *path);
    ~cgiff_file();
  
    bool read(cgiff_header_t *header);
  
    bool find(const char *id, cgiff_chunk_t *chunk);
    bool find(const cgiff_id_t id, cgiff_chunk_t *chunk);
    bool read(cgiff_chunk_t *chunk);
    
    template<typename T>
    bool read(T *data, size_t n) {
        return read(data, sizeof(T), n);
    }
    bool read(void *data, size_t s, size_t n);
    
private:
    FILE *file;
    bool owns_file;
};

#endif /* iff_file_hpp */