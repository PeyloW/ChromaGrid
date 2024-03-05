//
//  level.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-03-04.
//

#include "level.hpp"
#include "resources.hpp"

class tile_c {
public:
    tilestate_t state;

public:
    static const uint8_t STEP_MAX = 7;
    
    void tick() {
        if (_transition.step > 0) {
            _transition.step--;
            _dirty = true;
        }
    }

    uint8_t get_drawstate(tilestate_t *current, tilestate_t *from) const {
        *current = state;
        *from = _transition.from_state;
        return _transition.step;
    }
    
    bool check_dirty() {
        const bool d = _dirty;
        _dirty = false;
        return d;
    }
    
    color_e orb_color() const {
        return state.orb;
    }
    
    bool is_orb_color(color_e c) const {
        return (state.orb & c) == c;
    }
    
    bool at_target() const {
        return _transition.step == 0 && state.target == state.current;
    }
    
    bool try_add_orb(color_e c) {
        if (_transition.step == 0 && state.orb == none && state.type >= glass) {
            state.orb = c;
            _dirty = true;
            return true;
        } else {
            return false;
        }
    }
    
    color_e try_remove_orb() {
        if (_transition.step == 0 && state.orb != none && state.type != magnetic) {
            const auto c = state.orb;
            state.orb = none;
            _dirty = true;
            return c;
        } else {
            return none;
        }
    }
    
    void solve_remove_orb() {
        assert(state.orb != none && state.orb != both);
        _transition.from_state = state;
        _transition.step = STEP_MAX;
        state.orb = none;
        if (state.type == glass) {
            state.type = broken;
        }
        _dirty = true;
    }
        
private:
    struct transition_t {
        tilestate_t from_state;
        uint8_t step;
    } _transition;
    bool _dirty;
};

// Game-loop is:
//  1. Optionally try_remove_orb_at()
//  2. Optionally try_add_orb_at()
//  3. If 2 is successfull resolve_at()
//  4. tick() and redraw tiles with callback.
class grid_c {
public:
    static const int GRID_MAX = 12;
    tile_c tiles[GRID_MAX][GRID_MAX];
private:

    bool is_orb_at(color_e c, int x, int y) {
        if (x >= 0 && x < GRID_MAX && y >= 0 && y < GRID_MAX) {
            return tiles[x][y].is_orb_color(c);
        }
        return false;
    }
    
    template<class V>
    void visit_adjecent_at(int x, int y, V visitor) {
        for (int ay = MAX(0, y - 1); ay < MIN(GRID_MAX, y + 1); ay++) {
            for (int ax = x - 1; ax < x + 1; ax++) {
                visitor(tiles[ax][ay], ax, ay);
            }
        }
    }
    
    bool is_orb_solved_at(int x, int y) {
        const color_e c = tiles[x][y].orb_color();
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
    
    bool try_add_orb_at(color_e c, int x, int y) {
        assert(c >= gold && c <= silver);
        assert(x >= 0 && x < GRID_MAX);
        assert(y >= 0 && y < GRID_MAX);
        auto &tile = tiles[x][y];
        return tile.try_add_orb(c);
    }
    
    color_e try_remove_orb_at(int x, int y) {
        assert(x >= 0 && x < GRID_MAX);
        assert(y >= 0 && y < GRID_MAX);
        auto &tile = tiles[x][y];
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
            auto &tile = tiles[update.x][update.y];
            tile.solve_remove_orb();
        }
    }
    
