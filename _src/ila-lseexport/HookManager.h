#pragma once
#include <cstdint>
#include <dyncall_callback.h>
#include <functional>
#include <ll/api/Expected.h>
#include <ll/api/base/Containers.h>
#include <ll/api/memory/Hook.h>
#include <memory>
#include <string>

namespace ila {

class HookManager {
public:
    struct HookData;

    using HookId   = uint64;
    using Callback = std::function<DCsigchar(HookId, uintptr_t, DCArgs*, DCValue*)>;

private:
    HookId                                               mNextId{1};
    ll::SmallDenseMap<HookId, std::unique_ptr<HookData>> mHooks;

public:
    HookManager() = default;

public:
    static HookManager& getInstance();

    HookId nextCallbackId();
    bool   markOriginCalled(HookId id);
    bool   wasOriginCalled(HookId id) const;

    ll::Expected<HookId> hook(
        uintptr_t                target,
        std::string              signature,
        Callback                 callback,
        ll::memory::HookPriority priority,
        bool                     suspendThreads = true
    );

    bool unhook(HookId id, bool suspendThreads = true);
    bool unhookAll(bool suspendThreads = true);

private:
    static DCsigchar dispatch(DCCallback*, DCArgs* args, DCValue* result, void* userData) noexcept;
    void             collectRemoved();
};

} // namespace ila
