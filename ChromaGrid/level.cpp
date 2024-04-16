//
//  level.cpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-03-04.
//

#include "level.hpp"
#include "resources.hpp"

typedef enum __packed {
    no_changes = 0,
    added_tile = 1 << 0,
    removed_orb = 1 << 1,
    added_orb = 1 << 2,
    fused_orb = 1 << 3,
    broke_glass = 1 << 4
} tile_changes_e;
__forceinline tile_changes_e &operator|=(tile_changes_e &a, const tile_changes_e &b) {
    a = static_cast<tile_changes_e>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
    return a;
}

static tile_changes_e cgp_tile_changes = no_changes;

void level_result_t::calculate_score(bool succes) {
    if (succes) {
        uint16_t orbs_score, time_score;
        get_subscores(orbs_score, time_score);
        score = orbs_score + time_score;
    } else {
        score = 0;
    }
}

void level_result_t::get_subscores(uint16_t &orbs_score, uint16_t &time_score) const {
    assert(((long)orbs[0] + (long)orbs[1]) * (long)PER_ORB_SCORE <= INT16_MAX);
    assert((long)time * (long)PER_SECOND_SCORE <= INT16_MAX);
    orbs_score = (orbs[0] + orbs[1]) * PER_ORB_SCORE;
    time_score = time * PER_SECOND_SCORE;
}

bool level_result_t::merge_from(const level_result_t &new_result) {
    bool improved = false;
    if (new_result.score != 0) {
        if (time == 0 || new_result.time < time) {
            time = new_result.time;
            improved = true;
        }
        if (new_result.score > score) {
            score = new_result.score;
            improved = true;
        }
        if (moves == 0 || new_result.moves < moves) {
            moves = new_result.moves;
            improved = true;
        }
        if (improved) {
            orbs[0] = new_result.orbs[0];
            orbs[1] = new_result.orbs[1];
        }
    }
    return improved;
}


class tile_c {
public:
    static const uint8_t STEP_MAX = 16;
    
    __forceinline void tick() {
        if (transition.step > 0) {
            transition.step--;
            _dirty = true;
        }
    }
    
    __forceinline bool check_dirty() {
        const bool d = _dirty;
        _dirty = false;
        return d;
    }
    
    __forceinline color_e orb_color() const {
        return state.orb;
    }
    
    __forceinline bool is_orb_color(color_e c) const {
        return (state.orb & c) == c;
    }
    
    __forceinline bool is_remaining() const {
        if (state.target != none) {
            return state.target != state.current;
        } else {
            return false;
        }
    }
    
    __forceinline bool at_target() const {
        return transition.step == 0 && state.target == state.current;
    }
    
    bool try_add_orb(color_e c) {
        if (transition.step == 0 && state.orb == none && state.can_have_orb()) {
            state.orb = c;
            cgp_tile_changes |= added_orb;
            _dirty = true;
            return true;
        } else {
            return false;
        }
    }
    
    color_e try_remove_orb() {
        if (transition.step == 0 && state.orb != none && state.type != magnetic) {
            const auto c = state.orb;
            state.orb = none;
            cgp_tile_changes |= removed_orb;
            if (state.type == glass) {
                state.type = broken;
                cgp_tile_changes |= broke_glass;
            }
            _dirty = true;
            return c;
        } else {
            return none;
        }
    }
    
    void try_make_tile(tiletype_e type) {
        if (state.type == empty) {
            transition.from_state = state;
            transition.step = STEP_MAX;
            state.type = type;
            cgp_tile_changes |= added_tile;
        }
    }
    
    void solve_remove_orb() {
        assert(state.orb != none && state.orb != both);
        transition.from_state = state;
        transition.step = STEP_MAX;
        state.orb = none;
        cgp_tile_changes |= fused_orb;
        if (state.type == glass) {
            state.type = broken;
            cgp_tile_changes |= broke_glass;
        }
        if (state.current != state.target) {
            state.current = none;
        }
        _dirty = true;
    }
        
