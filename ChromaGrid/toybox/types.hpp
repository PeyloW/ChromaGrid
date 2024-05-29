//
//  types.hpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-11.
//

#ifndef types_h
#define types_h

#include "cincludes.hpp"
#include "utility.hpp"

namespace toybox {
    
    using namespace toybox;
    
    struct point_s {
        constexpr point_s() : x(0), y(0) {}
        constexpr point_s(int16_t x, int16_t y) : x(x), y(y) {}
        int16_t x, y;
        bool operator==(const point_s &p) const {
            return x == p.x && y == p.y;
        }
    };
    
    struct size_s {
        constexpr size_s() : width(0), height(0) {}
        constexpr size_s(int16_t w, int16_t h) : width(w), height(h) {}
        int16_t width, height;
        bool operator==(const size_s s) const {
            return width == s.width && height == s.height;
        }
        bool contains(const point_s point) const {
            return point.x >= 0 && point.y >= 0 && point.x < width && point.y < height;
        }
        bool is_empty() const {
            return width <= 0 || height <= 0;
        }
    };
    
    struct rect_s {        
        constexpr rect_s() : origin(), size() {}
        constexpr rect_s(const point_s &o, const size_s &s) : origin(o), size(s) {}
        constexpr rect_s(int16_t x, int16_t y, int16_t w, int16_t h) : origin(x, y), size(w, h) {}

        point_s origin;
        size_s size;
        __forceinline int16_t max_x() const { return origin.x + size.width - 1; }
        __forceinline int16_t max_y() const { return origin.y + size.height - 1; }
        bool operator==(const rect_s &r) const {
            return origin == r.origin && size == r.size;
        }
        bool contains(const point_s &point) const {
            const point_s at = point_s(point.x - origin.x, point.y - origin.y);
            return size.contains(at);
        }
        bool contained_by(const size_s &size) const {
            if (origin.x < 0 || origin.y < 0) return false;
            if (origin.x + this->size.width > size.width) return false;
            if (origin.y + this->size.height > size.height) return false;
            return true;
        }
        bool clip_to(const size_s size, point_s &at) {
            if (at.x < 0) {
                this->size.width += at.x;
                if (this->size.width <= 0) return false;
                this->origin.x -= at.x;
                at.x = 0;
            }
            if (at.y < 0) {
                this->size.height += at.y;
                if (this->size.height <= 0) return false;
                this->origin.y -= at.y;
                at.y = 0;
            }
            const auto dx = size.width - (at.x + this->size.width);
            if (dx < 0) {
                this->size.width += dx;
                if (this->size.width <= 0) return false;
            }
            const auto dy = size.height - (at.y + this->size.height);
            if (dy < 0) {
                this->size.height += dy;
                if (this->size.height <= 0) return false;
            }
            return true;
        }
    };
    
}

#endif /* types_h */
