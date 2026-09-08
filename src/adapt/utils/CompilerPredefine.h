#pragma once

struct _IMAGE_DOS_HEADER;

namespace mif::ila_lseexport::internal {
extern "C" ::_IMAGE_DOS_HEADER    __ImageBase; // NOLINT(bugprone-reserved-identifier)
[[nodiscard]] __forceinline void* getCurrentModuleHandle() noexcept { return &__ImageBase; }
} // namespace mif::ila_lseexport::internal

#define LL_ALLOCATOR         _Ret_notnull_ _Post_writable_byte_size_(size) _VCRT_ALLOCATOR
#define LL_NOTHROW_ALLOCATOR _Ret_maybenull_ _Success_(return != NULL) _Post_writable_byte_size_(size) _VCRT_ALLOCATOR