#include "adapt/utils/Lz4Utils.h"
#include <limits>
#include <lz4.h>

namespace mif::ila_lseexport {

std::optional<std::vector<uint8_t>> decompressLz4Block(std::span<uint8_t const> compressed, size_t rawSize) noexcept
    try {
    if (rawSize == 0) return std::vector<uint8_t>{};
    if (compressed.empty()) return std::nullopt;

    if (compressed.size() > static_cast<size_t>((std::numeric_limits<int>::max)())
        || rawSize > static_cast<size_t>((std::numeric_limits<int>::max)())) {
        return std::nullopt;
    }

    std::vector<uint8_t> result(rawSize);
    int const            decoded = LZ4_decompress_safe(
        reinterpret_cast<char const*>(compressed.data()),
        reinterpret_cast<char*>(result.data()),
        static_cast<int>(compressed.size()),
        static_cast<int>(result.size())
    );
    if (decoded < 0 || static_cast<size_t>(decoded) != rawSize) return std::nullopt;
    return result;
} catch (...) {
    return std::nullopt;
}

} // namespace mif::ila_lseexport
