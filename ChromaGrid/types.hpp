//
//  types.hpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-11.
//

#ifndef types_h
#define types_h

#include "cincludes.hpp"

struct cgpoint_t {
    int16_t x, y;
    bool operator==(const cgpoint_t &p) const {
        return x == p.x && y == p.y;
    }
};

struct cgsize_t {
    int16_t width, height;
    bool operator==(const cgsize_t s) const {
        return width == s.width && height == s.height;
    }
    bool contains(const cgpoint_t point) const {
        return point.x >= 0 && point.y >= 0 && point.x < width && point.y < height;
    }
    bool is_empty() const {
        return width <= 0 || height <= 0;
    }
};

struct cgrect_t {
    cgpoint_t origin;
    cgsize_t size;
    bool operator==(const cgrect_t &r) const {
        return origin == r.origin && size == r.size;
    }
    bool contains(const cgpoint_t &point) const {
        const cgpoint_t at = (cgpoint_t){static_cast<int16_t>(point.x - origin.x), static_cast<int16_t>(point.y - origin.y)};
        return size.contains(at);
    }
    bool contained_by(const cgsize_t &size) const {
        if (origin.x < 0 || origin.y < 0) return false;
        if (origin.x + this->size.width > size.width) return false;
        if (origin.y + this->size.height > size.height) return false;
        return true;
    }
    cgrect_t intersection(const cgrect_t rect) const {
        const int16_t x = MAX(origin.x, rect.origin.x);
        const int16_t y = MAX(origin.y, rect.origin.y);
        const int16_t w = MAX(0, MIN(origin.x + size.width, rect.origin.x + rect.size.width) - x);
        const int16_t h = MAX(0, MIN(origin.y + size.height, rect.origin.y + rect.size.height) - y);
        return (cgrect_t){ {x, y},  {w, h} };
    }
};

#endif /* types_h */
