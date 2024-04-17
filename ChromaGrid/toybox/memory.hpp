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
    class basic_ptr_c {
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
    
    template<typename T>
    class unique_ptr_c : public basic_ptr_c<T>, public nocopy_c {
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
    
}

#endif /* memory_h */
