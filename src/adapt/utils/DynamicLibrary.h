#pragma once
#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <system_error>

namespace mif::ila_lseexport {

using HandleT = void*;

class DynamicLibrary {
public:
    HandleT               lib = nullptr;
    std::filesystem::path tempFile;
    std::filesystem::path pdbFile;

public:
    DynamicLibrary();
    DynamicLibrary(std::filesystem::path const& path);
    DynamicLibrary(std::span<uint8_t const> data, std::span<uint8_t const> pdb = {});
    ~DynamicLibrary();

    DynamicLibrary(DynamicLibrary&& other) noexcept;
    DynamicLibrary& operator=(DynamicLibrary&& other) noexcept;
    DynamicLibrary(DynamicLibrary const&)            = delete;
    DynamicLibrary& operator=(DynamicLibrary const&) = delete;

    std::optional<std::system_error> load(std::filesystem::path const& path) noexcept;
    std::optional<std::system_error> load(std::span<uint8_t const> data, std::span<uint8_t const> pdb = {}) noexcept;
    std::optional<std::system_error> free() noexcept;

    void* getAddress(char const* name) noexcept;

    template <class T>
    T getAddress(char const* name) noexcept {
        return reinterpret_cast<T>(getAddress(name));
    }

    constexpr HandleT handle() const noexcept { return lib; }
};

} // namespace mif::ila_lseexport