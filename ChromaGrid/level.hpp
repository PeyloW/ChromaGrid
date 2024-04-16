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
#include "canvas.hpp"
#include "system.hpp"
#include "iffstream.hpp"

using namespace toybox;

DEFINE_IFF_ID (CGLV); // ChromaGrid LeVel
DEFINE_IFF_ID (LVHD); // LeVel HeaDer
DEFINE_IFF_ID (TSTS); // Tile STateS
DEFINE_IFF_ID (CGLR); // ChromaGrid Level Results

#define DEBUG_CPU_LEVEL_DRAW_TIME 0x007
#define DBEUG_CPU_LEVEL_TICK 0x030
#define DBEUG_CPU_LEVEL_RESOLVE 0x700
#define DEBUG_CPU_LEVEL_GRID_TICK 0x200
#define DEBUG_CPU_LEVEL_GRID_DRAW 0x400

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

struct __packed_struct tilestate_t  {
    tiletype_e type;
    color_e target;
    color_e current;
    color_e orb;
    inline bool can_have_orb() const {
        return type >= glass;
    }
};
static_assert(sizeof(tilestate_t) == 4, "tilestate_t size overflow");
namespace toystd {
    template<>
    struct struct_layout<tilestate_t> {
        static constexpr char *value = "4b";
    };
}

struct level_recipe_t {
    struct __packed_struct header_t {
        uint8_t width, height;
        uint8_t orbs[2];
        uint16_t time;
    } header;
    const char *text;
    tilestate_t tiles[];
    static const int MAX_SIZE = 16 + sizeof(tilestate_t) * 12 * 12;
    bool empty() const;
    int get_size() const;
    bool save(iffstream_c &iff);
    bool load(iffstream_c &iff, iff_chunk_s &start_chunk);
};
static_assert(sizeof(level_recipe_t::header) == 6, "level_recipe_t::header size mismatch");
#ifndef __M68000__
static_assert(__offsetof(level_recipe_t, tiles) == 16, "offset of level_recipe_t::tiles mismatch");
#endif
namespace toystd {
    template<>
    struct struct_layout<level_recipe_t::header_t> {
        static constexpr char *value = "4b1w";
    };
}
    
struct __packed_struct level_result_t {
    static const uint16_t FAILED_SCORE = 0;
    static const uint16_t PER_ORB_SCORE = 100;
    static const uint16_t PER_SECOND_SCORE = 10;
    uint16_t score;
    uint8_t orbs[2];
    uint16_t time;
    uint16_t moves;
    void calculate_score(bool succes);
    void get_subscores(uint16_t &orbs_score, uint16_t &time_score) const;
    bool merge_from(const level_result_t &new_result);
    bool save(iffstream_c &iff);
    bool load(iffstream_c &iff, iff_chunk_s &start_chunk);
};
static_assert(sizeof(level_result_t) == 8, "level_result_t size mismatch");
namespace toystd {
    template<>
    struct struct_layout<level_result_t> {
        static constexpr char *value = "1w2b2w";
    };
}

void draw_tilestate(canvas_c &screen, const tilestate_t &state, point_s at, bool selected = false);
void draw_orb(canvas_c &screen, color_e color, point_s at);

class grid_c;

class level_t : public nocopy_c {
public:
    typedef enum __packed {
        normal,
        failed,
        success
    } state_e;
    
    level_t(level_recipe_t *recipe);
    ~level_t();

    state_e update_tick(canvas_c &screen, mouse_c &mouse, int passed_seconds);

    void draw_all(canvas_c &screen) const;
    
    const tilestate_t &tilestate_at(int x, int y) const;
    
    void get_results(level_result_t *results) const {
        *results = _results;
    }
private:
    void draw_tile(canvas_c &screen, int x, int y) const;
    void draw_time(canvas_c &screen) const;
    void draw_orb_counts(canvas_c &screen) const;
    void draw_move_count(canvas_c &screen) const;

    level_result_t _results;
    grid_c *_grid;
};

#endif /* grid_h */
