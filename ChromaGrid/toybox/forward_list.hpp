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

template<class Type, size_t Count = 16>
class forward_list_c {
public:
    typedef Type value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    struct _node_s {
        _node_s *next;
        value_type value;
        template<class... Args>
        _node_s(_node_s *next, Args&&... args) : next(next), value(forward<Args>(args)...) {}
        inline ~_node_s() = default;
        void *operator new(size_t count) {
            return allocator::allocate();
        }
        void operator delete(void *ptr) noexcept {
            allocator::deallocate(ptr);
        }
    };
    typedef static_allocator_c<_node_s, Count> allocator;
    template<class TypeI>
    struct _iterator_s {
        typedef TypeI value_type;
        typedef value_type* pointer;
        typedef value_type& reference;

        reference operator*() const { return _node->value; }
        pointer operator->() const { return &_node->value; }
        _iterator_s operator++() { _node = _node->next; return *this; }
        _iterator_s operator++(int) { auto &tmp = *this; _node = _node->next; return tmp; }
        bool operator==(const _iterator_s& o) const { return _node == o._node; }
        bool operator!=(const _iterator_s& o) const { return _node != o._node; }
        _iterator_s(_node_s *node) : _node(node) {}
        template<typename = enable_if<!is_same<Type, TypeI>::value>>
        _iterator_s(const _iterator_s<Type> &other) : _node(other._node) {}
        _node_s *_node;
    };
    typedef _iterator_s<Type> iterator;
    typedef _iterator_s<const Type> const_iterator;

    forward_list_c() { _head = nullptr; }
    ~forward_list_c() { clear(); }
    
    bool empty() const __pure { return _head == nullptr; }
    void clear() {
        auto it = before_begin();
        while (it._node->next) {
            erase_after(it);
        }
    }
    
    reference front() __pure { return _head->value; }
    const_reference front() const __pure { return _head->value; }
    
    iterator before_begin() __pure {
        auto before_head = const_cast<_node_s**>(&_head);
        return iterator(reinterpret_cast<_node_s*>(before_head));
    }
    const_iterator before_begin() const __pure {
        auto before_head = const_cast<_node_s**>(&_head);
        return const_iterator(reinterpret_cast<_node_s*>(before_head));
    }
    iterator begin() __pure { return iterator(_head); }
    const_iterator begin() const __pure { return const_iterator(_head); }
    iterator end() __pure { return iterator(nullptr); }
    const_iterator end() const __pure { return const_iterator(nullptr); }

    void push_front(const_reference value) {
        insert_after(before_begin(), value);
    }
    template<class ...Args>
    void emplace_front(Args&& ...args) {
        emplace_after(before_begin(), forward<Args>(args)...);
    }
    void insert_after(const_iterator pos, const_reference value) {
        assert(owns_node(pos._node));
        pos._node->next = new _node_s{pos._node->next, value};
    }
    template<class ...Args>
    void emplace_after(const_iterator pos, Args&& ...args) {
        assert(owns_node(pos._node));
        pos._node->next = new _node_s(pos._node->next, forward<Args>(args)...);
    }
    void pop_front() {
        erase_after(before_begin());
    }
    void erase_after(const_iterator pos) {
        assert(owns_node(pos._node));
        auto tmp = pos._node->next;
        pos._node->next = tmp->next;
        delete tmp;
    }
    void splice_after(const_iterator pos, forward_list_c &other, const_iterator it) {
        assert(owns_node(pos._node));
        assert(other.owns_node(it._node));
        auto tmp = it._node->next;
        it._node->next = tmp->next;
        tmp->next = pos._node->next;
        pos._node->next = tmp;
    }
private:
    bool owns_node(_node_s * node) const __pure {
        auto before_head = const_cast<_node_s**>(&_head);
        auto n = reinterpret_cast<_node_s*>(before_head);
        while (n) {
            if (n == node) return true;
            n = n->next;
        }
        return false;;
    }
    _node_s *_head;
};

}

#endif /* forward_list_h */