    template<typename CB>
    bool tick(CB callback) {
        bool completed = true;
        for (int y = 0; y < GRID_MAX; y++) {
            for (int x = 0; x < GRID_MAX; x++) {
                auto &tile = tiles[x][y];
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
        auto &tile = tiles[x][y];
        return tile.get_drawstate(current, from);
    }
    
};


level_t::level_t(recipe_t *recipe) :
    _time(recipe->time),
    _time_count(0),
    _grid((grid_c*)calloc(1, sizeof(grid_c)))
{
    assert(recipe->width <= grid_c::GRID_MAX);
    assert(recipe->height <= grid_c::GRID_MAX);
    _orbs[0] = recipe->orbs[0];
    _orbs[1] = recipe->orbs[1];
    
    int off_x = (grid_c::GRID_MAX - recipe->width) / 2;
    int off_y = (grid_c::GRID_MAX - recipe->height) / 2;

    for (int y = 0; y < recipe->height; y++) {
        for (int x = 0; x < recipe->width; x++) {
            auto &src_tile = recipe->tiles[x + y * recipe->width];
            auto &dst_tile = _grid->tiles[off_x + x][off_y + y];
            dst_tile.state = src_tile;
        }
    }
    
}


level_t::~level_t() {
    free(_grid);
}

#define LABEL_X_INSET 200
#define TIME_Y_INSET 70
#define TIME_X_TRAIL (320 - 8)
#define ORB_X_INSET 240
#define ORB_X_LEAD 16
#define ORB_X_SPACING 40
#define ORB_Y_INSET 90

void level_t::draw_all(cgimage_c &screen) const {
    auto &rsc = cgresources_c::shared();
    for (int y = 0; y < grid_c::GRID_MAX; y++) {
        for (int x = 0; x < grid_c::GRID_MAX; x++) {
            draw_tile(screen, x, y);
        }
    }
    draw_orb_counts(screen);
    screen.draw(rsc.font, "TIME:", (cgpoint_t){LABEL_X_INSET, TIME_Y_INSET}, cgimage_c::align_left);
    draw_time(screen);
    
    screen.draw(rsc.font, "ORBS:", (cgpoint_t){LABEL_X_INSET, ORB_Y_INSET}, cgimage_c::align_left);
    cgrect_t rect = (cgrect_t){{0, 0}, {16, 10}};
    for (int i = 0; i < 2; i++) {
        cgpoint_t at = (cgpoint_t){(int16_t)(ORB_X_INSET + i * ORB_X_SPACING), ORB_Y_INSET - 1};
        screen.draw(rsc.orbs, rect, at);
        rect.origin.x += 16;
    }
    draw_orb_counts(screen);
}

void level_t::draw_tile(cgimage_c &screen, int x, int y) const {
    auto &rsc = cgresources_c::shared();
    auto &tile = _grid->tiles[x][y];
    if (tile.state.type != tiletype_e::empty) {
        int16_t tx = tile.state.target * 16;
        tx += tile.state.current * 48;
        int16_t ty = (tile.state.type - 1) * 16;
        const cgrect_t rect = (cgrect_t){{tx, ty}, {16, 16}};
        const cgpoint_t at = (cgpoint_t){(int16_t)(x * 16), (int16_t)(y * 16)};
        screen.draw_aligned(rsc.tiles, rect, at);
        if (tile.state.orb != color_e::none) {
            int16_t tx = (tile.state.orb - 1) * 16;
            int16_t ty = 0;
            const cgrect_t rect = (cgrect_t){{tx, ty}, {16, 10}};
            const cgpoint_t at = (cgpoint_t){(int16_t)(x * 16), (int16_t)(y * 16 + 3)};
            screen.draw(rsc.orbs, rect, at);
        }
    }
}

void level_t::draw_time(cgimage_c &screen) const {
    auto &rsc = cgresources_c::shared();
    int min = _time / 60;
    int sec = _time % 60;
    char buf[5];
    buf[0] = '0' + min;
    buf[1] = ':';
    buf[2] = '0' + (sec / 10);
    buf[3] = '0' + (sec % 10);
    buf[4] = 0;
    
    const cgpoint_t at = (cgpoint_t){ TIME_X_TRAIL - 32, TIME_Y_INSET };
    const cgrect_t rect = (cgrect_t){at, (cgsize_t){32, 8}};

    screen.draw(rsc.background, rect, at);
    screen.draw(rsc.mono_font, buf, at, cgimage_c::align_left);
}

void level_t::draw_orb_counts(cgimage_c &screen) const {
    auto &rsc = cgresources_c::shared();

    for (int i = 0; i < 2; i++) {
        char buf[3];
        auto d1 = _orbs[i] / 10;
        buf[0] = d1 ? '9' : '0' + d1;
        buf[1] = '0' + _orbs[i] % 10;
        buf[2] = 0;
        cgpoint_t at = (cgpoint_t){(int16_t)(ORB_X_INSET + ORB_X_LEAD + i * ORB_X_SPACING), ORB_Y_INSET};
        screen.draw(rsc.mono_font, buf, at, cgimage_c::align_left);
    }
}

level_t::state_e level_t::update_tick(cgimage_c &screen, int delta_ticks) {
    _time_count += delta_ticks;
    if (_time_count >= 50) {
        _time--;
        _time_count -= 50;
        draw_time(screen);
    }
    return _time > 0 ? normal : failed;
}
