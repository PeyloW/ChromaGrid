//
//  grid.hpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-01-30.
//

#ifndef grid_h
#define grid_h

#include "cincludes.hpp"

typedef enum color_t {
    none, gold, silver, both
} color_t;

typedef enum tiletype_t {
    empty,
    blocked,
    broken,
    glass,
    regular,
    magnetic
} tiletype_t;

union __packed tilestate_t {
    struct __packed {
        color_t target:2;
        color_t current:2;
        color_t orb:2;
        tiletype_t type:4;
        uint8_t _reserved:3; // Reserved for step in gfx lookup
    };
    uint16_t index;
};
static_assert(sizeof(union tilestate_t) <= sizeof(uint16_t), "tilestate_t size overflow");

class __packed tile_c {

    tilestate_t _state;
    struct __packed transition_t {
        tilestate_t from_state;
        uint8_t step;
    } _transition;
    bool _dirty:1;

public:
    static const uint8_t STEP_MAX = 7;
    
    void tick() {
        if (_transition.step > 0) {
            _transition.step--;
            _dirty = true;
        }
    }

    uint8_t get_drawstate(tilestate_t *current, tilestate_t *from) const {
        *current = _state;
        *from = _transition.from_state;
        return _transition.step;
    }
    
    bool check_dirty() {
        const bool d = _dirty;
        _dirty = false;
        return d;
    }
    
    color_t orb_color() const {
        return _state.orb;
    }
    
    bool is_orb_color(color_t c) const {
        return (_state.orb & c) == c;
    }
    
    bool at_target() const {
        return _transition.step == 0 && _state.target == _state.current;
    }
    
    bool try_add_orb(color_t c) {
        if (_transition.step == 0 && _state.orb == none && _state.type >= glass) {
            _state.orb = c;
            _dirty = true;
            return true;
        } else {
            return false;
        }
    }
    
    color_t try_remove_orb() {
        if (_transition.step == 0 && _state.orb != none && _state.type != magnetic) {
            const auto c = _state.orb;
            _state.orb = none;
            _dirty = true;
            return c;
        } else {
            return none;
        }
    }
    
    void solve_remove_orb() {
        assert(_state.orb != none && _state.orb != both);
        _transition.from_state = _state;
        _transition.step = STEP_MAX;
        _state.orb = none;
        if (_state.type == glass) {
            _state.type = broken;
        }
        _dirty = true;
    }
        
};


// Game-loop is:
//  1. Optionally try_remove_orb_at()
//  2. Optionally try_add_orb_at()
//  3. If 2 is successfull resolve_at()
//  4. tick() and redraw tiles with callback.
class __packed grid_c {
    static const int GRID_MAX = 12;
    tile_c _tiles[GRID_MAX][GRID_MAX];
        
    bool is_orb_at(color_t c, int x, int y) {
        if (x >= 0 && x < GRID_MAX && y >= 0 && y < GRID_MAX) {
            return _tiles[x][y].is_orb_color(c);
        }
        return false;
    }
    
    template<class V>
    void visit_adjecent_at(int x, int y, V visitor) {
        for (int ay = MAX(0, y - 1); ay < MIN(GRID_MAX, y + 1); ay++) {
            for (int ax = x - 1; ax < x + 1; ax++) {
                visitor(_tiles[ax][ay], ax, ay);
            }
        }
    }
    
    bool is_orb_solved_at(int x, int y) {
        const color_t c = _tiles[x][y].orb_color();
        assert(c != none);
        int cnt = 0;
        visit_adjecent_at(x, y, [&cnt, c] (const tile_c &tile, int x, int y) {
            if (tile.is_orb_color(c)) {
                cnt++;
            }
        });
        return cnt >= 4;
    }
    
public:
    
    bool try_add_orb_at(color_t c, int x, int y) {
        assert(c >= gold && c <= silver);
        assert(x >= 0 && x < GRID_MAX);
        assert(y >= 0 && y < GRID_MAX);
        auto &tile = _tiles[x][y];
        return tile.try_add_orb(c);
    }
    
    color_t try_remove_orb_at(int x, int y) {
        assert(x >= 0 && x < GRID_MAX);
        assert(y >= 0 && y < GRID_MAX);
        auto &tile = _tiles[x][y];
        return tile.try_remove_orb();
    }

    void resolve_at(int x, int y) {
        assert(x >= 0 && x < GRID_MAX);
        assert(y >= 0 && y < GRID_MAX);
        struct { int x; int y; } updates[9];
        int update_count = 0;
        visit_adjecent_at(x, y, [this, &updates, &update_count] (const tile_c &tile, int x, int y) {
            if (is_orb_solved_at(x, x)) {
                updates[update_count++] = { x, y };
            }
        });
        for (int i = 0; i < update_count; i++) {
            auto &update = updates[i];
            auto &tile = _tiles[update.x][update.y];
            tile.solve_remove_orb();
        }
    }
    
    template<typename CB>
    bool tick(CB callback) {
        bool completed = true;
        for (int y = 0; y < GRID_MAX; y++) {
            for (int x = 0; x < GRID_MAX; x++) {
                auto &tile = _tiles[x][y];
                tile.tick();
                if (tile.check_dirty()) {
                    callback(x, y);
                }
                completed &= tile.at_target();
            }
        }
        return completed;
    }
        
    uint8_t get_drawstate_at(int x, int y, tilestate_t *current, tilestate_t *from) const {
        assert(x >= 0 && x < GRID_MAX);
        assert(y >= 0 && y < GRID_MAX);
        auto &tile = _tiles[x][y];
        return tile.get_drawstate(current, from);
    }
    
};

#endif /* grid_h */
