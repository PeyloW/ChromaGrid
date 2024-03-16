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


CGDEFINE_ID (CGLV); // ChromaGrid LeVel
CGDEFINE_ID (LVHD); // LeVel HeaDer
CGDEFINE_ID (TSTS); // Tile STateS
CGDEFINE_ID (CGLR); // ChromaGrid Level Results

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

struct __attribute__((aligned (2))) tilestate_t  {
    tiletype_e type;
    color_e target;
    color_e current;
    color_e orb;
    inline bool can_have_orb() const {
        return type >= glass;
    }
};
static_assert(sizeof(tilestate_t) == 4, "tilestate_t size overflow");

struct level_recipe_t {
    struct __packed_struct header_t {
        uint8_t width, height;
        uint8_t orbs[2];
        uint16_t time;
    } header;
    static_assert(sizeof(header) == 6, "level_result_t size mismatch");
    const char *text;
    tilestate_t tiles[];
    static const int MAX_SIZE = sizeof(struct header_t) + sizeof(char *) + sizeof(tilestate_t) * 12 * 12;
    bool empty() const;
    int get_size() const;
    bool save(cgiff_file_c &iff);
    bool load(cgiff_file_c &iff, cgiff_chunk_t &start_chunk);
};

struct __packed_struct level_result_t {
    static const uint32_t FAILED_SCORE = 0;
    static const uint32_t PER_ORB_SCORE = 100;
    static const uint32_t PER_SECOND_SCORE = 10;
    uint32_t score;
    uint32_t orbs_score;
    uint32_t time_score;
    uint8_t orbs[2];
    uint16_t time;
    uint16_t moves;
    void calculate_scores(bool succes);
    bool merge_from(const level_result_t &new_result);
    bool save(cgiff_file_c &iff);
    bool load(cgiff_file_c &iff, cgiff_chunk_t &start_chunk);
};
static_assert(sizeof(level_result_t) == 18, "level_result_t size mismatch");

void draw_tilestate(cgimage_c &screen, const tilestate_t &state, cgpoint_t at, bool selected = false);
void draw_orb(cgimage_c &screen, color_e color, cgpoint_t at);

class grid_c;

class level_t : cgnocopy_c {
public:
    typedef enum __packed {
        normal,
        failed,
        success
    } state_e;
    
    level_t(level_recipe_t *recipe);
    ~level_t();

    state_e update_tick(cgimage_c &screen, cgmouse_c &mouse, int delta_ticks);

    void draw_all(cgimage_c &screen) const;
    
    void get_results(level_result_t *results) const {
        *results = _results;
    }
private:
    void draw_tile(cgimage_c &screen, int x, int y) const;
    void draw_time(cgimage_c &screen) const;
    void draw_orb_counts(cgimage_c &screen) const;
    void draw_move_count(cgimage_c &screen) const;

    level_result_t _results;
    int _time_count;
    grid_c *_grid;
};

#endif /* grid_h */
