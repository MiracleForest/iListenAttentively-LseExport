#pragma once
#include <cstddef>
#include <ll/api/mod/NativeMod.h>
#include <mc/platform/brstd/function_ref.h>
#include <string>
#include <unordered_map>

namespace mif::ila_lseexport {

class LseExport {

public:
    static LseExport& getInstance();

    LseExport() : mSelf(*ll::mod::NativeMod::current()) {}

    [[nodiscard]] ll::mod::NativeMod& getSelf() const { return mSelf; }

    bool load();
    bool enable();
    bool disable();
    bool unload();

    void exportEvent();
    void registerDefaultEventsAlias();

    static void modify(void* ptr, size_t length, brstd::function_ref<void()> callback, bool pauseThread = false);

private:
    ll::mod::NativeMod&                          mSelf;
    std::unordered_map<std::string, std::string> mEventNameAlias;
};

} // namespace mif::ila_lseexport