    tilestate_t state;
    struct transition_t {
        tilestate_t from_state;
        uint8_t step;
    } transition;
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

    inline bool is_orb_at(color_e c, int x, int y) const {
        if (x >= 0 && x < GRID_MAX && y >= 0 && y < GRID_MAX) {
            return tiles[x][y].is_orb_color(c);
        }
        return false;
    }
    
    template<class V>
    inline void visit_adjecent_at(int x, int y, V visitor) const {
        for (int ay = MAX(0, y - 1); ay < MIN(GRID_MAX, y + 2); ay++) {
            for (int ax = MAX(0, x - 1); ax < MIN(GRID_MAX, x + 2); ax++) {
                visitor(tiles[ax][ay], ax, ay);
            }
        }
    }
    template<class V>
    inline void visit_adjecent_at(int x, int y, V visitor) {
        for (int ay = MAX(0, y - 1); ay < MIN(GRID_MAX, y + 2); ay++) {
            for (int ax = MAX(0, x - 1); ax < MIN(GRID_MAX, x + 2); ax++) {
                visitor(tiles[ax][ay], ax, ay);
            }
        }
    }

    template<class V>
    inline void visit_across_at(int x, int y, V visitor) {
        for (int ay = MAX(0, y - 1); ay < MIN(GRID_MAX, y + 2); ay++) {
            visitor(tiles[x][ay], x, ay);
        }
        for (int ax = MAX(0, x - 1); ax < MIN(GRID_MAX, x + 2); ax++) {
            visitor(tiles[ax][y], ax, y);
        }
    }
    
    inline bool is_orb_solved_at(int x, int y) const {
        const color_e c = tiles[x][y].orb_color();
        int cnt = 0;
        if (c != none) {
            visit_adjecent_at(x, y, [&cnt, c] (const tile_c &tile, int x, int y) {
                if (tile.is_orb_color(c)) {
                    cnt++;
                }
            });
        }
        return cnt >= 4;
    }
    
public:
    
    bool try_add_orb_at(color_e c, int x, int y) {
        assert(c >= gold && c <= silver);
        assert(x >= 0 && x < GRID_MAX);
        assert(y >= 0 && y < GRID_MAX);
        auto &src_tile = tiles[x][y];
        if (src_tile.try_add_orb(c)) {
            visit_across_at(x, y, [this, &src_tile] (tile_c &tile, int x, int y) {
                tile.try_make_tile(src_tile.state.type);
            });
            return true;
        } else {
            return false;
        }
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
        vector_c<point_s, 9> updates;
        visit_adjecent_at(x, y, [this, &updates] (tile_c &tile, int x, int y) {
            if (is_orb_solved_at(x, y)) {
                tile.state.current = tile.state.orb;
                updates.push_back((point_s){(int16_t)x, (int16_t)y});
            }
        });
        for (auto update = updates.begin(); update != updates.end(); update++) {
            auto &tile = tiles[update->x][update->y];
            tile.solve_remove_orb();
        }
    }
    
    template<typename CB>
    bool tick(uint16_t &remaining, CB callback) {
        bool completed = true;
        remaining = 0;
        for (int y = GRID_MAX; --y != -1; ) {
            for (int x = GRID_MAX; --x != -1; ) {
                auto &tile = tiles[x][y];
                tile.tick();
                if (tile.check_dirty()) {
                    callback(x, y);
                }
                completed &= tile.at_target();
                if (tile.is_remaining()) {
                    remaining++;
                }
            }
        }
        return completed;
    }
};


