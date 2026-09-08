#pragma once
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace mif::ila_lseexport {

std::optional<std::vector<uint8_t>> decompressLz4Block(std::span<uint8_t const> compressed, size_t rawSize) noexcept;

} // namespace mif::ila_lseexport
