//
//  types.hpp
//  ChromaGrid
//
//  Created by Fredrik on 2024-02-11.
//

#ifndef types_h
#define types_h

#include "cincludes.hpp"

class cgnocopy_c {
protected:
    __forceinline cgnocopy_c() {}
    cgnocopy_c(const cgnocopy_c&) = delete;
    cgnocopy_c& operator=(const cgnocopy_c&) = delete;
};

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
    bool clip_to(const cgsize_t size, cgpoint_t &at) {
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

static inline int sqrt(int x) {
    if (x == 0 || x == 1) {
        return x;
    } else {
        int i = 1, result = 1;
        while (result <= x) {
            i++;
            result = i * i;
        }
        return i - 1;
    }
}

extern void* operator new (size_t count, void *p);

template<typename T> struct cgremove_reference      { typedef T type; };
template<typename T> struct cgremove_reference<T&>  { typedef T type; };
template<typename T> struct cgremove_reference<T&&> { typedef T type; };

template <class T>
__forceinline T&& cgforward(typename cgremove_reference<T>::type& t) {
    return static_cast<T&&>(t);
}
template <class T>
__forceinline T&& cgforward(typename cgremove_reference<T>::type&& t) {
    return static_cast<T&&>(t);
}

template<class TYPE, int COUNT>
class cgvector_c : cgnocopy_c {
public:
    inline cgvector_c() : _size(0) {}
    
    __forceinline TYPE *begin() const { return (TYPE *)&_buffer[0]; }
    __forceinline TYPE *end() const { return begin() + _size; }
    __forceinline int size() const { return _size; }
    
    inline TYPE& operator[](const int i) const {
        assert(i < _size);
        return begin()[i];
    }
    inline TYPE& front() const {
        assert(_size > 0);
        return *begin();
    }
    inline TYPE& back() const {
        assert(_size > 0);
        return *(end() - 1);
    }

    inline void push_back(const TYPE& value) {
        assert(_size < COUNT);
        begin()[_size++] = value;
    }
    template<class... Args>
    inline void emplace_back(Args&&... args) {
        assert(_size < COUNT);
        new (begin() + _size++) TYPE(cgforward<Args>(args)...);
    }

    inline void clear() {
        _size = 0;
    }
    inline void pop_back() {
        assert(_size > 0);
        _size--;
    }

private:
    uint8_t _buffer[sizeof(TYPE) * COUNT];
    int _size;
};

#endif /* types_h */