level_t::level_t(level_recipe_t *recipe) :
    _grid((grid_c*)calloc(1, sizeof(grid_c)))
{
    assert(recipe->header.width <= grid_c::GRID_MAX);
    assert(recipe->header.height <= grid_c::GRID_MAX);

    _results.score = 0;
    _results.orbs[0] = recipe->header.orbs[0];
    _results.orbs[1] = recipe->header.orbs[1];
    _results.time = recipe->header.time;
    _results.moves = 0;
    _remaining = 0;
    
    int off_x = (grid_c::GRID_MAX - recipe->header.width) / 2;
    int off_y = (grid_c::GRID_MAX - recipe->header.height) / 2;

    for (int y = 0; y < recipe->header.height; y++) {
        for (int x = 0; x < recipe->header.width; x++) {
            auto &src_tile = recipe->tiles[x + y * recipe->header.width];
            auto &dst_tile = _grid->tiles[off_x + x][off_y + y];
            dst_tile.state = src_tile;
            if (dst_tile.state.target != none && dst_tile.state.target != dst_tile.state.current) {
                _remaining++;
            }
        }
    }
    
}


level_t::~level_t() {
    free(_grid);
}

#define LABEL_X_INSET 200
#define TIME_Y_INSET 65
#define TIME_X_TRAIL (320 - 8)
#define ORB_X_INSET 248
#define ORB_X_LEAD 16
#define ORB_X_SPACING 32
#define ORB_Y_INSET 85
#define MOVES_Y_INSET 105
#define REMAINING_Y_INSET 125

void level_t::draw_all(canvas_c &screen) const {
    auto &rsc = cgresources_c::shared();
    for (int y = 0; y < grid_c::GRID_MAX; y++) {
        for (int x = 0; x < grid_c::GRID_MAX; x++) {
            draw_tile(screen, x, y);
        }
    }
    screen.draw(rsc.font, "TIME:", (point_s){LABEL_X_INSET, TIME_Y_INSET}, canvas_c::align_left);
    draw_time(screen);
    
    screen.draw(rsc.font, "ORBS:", (point_s){LABEL_X_INSET, ORB_Y_INSET}, canvas_c::align_left);
    rect_s rect = (rect_s){{0, 0}, {16, 10}};
    for (int i = 0; i < 2; i++) {
        point_s at = (point_s){(int16_t)(ORB_X_INSET + i * ORB_X_SPACING), ORB_Y_INSET - 1};
        draw_orb(screen, (color_e)(i + 1), at);
        rect.origin.x += 16;
    }
    draw_orb_counts(screen);
    
    screen.draw(rsc.font, "MOVES:", (point_s){LABEL_X_INSET, MOVES_Y_INSET}, canvas_c::align_left);
    draw_move_count(screen);

    screen.draw(rsc.font, "REMAINING:", (point_s){LABEL_X_INSET, REMAINING_Y_INSET}, canvas_c::align_left);
    draw_remaining_count(screen);
}

const tilestate_t &level_t::tilestate_at(int x, int y) const {
    return _grid->tiles[x][y].state;
}

inline static const rect_s tilestate_src_rect(const tilestate_t &state) {
    int16_t tx = state.target * 16;
    tx += state.current * 48;
    int16_t ty = (state.type - 1) * 16;
    return (rect_s){{tx, ty}, {16, 16}};
}

inline static void draw_tilestate(canvas_c &screen, const cgresources_c &rsc, const tilestate_t &state, int x, int y) {
    const point_s at = (point_s){(int16_t)(x * 16), (int16_t)(y * 16)};
    if (state.type == empty) {
        if (state.target != none) {
            const rect_s rect = (rect_s){ {(int16_t)((state.target - 1) * 16), 0}, { 16, 16 } };
            screen.draw(rsc.empty_tile, rect, at);
        }
        return;
    }
    const rect_s rect = tilestate_src_rect(state);
    screen.draw_aligned(rsc.tiles, rect, at);
}

