//
//  forward_list.hpp
//  ChromaGrid
//
//  Created by Fredrik Olsson on 2024-03-24.
//

#ifndef forward_list_h
#define forward_list_h

#include "static_allocator.hpp"
#include "utility.hpp"

namespace toystd {

template<class T, size_t Count = 16>
class forward_list_c {
public:
    struct node_t {
        node_t *next;
        T value;
        template<class... Args>
        node_t(node_t *next, Args&&... args) : next(next), value(forward<Args>(args)...) {}
        inline ~node_t() = default;
        void *operator new(size_t count) {
            return allocator::allocate();
        }
        void operator delete(void *ptr) noexcept {
            allocator::deallocate(ptr);
        }
    };
    typedef static_allocator_c<node_t, Count> allocator;
    struct iterator {
        friend class cgforward_list_c;
        T &operator*() const { return _node->value; }
        T *operator->() const { return &_node->value; }
        iterator &operator++() { _node = _node->next; return *this; }
        iterator &operator++(int) { auto &tmp = *this; _node = _node->next; return tmp; }
        bool operator==(const iterator& o) const { return _node == o._node; }
        bool operator!=(const iterator& o) const { return _node != o._node; }
        iterator(node_t *node) : _node(node) {}
        node_t *_node;
    };
    forward_list_c() { _head = nullptr; }
    ~forward_list_c() { clear(); }
    
    bool empty() const { return _head == nullptr; }
    void clear() {
        auto it = before_begin();
        while (it._node->next) {
            erase_after(it);
        }
    }
    
    T &front() const { return _head->value; }
    iterator before_begin() const {
        auto before_head = const_cast<node_t**>(&_head);
        return iterator(reinterpret_cast<node_t*>(before_head));
    }
    iterator begin() const { return iterator(_head); }
    iterator end() const { return iterator(nullptr); }
    
    void push_front(const T &value) {
        insert_after(before_begin(), value);
    }
    template<class ...Args>
    void emplace_front(Args&& ...args) {
        emplace_after(before_begin(), forward<Args>(args)...);
    }
    void insert_after(iterator pos, const T &value) {
        assert(owns_node(pos._node));
        pos._node->next = new node_t{pos._node->next, value};
    }
    template<class ...Args>
    void emplace_after(iterator pos, Args&& ...args) {
        assert(owns_node(pos._node));
        pos._node->next = new node_t(pos._node->next, forward<Args>(args)...);
    }
    void pop_front() {
        erase_after(before_begin());
    }
    void erase_after(iterator pos) {
        assert(owns_node(pos._node));
        auto tmp = pos._node->next;
        pos._node->next = tmp->next;
        delete tmp;
    }
    void splice_after(iterator pos, forward_list_c &other, iterator it) {
        assert(owns_node(pos._node));
        assert(other.owns_node(it._node));
        auto tmp = it._node->next;
        it._node->next = tmp->next;
        tmp->next = pos._node->next;
        pos._node->next = tmp;
    }
private:
    bool owns_node(node_t * node) const {
        auto before_head = const_cast<node_t**>(&_head);
        auto n = reinterpret_cast<node_t*>(before_head);
        while (n) {
            if (n == node) return true;
            n = n->next;
        }
        return false;;
    }
    node_t *_head;
};

}

#endif /* forward_list_h */
