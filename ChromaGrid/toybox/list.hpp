//
//  list.hpp
//  ChromaGrid
//
//  Created by Fredrik Olsson on 2024-03-24.
//

#ifndef list_h
#define list_h

#include "static_allocator.hpp"
#include "utility.hpp"

namespace toybox {
    
    /**
     `list_c` is a minimal implementation of `std::forward_list` with a
     statically allocated backing store, for performance reasons.
     TODO: Treat Count of 0 as a dynamic list.
     */
    template<class Type, size_t Count = 16>
    class list_c {
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
                assert(allocator::alloc_size >= count);
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
            
            __forceinline reference operator*() const { return _node->value; }
            __forceinline pointer operator->() const { return &_node->value; }
            __forceinline _iterator_s operator++() { _node = _node->next; return *this; }
            __forceinline _iterator_s operator++(int) { auto &tmp = *this; _node = _node->next; return tmp; }
            __forceinline bool operator==(const _iterator_s& o) const { return _node == o._node; }
            __forceinline bool operator!=(const _iterator_s& o) const { return _node != o._node; }
            __forceinline _iterator_s(_node_s *node) : _node(node) {}
            template<typename = enable_if<!is_same<Type, TypeI>::value>>
            __forceinline _iterator_s(const _iterator_s<Type> &other) : _node(other._node) {}
            _node_s *_node;
        };
        typedef _iterator_s<Type> iterator;
        typedef _iterator_s<const Type> const_iterator;
        
        list_c() { _head = nullptr; }
        ~list_c() { clear(); }
        
        inline bool empty() const __pure { return _head == nullptr; }
        void clear() {
            auto it = before_begin();
            while (it._node->next) {
                erase_after(it);
            }
        }
        
        inline reference front() __pure { return _head->value; }
        inline const_reference front() const __pure { return _head->value; }
        
        inline iterator before_begin() __pure {
            auto before_head = const_cast<_node_s**>(&_head);
            return iterator(reinterpret_cast<_node_s*>(before_head));
        }
        inline const_iterator before_begin() const __pure {
            auto before_head = const_cast<_node_s**>(&_head);
            return const_iterator(reinterpret_cast<_node_s*>(before_head));
        }
        inline iterator begin() __pure { return iterator(_head); }
        inline const_iterator begin() const __pure { return const_iterator(_head); }
        inline iterator end() __pure { return iterator(nullptr); }
        inline const_iterator end() const __pure { return const_iterator(nullptr); }
    
        inline void push_front(const_reference value) {
            insert_after(before_begin(), value);
        }
        template<class ...Args>
        inline reference emplace_front(Args&& ...args) {
            return *emplace_after(before_begin(), forward<Args>(args)...);
        }
        inline iterator insert_after(const_iterator pos, const_reference value) {
            assert(owns_node(pos._node));
            pos._node->next = new _node_s{pos._node->next, value};
            return iterator(pos._node->next);
        }
        template<class ...Args>
        inline iterator emplace_after(iterator pos, Args&& ...args) {
            assert(owns_node(pos._node));
            pos._node->next = new _node_s(pos._node->next, forward<Args>(args)...);
            return iterator(pos._node->next);
        }
        inline void pop_front() {
            erase_after(before_begin());
        }
        inline iterator erase_after(const_iterator pos) {
            assert(owns_node(pos._node));
            auto tmp = pos._node->next;
            pos._node->next = tmp->next;
            delete tmp;
            return iterator(pos._node->next);
        }
        void splice_after(const_iterator pos, list_c &other, const_iterator it) {
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

#endif /* list_h */