void draw_tilestate(canvas_c &screen, const tilestate_t &state, point_s at, bool selected) {
    auto &rsc = cgresources_c::shared();
    if (state.type == empty) {
        const rect_s rect = (rect_s){at, {16, 16}};
        screen.draw(rsc.background, rect, at);
        switch (state.target) {
            case none:
                break;
            case both: {
                point_s o_at = at;
                o_at.x += 2;
                o_at.y += 2;
                rect_s rect = (rect_s){ {16, 0}, { 16, 16 } };
                screen.draw(rsc.empty_tile, rect, o_at);
                o_at.x -= 4;
                o_at.y -= 4;
                rect = (rect_s){ {0, 0}, { 16, 16 } };
                screen.draw(rsc.empty_tile, rect, o_at);
                break;
            }
            default: {
                const rect_s rect = (rect_s){ {(int16_t)((state.target - 1) * 16), 0}, { 16, 16 } };
                screen.draw(rsc.empty_tile, rect, at);
                break;
            }
        }
    } else {
        const rect_s rect = tilestate_src_rect(state);
        screen.draw(rsc.tiles, rect, at);
    }
    point_s o_at = at;
    switch (state.orb) {
        case none:
            break;
        case both: {
            o_at.x += 3;
            o_at.y += 6;
            draw_orb(screen, color_e::silver, o_at);
            o_at.x -= 6;
            o_at.y -= 6;
            draw_orb(screen, color_e::gold, o_at);
            break;
        }
        default:
            o_at.y += 3;
            draw_orb(screen, state.orb, o_at);
            break;
    }
    if (selected) {
        screen.draw(rsc.selection, at);
    }
}

inline static void draw_orb(canvas_c &screen, const cgresources_c &rsc, color_e color, int shade, int x, int y) {
    int16_t tx = (color - 1) * 16 + 3;
    int16_t ty = 10 * shade;
    const rect_s rect = (rect_s){{tx, ty}, {10, 10}};
    const point_s at = (point_s){(int16_t)(x * 16 + 3), (int16_t)(y * 16 + 3)};
    screen.draw(rsc.orbs, rect, at);
}

void draw_orb(canvas_c &screen, color_e color, point_s at) {
    auto &rsc = cgresources_c::shared();
    int16_t tx = (color - 1) * 16 + 3;
    const rect_s rect = (rect_s){{tx, 0}, {10, 10}};
    at.x += 3;
    screen.draw(rsc.orbs, rect, at);
}

void level_t::draw_tile(canvas_c &screen, int x, int y) const {
    auto &rsc = cgresources_c::shared();
    auto &tile = _grid->tiles[x][y];
    if (tile.state.type == empty && tile.state.target == none) {
        return;
    } else {
        if (tile.transition.step > 0) {
            draw_tilestate(screen, rsc, tile.transition.from_state, x, y);
            const int shade = canvas_c::STENCIL_FULLY_OPAQUE - tile.transition.step * canvas_c::STENCIL_FULLY_OPAQUE / tile_c::STEP_MAX;
            auto stencil = canvas_c::get_stencil(canvas_c::orderred, shade);
            screen.with_stencil(stencil, [&, this] {
                draw_tilestate(screen, rsc, tile.state, x, y);
            });
        } else {
            draw_tilestate(screen, rsc, tile.state, x, y);
        }
        
        if (tile.state.orb != color_e::none) {
            draw_orb(screen, rsc, tile.state.orb, 0, x, y);
        } else if (tile.transition.step > 0 && tile.transition.from_state.orb != color_e::none) {
            const int shade = 7 - tile.transition.step * 7 / tile_c::STEP_MAX;
            draw_orb(screen, rsc, tile.transition.from_state.orb, shade, x, y);
        }
    }
}

void level_t::draw_time(canvas_c &screen) const {
    auto &rsc = cgresources_c::shared();
    int min = _results.time / 60;
    int sec = _results.time % 60;
    char buf[5];
    buf[0] = '0' + min;
    buf[1] = ':';
    buf[2] = '0' + (sec / 10);
    buf[3] = '0' + (sec % 10);
    buf[4] = 0;
    
    const point_s at = (point_s){ TIME_X_TRAIL - 32, TIME_Y_INSET };
    const rect_s rect = (rect_s){at, (size_s){32, 8}};

    screen.draw(rsc.background, rect, at);
    screen.draw(rsc.mono_font, buf, at, canvas_c::align_left);
}

