#include "adapt/utils/DynamicLibrary.h"
#include "adapt/utils/StringUtils.h"
#include <Windows.h>
#include <chrono>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <fstream>
#include <limits>
#include <optional>
#include <random>
#include <span>
#include <system_error>
#include <utility>

namespace mif::ila_lseexport {

namespace {
struct u8system_category : public std::_System_error_category {
    constexpr u8system_category() noexcept : _System_error_category() {}
    [[nodiscard]] std::string message(int errCode) const override {
        std::_System_error_message msg{static_cast<unsigned long>(errCode)};
        if (msg._Length) {
            std::string res{string_utils::str2str({msg._Str, msg._Length})};
            if (res.ends_with('\n')) {
                res.pop_back();
                if (res.ends_with('\r')) {
                    res.pop_back();
                }
            }
            return string_utils::replaceAll(res, "\r\n", ", ");
        }
        return "unknown error";
    }
};

inline std::error_category const& u8system_category() noexcept {
    return std::_Immortalize_memcpy_image<struct u8system_category>();
}

std::system_error getLastSystemError() noexcept { return std::error_code{(int)GetLastError(), u8system_category()}; }

std::expected<std::wstring, std::system_error> getTempDir() {
    std::wstring tempDir(32768, L'\0');
    auto         length = GetTempPathW(static_cast<DWORD>(tempDir.size()), tempDir.data());
    if (length == 0 || length >= tempDir.size()) return std::unexpected{getLastSystemError()};
    tempDir.resize(length);
    return tempDir;
}

std::wstring makeRandomSuffix() noexcept {
    constexpr wchar_t hexChars[] = L"0123456789abcdef";
    std::wstring      result;
    result.reserve(8);
    try {
        std::random_device rd;
        for (int i = 0; i < 8; ++i) result += hexChars[rd() % 16];
    } catch (...) {
        auto value = static_cast<unsigned long long>(std::chrono::steady_clock::now().time_since_epoch().count());
        while (value != 0) {
            result += hexChars[value % 16];
            value  /= 16;
        }
    }
    return result;
}

std::optional<std::system_error> writeFile(std::filesystem::path const& file, std::span<uint8_t const> data) noexcept {
    std::error_code ec;
    std::filesystem::create_directories(file.parent_path(), ec);
    if (ec) return std::system_error{ec};
    std::ofstream stream;
    stream.open(file, std::ios_base::out | std::ios_base::binary);
    if (!stream.is_open()) return std::system_error{std::make_error_code(std::errc::io_error)};
    stream.write(reinterpret_cast<char const*>(data.data()), static_cast<std::streamsize>(data.size()));
    stream.close();
    if (!stream) return std::system_error{std::make_error_code(std::errc::io_error)};
    return std::nullopt;
}
} // namespace

DynamicLibrary::DynamicLibrary() = default;
DynamicLibrary::DynamicLibrary(std::filesystem::path const& path) { load(path); }
DynamicLibrary::DynamicLibrary(std::span<uint8_t const> data, std::span<uint8_t const> pdb) { load(data, pdb); }
DynamicLibrary::~DynamicLibrary() { free(); }
DynamicLibrary::DynamicLibrary(DynamicLibrary&& other) noexcept
: lib{std::exchange(other.lib, nullptr)},
  tempFile{std::move(other.tempFile)},
  pdbFile{std::move(other.pdbFile)} {}
DynamicLibrary& DynamicLibrary::operator=(DynamicLibrary&& other) noexcept {
    if (this != &other) {
        (void)free();
        lib      = std::exchange(other.lib, nullptr);
        tempFile = std::move(other.tempFile);
        pdbFile  = std::move(other.pdbFile);
    }
    return *this;
}

std::optional<std::system_error> DynamicLibrary::load(std::filesystem::path const& path) noexcept {
    if (lib) return std::system_error{{}};

    lib = LoadLibraryW(path.c_str());
    if (!lib) return getLastSystemError();

    return {};
}

std::optional<std::system_error> DynamicLibrary::load(std::span<uint8_t const> data, std::span<uint8_t const> pdb) noexcept {
    if (lib) return std::system_error{{}};
    auto tempDir = getTempDir();
    if (!tempDir) return tempDir.error();

    tempFile = std::filesystem::path{*tempDir}
             / (L"iListenAttentively-LseExport-" + std::to_wstring(GetCurrentProcessId()) + L"-" + makeRandomSuffix()
                + L".dll");

    if (auto error = writeFile(tempFile, data)) return error;

    if (!pdb.empty()) {
        pdbFile = tempFile;
        pdbFile.replace_extension(L".pdb");
        if (auto error = writeFile(pdbFile, pdb)) return error;
    }

    return load(tempFile);
}

std::optional<std::system_error> DynamicLibrary::free() noexcept {
    if (lib) {
        if (!FreeLibrary((HMODULE)lib)) return getLastSystemError();
        lib = nullptr;
    }
    if (!tempFile.empty()) {
        std::error_code ec;
        std::filesystem::remove(tempFile, ec);
        tempFile.clear();
    }
    if (!pdbFile.empty()) {
        std::error_code ec;
        std::filesystem::remove(pdbFile, ec);
        pdbFile.clear();
    }
    return {};
}
void* DynamicLibrary::getAddress(char const* name) noexcept { return GetProcAddress((HMODULE)lib, name); }

} // namespace mif::ila_lseexport