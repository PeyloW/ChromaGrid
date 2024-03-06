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
};
static_assert(sizeof(tilestate_t) == 4, "tilestate_t size overflow");


class grid_c;

class level_t : cgnocopy_c {
public:
    typedef struct {
        uint8_t width, height;
        uint8_t orbs[2];
        uint8_t time;
        tilestate_t tiles[];
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
