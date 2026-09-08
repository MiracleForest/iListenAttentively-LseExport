#include "adapt/utils/CompilerPredefine.h"
#include "fake_levilamina/LeviLamina.h"

extern "C" __declspec(dllexport) int const ll_memory_operator_overrided{};

void operator delete(void* p) noexcept { ll::memory::getDefaultAllocator().release(p); }

void operator delete(void* p, std::nothrow_t const&) noexcept { operator delete(p); }

void operator delete[](void* p) noexcept { operator delete(p); }

void operator delete[](void* p, std::nothrow_t const&) noexcept { operator delete[](p); }

void operator delete(void* p, size_t) noexcept { operator delete(p); }

void operator delete[](void* p, size_t) noexcept { operator delete[](p); }

void operator delete(void* p, std::align_val_t) noexcept { ll::memory::getDefaultAllocator().alignedRelease(p); }

void operator delete(void* p, std::align_val_t alignment, std::nothrow_t const&) noexcept {
    operator delete(p, alignment);
}
void operator delete[](void* p, std::align_val_t alignment) noexcept { operator delete(p, alignment); }

void operator delete[](void* p, std::align_val_t alignment, std::nothrow_t const&) noexcept {
    operator delete[](p, alignment);
}
void operator delete(void* p, size_t, std::align_val_t alignment) noexcept { operator delete(p, alignment); }

void operator delete[](void* p, size_t, std::align_val_t alignment) noexcept { operator delete[](p, alignment); }

[[nodiscard]] LL_ALLOCATOR void* operator new(size_t size) {
    if (void* const block = ll::memory::getDefaultAllocator().allocate(size)) {
        return block;
    }
    ll::memory::throwMemoryException(size);
}
[[nodiscard]] LL_NOTHROW_ALLOCATOR void* operator new(size_t size, std::nothrow_t const&) noexcept {
    return ll::memory::getDefaultAllocator().allocate(size);
}
[[nodiscard]] LL_ALLOCATOR void* operator new[](size_t size) { return operator new(size); }

[[nodiscard]] LL_NOTHROW_ALLOCATOR void* operator new[](size_t size, std::nothrow_t const& tag) noexcept {
    return operator new(size, tag);
}
[[nodiscard]] LL_ALLOCATOR void* operator new(size_t size, std::align_val_t alignment) {
    if (void* const block = ll::memory::getDefaultAllocator().alignedAllocate(size, static_cast<size_t>(alignment))) {
        return block;
    }
    ll::memory::throwMemoryException(size);
}
[[nodiscard]] LL_NOTHROW_ALLOCATOR void*
operator new(size_t size, std::align_val_t alignment, std::nothrow_t const&) noexcept {
    return ll::memory::getDefaultAllocator().alignedAllocate(size, static_cast<size_t>(alignment));
}
[[nodiscard]] LL_ALLOCATOR void* operator new[](size_t size, std::align_val_t alignment) {
    return operator new(size, alignment);
}
[[nodiscard]] LL_NOTHROW_ALLOCATOR void*
operator new[](size_t size, std::align_val_t alignment, std::nothrow_t const& tag) noexcept {
    return operator new(size, alignment, tag);
}