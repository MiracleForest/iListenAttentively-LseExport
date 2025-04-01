#pragma once
#include <ll/api/mod/NativeMod.h>

namespace ila {

extern std::unordered_map<std::string, std::string> mEventNameAlias;

void exportEvent();

class LseExport {

public:
    static LseExport& getInstance();

    LseExport() : mSelf(*ll::mod::NativeMod::current()) {}

    [[nodiscard]] ll::mod::NativeMod& getSelf() const { return mSelf; }

    bool load();

    bool enable();

    bool disable();

    void registerDefaultEventsAlias();

    void exportEvent();

private:
    ll::mod::NativeMod&                          mSelf;
    std::unordered_map<std::string, std::string> mEventNameAlias;
};

} // namespace ila
