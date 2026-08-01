#pragma once
#include <ll/api/mod/NativeMod.h>

namespace ila {

class LseExport {

public:
    static LseExport& getInstance();

    LseExport() : mSelf(*ll::mod::NativeMod::current()) {}

    [[nodiscard]] ll::mod::NativeMod& getSelf() const { return mSelf; }

    bool load();
    bool enable();
    bool disable();

    void exportEventV1();
    void registerDefaultEventsAliasV1();
    void exportEventV2();
    void registerDefaultEventsAliasV2();
    void exportEventV3();
    void registerDefaultEventsAliasV3();

    static void modify(void* ptr, size_t length, brstd::function_ref<void()> callback, bool pauseThread = false);

private:
    ll::mod::NativeMod&                          mSelf;
    std::unordered_map<std::string, std::string> mEventNameAlias;
};

} // namespace ila