void level_t::draw_orb_counts(canvas_c &screen) const {
    auto &rsc = cgresources_c::shared();

    for (int i = 0; i < 2; i++) {
        char buf[3];
        auto d1 = _results.orbs[i] / 10;
        buf[0] = d1 ? '0' + d1 :  ' ';
        buf[1] = '0' + _results.orbs[i] % 10;
        buf[2] = 0;
        point_s at = (point_s){(int16_t)(ORB_X_INSET + ORB_X_LEAD + i * ORB_X_SPACING), ORB_Y_INSET};
        rect_s rect = (rect_s){ at, {16, 8}};
        screen.draw(rsc.background, rect, at);
        screen.draw(rsc.mono_font, buf, at, canvas_c::align_left);
    }
}

void level_t::draw_move_count(canvas_c &screen) const {
    auto &rsc = cgresources_c::shared();
    int moves = _results.moves;
    char buf[4];
    buf[3] = 0;
    for (int i = 3; --i != -1; ) {
        uint8_t r = moves % 10;
        moves /= 10;
        buf[i] = (r == 0 && moves == 0 && i != 2) ? ' ' : '0' + r;
    }
    const point_s at = (point_s){ TIME_X_TRAIL - 24, MOVES_Y_INSET };
    const rect_s rect = (rect_s){at, (size_s){24, 8}};

    screen.draw(rsc.background, rect, at);
    screen.draw(rsc.mono_font, buf, at, canvas_c::align_left);
}

void level_t::draw_remaining_count(canvas_c &screen) const {
    auto &rsc = cgresources_c::shared();
    int moves = _remaining;
    char buf[4];
    buf[3] = 0;
    for (int i = 3; --i != -1; ) {
        uint8_t r = moves % 10;
        moves /= 10;
        buf[i] = (r == 0 && moves == 0 && i != 2) ? ' ' : '0' + r;
    }
    const point_s at = (point_s){ TIME_X_TRAIL - 24, REMAINING_Y_INSET };
    const rect_s rect = (rect_s){at, (size_s){24, 8}};

    screen.draw(rsc.background, rect, at);
    screen.draw(rsc.mono_font, buf, at, canvas_c::align_left);
}

