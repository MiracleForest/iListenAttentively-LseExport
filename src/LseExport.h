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

private:
    ll::mod::NativeMod& mSelf;
};

void exportEvent();

} // namespace ila
