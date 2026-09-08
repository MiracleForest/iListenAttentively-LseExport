#pragma once

struct _IMAGE_DOS_HEADER;

namespace mif::ila_lseexport::internal {
extern "C" ::_IMAGE_DOS_HEADER __ImageBase; // NOLINT(bugprone-reserved-identifier)

[[nodiscard]] __forceinline void* getCurrentModuleHandle() noexcept { return &__ImageBase; }

} // namespace mif::ila_lseexport::internal