level_t::state_e level_t::update_tick(canvas_c &screen, mouse_c &mouse, int passed_seconds) {
    if (passed_seconds) {
        _results.time -= passed_seconds;
        debug_cpu_color(DEBUG_CPU_LEVEL_DRAW_TIME);
        draw_time(screen);
    }
    debug_cpu_color(DBEUG_CPU_LEVEL_TICK);

    auto at = mouse.get_postion();
    at.x /= 16; at.y /= 16;

    if (at.x < grid_c::GRID_MAX && at.y < grid_c::GRID_MAX) {
        debug_cpu_color(DBEUG_CPU_LEVEL_RESOLVE);
        bool lb = mouse.get_state(mouse_c::left) == mouse_c::clicked;
        bool rb = mouse.get_state(mouse_c::right) == mouse_c::clicked;
        if (lb || rb) {
            cgp_tile_changes = no_changes;
            // Try remove an orb
            auto color = _grid->try_remove_orb_at(at.x, at.y);
            if (color != color_e::none) {
                _results.orbs[color - 1] += 1;
                goto done;
            }
            // Try add an orb
            color = lb ? color_e::gold : color_e::silver;
            if (_results.orbs[color - 1] > 0) {
                if (_grid->try_add_orb_at(color, at.x, at.y)) {
                    _results.orbs[color - 1] -= 1;
                    _grid->resolve_at(at.x, at.y);
                }
            }
        done:
            if ((cgp_tile_changes & added_orb) || cgp_tile_changes & removed_orb) {
                _results.moves += 1;
                draw_orb_counts(screen);
                draw_move_count(screen);
            }
            auto &rsc = cgresources_c::shared();
            if (cgp_tile_changes >= (broke_glass + fused_orb)) {
                rsc.fuse_break_tile.set_active();
            } else if (cgp_tile_changes >= broke_glass) {
                rsc.break_tile.set_active();
            } else if (cgp_tile_changes >= fused_orb) {
                rsc.fuse_orb.set_active();
            } else if (cgp_tile_changes >= added_orb) {
                rsc.drop_orb.set_active();
            } else if (cgp_tile_changes >= removed_orb) {
                rsc.take_orb.set_active();
            } else {
                rsc.no_drop_orb.set_active();
            }
        }
        debug_cpu_color(DEBUG_CPU_LEVEL_GRID_TICK);
        uint16_t remaining = 0;
        auto completed = _grid->tick(remaining, [this, &screen] (int x, int y) {
            debug_cpu_color(DEBUG_CPU_LEVEL_GRID_DRAW);
            draw_tile(screen, x, y);
            debug_cpu_color(DEBUG_CPU_LEVEL_GRID_TICK);
        });
        if (remaining != _remaining) {
            _remaining = remaining;
            draw_remaining_count(screen);
        }
        if (completed) {
            return success;
        }
    }
    
    return (int16_t)_results.time > 0 ? normal : failed;
}

bool level_recipe_t::empty() const {
    return header.width == 0 || header.height == 0;
}

int level_recipe_t::get_size() const {
    return sizeof(level_recipe_t) + sizeof(tilestate_t) * header.width * header.height;
}

bool level_recipe_t::save(iffstream_c &iff) {
    iff_group_s group;
    iff_chunk_s chunk;
    if (iff.begin(group, IFF_FORM)) {
        iff.write(&IFF_CGLV_ID);
        
        iff.begin(chunk, IFF_LVHD);
        iff.write(&header);
        iff.end(chunk);
        
        if (text) {
            iff.begin(chunk, IFF_TEXT);
            iff.write((uint8_t *)text, strlen(text) + 1);
            iff.end(chunk);
        }
        
        iff.begin(chunk, IFF_TSTS);
        for (int i = 0; i < header.width * header.height; i++) {
            if(!iff.write(&tiles[i])) {
                return false;
            }
        }
        iff.end(chunk);
        
        return iff.end(group);
    }
    return false;
}

bool level_recipe_t::load(iffstream_c &iff, iff_chunk_s &start_chunk) {
    assert(start_chunk.id == IFF_FORM_ID);
    iff_group_s group;
    if (iff.expand(start_chunk, group) && group.subtype == IFF_CGLV_ID) {
        iff_chunk_s chunk;
        iff.next(group, IFF_LVHD, chunk);
        assert(chunk.size == sizeof(level_recipe_t::header));
        iff.read(&header);
        
        if (iff.next(group, IFF_TEXT, chunk)) {
            text = (const char *)calloc(1, chunk.size);
            iff.read((uint8_t *)text, chunk.size);
        }

        iff.next(group, IFF_TSTS, chunk);
        for (int i = 0; i < header.width * header.height; i++) {
            iff.read(&tiles[i]);
        }
        return true;
    }
    return false;
}

bool level_result_t::save(iffstream_c &iff) {
    iff_chunk_s chunk;
    if (iff.begin(chunk, IFF_CGLR)) {
        iff.write(this);
        return iff.end(chunk);
    }
    return false;
}

bool level_result_t::load(iffstream_c &iff, iff_chunk_s &start_chunk) {
    assert(start_chunk.id == IFF_CGLR_ID);
    if (start_chunk.size != sizeof(level_result_t)) {
        return false;
    }
    return iff.read(this);
}
