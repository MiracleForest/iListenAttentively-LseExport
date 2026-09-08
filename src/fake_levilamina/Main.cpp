#include "fake_levilamina/LeviLamina.h"

namespace ll {

static io::Logger logger;

void ll::OutputStream::print(std::string_view) const noexcept {}
namespace io {
void Logger::printStr(LogLevel, std::string&&) const noexcept {}
bool Logger::shouldLog(LogLevel) const noexcept { return false; }
} // namespace io
namespace plugin {
void                          Plugin::onLoad(CallbackFn) noexcept {}
void                          Plugin::onUnload(CallbackFn) noexcept {}
void                          Plugin::onEnable(CallbackFn) noexcept {}
void                          Plugin::onDisable(CallbackFn) noexcept {}
io::Logger&                   Plugin::getLogger() const { return logger; }
std::shared_ptr<NativePlugin> NativePlugin::getByHandle(void*) { return {}; }
} // namespace plugin
namespace mod {
void                       Mod::onLoad(CallbackFn) noexcept {}
void                       Mod::onUnload(CallbackFn) noexcept {}
void                       Mod::onEnable(CallbackFn) noexcept {}
void                       Mod::onDisable(CallbackFn) noexcept {}
io::Logger&                Mod::getLogger() const { return logger; }
std::shared_ptr<NativeMod> NativeMod::getByHandle(void*) { return {}; }
} // namespace mod
namespace memory {
FuncPtr resolveSymbol(char const*) { return nullptr; }
} // namespace memory
ll::data::Version getGameVersion() { return {}; }
ll::data::Version getLoaderVersion() { return {}; }
} // namespace ll