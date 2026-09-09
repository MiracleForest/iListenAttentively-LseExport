#pragma once
#include "fake_levilamina/Version.h"
#include <format>
#include <fstream>
#include <functional>
#include <memory>

#ifdef LL_EXPORT
#define LLAPI [[maybe_unused]] __declspec(dllexport)
#else
#define LLAPI [[maybe_unused]] __declspec(dllimport)
#endif

namespace Bedrock::Memory {
class IMemoryAllocator {
public:
    virtual ~IMemoryAllocator()                       = default;
    virtual void* allocate(uint64_t)                  = 0;
    virtual void  release(void*)                      = 0;
    virtual void* alignedAllocate(uint64_t, uint64_t) = 0;
    virtual void  alignedRelease(void*)               = 0;
};
} // namespace Bedrock::Memory

namespace ll {

LLAPI ll::data::Version getGameVersion();
LLAPI ll::data::Version getLoaderVersion();

namespace memory {
using FuncPtr = void*;
LLAPI FuncPtr resolveSymbol(char const* symbol);
LLAPI Bedrock::Memory::IMemoryAllocator& getDefaultAllocator();
[[noreturn]] LLAPI void throwMemoryException(size_t);
} // namespace memory

class OutputStream {
public:
    alignas(8) char filter[0x2A0];

private:
    LLAPI void print(std::string_view) const noexcept;

public:
    void operator()(std::string_view str) const noexcept { print(str); }
};
class Logger {
public:
    std::string                  title;
    std::optional<std::ofstream> ofs{std::nullopt};
    int                          consoleLevel{-1};
    int                          fileLevel{-1};
    int                          playerLevel{-1};

    OutputStream debug;
    OutputStream info;
    OutputStream warn;
    OutputStream error;
    OutputStream fatal;
};

namespace io {
enum class LogLevel : int {
    Off = -1,
    Fatal,
    Error,
    Warn,
    Info,
    Debug,
    Trace,
};
class Logger : public std::enable_shared_from_this<Logger> {
public:
    Logger()                         = default;
    ~Logger()                        = default;
    Logger(Logger const&)            = delete;
    Logger(Logger&&)                 = delete;
    Logger& operator=(Logger const&) = delete;
    Logger& operator=(Logger&&)      = delete;

private:
    LLAPI void printStr(LogLevel, std::string&&) const noexcept;

public:
    void       operator()(LogLevel level, std::string&& str) const noexcept { printStr(level, std::move(str)); }
    LLAPI bool shouldLog(LogLevel level) const noexcept;
};
} // namespace io

namespace plugin {
class Plugin {
public:
    Plugin(Plugin const&)            = delete;
    Plugin(Plugin&&)                 = delete;
    Plugin& operator=(Plugin const&) = delete;
    Plugin& operator=(Plugin&&)      = delete;

    using callback_t = bool(Plugin&);
    using CallbackFn = std::function<callback_t>;

    LLAPI void onLoad(CallbackFn) noexcept;
    LLAPI void onUnload(CallbackFn) noexcept;
    LLAPI void onEnable(CallbackFn) noexcept;
    LLAPI void onDisable(CallbackFn) noexcept;

    LLAPI Logger& getLogger() const;

    template <typename... Args>
    void fatal(std::format_string<Args...> fmt, Args&&... args) const {
        getLogger().fatal(std::vformat(fmt.get(), std::make_format_args(args...)));
    }

    template <typename... Args>
    void debug(std::format_string<Args...> fmt, Args&&... args) const {
        getLogger().debug(std::vformat(fmt.get(), std::make_format_args(args...)));
    }
};
class NativePlugin : public Plugin {
public:
    LLAPI static std::shared_ptr<NativePlugin> getByHandle(void* handle);
};
} // namespace plugin

namespace mod {
class Mod {
public:
    Mod(Mod const&)            = delete;
    Mod(Mod&&)                 = delete;
    Mod& operator=(Mod const&) = delete;
    Mod& operator=(Mod&&)      = delete;

    using callback_t = bool(Mod&);
    using CallbackFn = std::function<callback_t>;

    LLAPI void onLoad(CallbackFn) noexcept;
    LLAPI void onUnload(CallbackFn) noexcept;
    LLAPI void onEnable(CallbackFn) noexcept;
    LLAPI void onDisable(CallbackFn) noexcept;

    LLAPI io::Logger& getLogger() const;

    template <typename... Args>
    void fatal(std::format_string<Args...> fmt, Args&&... args) const {
        getLogger()(ll::io::LogLevel::Fatal, std::vformat(fmt.get(), std::make_format_args(args...)));
    }

    template <typename... Args>
    void debug(std::format_string<Args...> fmt, Args&&... args) const {
        getLogger()(ll::io::LogLevel::Debug, std::vformat(fmt.get(), std::make_format_args(args...)));
    }
};
class NativeMod : public Mod {
public:
    LLAPI static std::shared_ptr<NativeMod> getByHandle(void* handle);
};
} // namespace mod

} // namespace ll
