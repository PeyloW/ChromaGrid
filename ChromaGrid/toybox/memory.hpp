//
//  memory.hpp
//  toybox
//
//  Created by Fredrik on 2024-04-16.
//

#ifndef memory_h
#define memory_h

#include "static_allocator.hpp"

namespace toystd {

    template<typename T>
    class __packed_struct basic_ptr_c {
    public:
        basic_ptr_c(T* ptr = nullptr) : _ptr(ptr) {}
        
        T* get() const __pure { return _ptr; }
        
        
        __forceinline T* operator->() const __pure  { return _ptr; }
        __forceinline T& operator*() const __pure  { return *(_ptr); }
        __forceinline T& operator[](int i) const __pure  { return _ptr[i]; }
        __forceinline T* operator+(int16_t i) const __pure  { return _ptr + i; }
        __forceinline T* operator+(uint16_t i) const __pure  { return _ptr + i; }
        __forceinline T* operator+(int32_t i) const __pure  { return _ptr + i; }
        __forceinline T* operator+(uint32_t i) const __pure  { return _ptr + i; }
        
        __forceinline explicit operator bool() const __pure  { return _ptr != nullptr; }
        __forceinline bool operator==(const basic_ptr_c &o) const __pure { return _ptr == o._ptr; }
        __forceinline bool operator==(T *o) const __pure { return _ptr == o; }
        __forceinline bool operator!=(const basic_ptr_c &o) const __pure { return _ptr != o._ptr; }
        __forceinline bool operator!=(T *o) const __pure { return _ptr != o; }

    protected:
        T* _ptr;
    };
    static_assert(sizeof(basic_ptr_c<void*>) == sizeof(void*), "basic_ptr_c size mismatch.");
    
    template<typename T>
    class __packed_struct unique_ptr_c : public basic_ptr_c<T>, public nocopy_c {
    public:
        unique_ptr_c(T* ptr = nullptr) : basic_ptr_c<T>(ptr) {}
        ~unique_ptr_c() { cleanup(); }

        unique_ptr_c(unique_ptr_c &&o) {
            if (this->_ptr != o._ptr) cleanup();
            this->_ptr = o._ptr;
            o._ptr = nullptr;
        }

        void reset(T* p = nullptr) {
            if (this->_ptr != p) cleanup();
            this->_ptr = p;
        }
        
    private:
        void cleanup() {
            if (this->_ptr) {
                delete this->_ptr;
                this->_ptr = nullptr;
            }
        }
    };
    static_assert(sizeof(unique_ptr_c<void*>) == sizeof(void*), "unique_ptr_c size mismatch.");

    namespace detail {
        struct __packed_struct shared_count_t {
            shared_count_t() : count(1) {}
            uint16_t count;
            void *operator new(size_t count) {
                return allocator::allocate();
            }
            void operator delete(void *ptr) noexcept {
                allocator::deallocate(ptr);
            }
            typedef static_allocator_c<shared_count_t, 64> allocator;
        };
    }
    
    template<typename T>
    class __packed_struct shared_ptr_c : public basic_ptr_c<T> {
    public:
        shared_ptr_c(T* ptr = nullptr) : basic_ptr_c<T>(ptr), _count(ptr ? new detail::shared_count_t() : nullptr) {}
        ~shared_ptr_c() { cleanup(); }
        shared_ptr_c(const shared_ptr_c &o) {
            if (this->_ptr != o._ptr) cleanup();
            this->_ptr = o._ptr;
            take_count(o._count);
        }
        shared_ptr_c(shared_ptr_c &&o) {
            if (this->_ptr != o._ptr) cleanup();
            this->_ptr = o._ptr;
            take_count(o._count);
            o.cleanup();
        }
        shared_ptr_c& operator=(const shared_ptr_c &o) {
            if (this->_ptr != o._ptr) cleanup();
            this->_ptr = o._ptr;
            take_count(o._count);
            return *this;
        }
        shared_ptr_c& operator=(shared_ptr_c &&o) {
            if (this->_ptr != o._ptr) cleanup();
            this->_ptr = o._ptr;
            take_count(o._count);
            o.cleanup();
            return *this;
        }

        uint16_t get_count() const __pure { return _count->count; }
        void reset(T* p = nullptr) {
            if (this->_ptr != p) cleanup();
            this->_ptr = p;
            _count = p ? new detail::shared_count_t() : nullptr;
        }
        
    private:
        detail::shared_count_t *_count;
        void cleanup() {
            if (_count) {
                _count->count--;
                if (_count->count == 0) {
                    delete this->_ptr;
                    delete _count;
                    this->_ptr = nullptr;
                    _count = nullptr;
                }
            }
        }
        void take_count(detail::shared_count_t *count) {
            this->_count = count;
            if (this->_count) {
                this->_count->count++;
            }
        }
    };
    static_assert(sizeof(shared_ptr_c<void*>) == sizeof(void*) * 2, "shared_ptr_c size mismatch.");

}

#endif /* memory_h */
