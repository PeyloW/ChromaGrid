//
//  grid.hpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-01-30.
//

#ifndef grid_h
#define grid_h

#include "cincludes.hpp"
#include "types.hpp"
#include "graphics.hpp"
#include "system.hpp"
#include "iff_file.hpp"

typedef enum __packed {
    none, gold, silver, both
} color_e;

typedef enum __packed {
    empty,
    blocked,
    broken,
    glass,
    regular,
    magnetic
} tiletype_e;

struct __attribute__((aligned (4))) tilestate_t  {
    tiletype_e type;
    color_e target;
    color_e current;
    color_e orb;
    inline bool can_have_orb() const {
        return type >= glass;
    }
};
static_assert(sizeof(tilestate_t) == 4, "tilestate_t size overflow");

void draw_tilestate(cgimage_c &screen, const tilestate_t &state, cgpoint_t at, bool selected = false);
void draw_orb(cgimage_c &screen, color_e color, cgpoint_t at);

class grid_c;

class level_t : cgnocopy_c {
public:
    typedef struct recipe_t {
        struct header_t {
            uint8_t width, height;
            uint8_t orbs[2];
            uint16_t time;
        } header;
        const char *text;
        tilestate_t tiles[];
        static const int MAX_SIZE = sizeof(struct header_t) + sizeof(char *) + sizeof(tilestate_t) * 12 * 12;
        int get_size() const;
        bool save(cgiff_file_c &iff);
        bool load(cgiff_file_c &iff, cgiff_chunk_t &start_chunk);
    } recipe_t;
    
    typedef enum __packed {
        normal,
        failed,
        success
    } state_e;
    
    level_t(recipe_t *recipe);
    ~level_t();

    state_e update_tick(cgimage_c &screen, cgmouse_c &mouse, int delta_ticks);

    void draw_all(cgimage_c &screen) const;
    
    void get_remaining(uint8_t *orbs, uint8_t *time) const {
        *orbs = _orbs[0] + _orbs[1];
        *time = _time;
    }
private:
    void draw_tile(cgimage_c &screen, int x, int y) const;
    void draw_time(cgimage_c &screen) const;
    void draw_orb_counts(cgimage_c &screen) const;

    
    uint8_t _orbs[2];
    uint8_t _time;
    int _time_count;
    grid_c *_grid;
};

#endif /* grid_h */
