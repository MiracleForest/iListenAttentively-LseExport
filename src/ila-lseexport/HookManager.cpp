#include "ila-lseexport/HookManager.h"
#include <cstring>
#include <fmt/format.h>
#include <utility>

namespace mif::ila_lseexport {

struct HookManager::HookData {
    HookId              mId{};
    uintptr_t           mTarget{};
    ll::memory::FuncPtr mOriginal{};
    std::string         mSignature;
    Callback            mCcallback;
    DCCallback*         mDetour{};
    size_t              mExecuting{};
    bool                mActive{true};
    std::vector<bool>   mOriginCalled;

    ~HookData() {
        if (mDetour) dcbFreeCallback(mDetour);
    }
};

HookManager& HookManager::getInstance() {
    static HookManager instance;
    return instance;
}

HookManager::HookId HookManager::nextCallbackId() { return mNextId++; }

bool HookManager::markOriginCalled(HookId id) {
    auto it = mHooks.find(id);
    if (it == mHooks.end() || it->second->mOriginCalled.empty()) return false;
    it->second->mOriginCalled.back() = true;
    return true;
}

bool HookManager::wasOriginCalled(HookId id) const {
    auto it = mHooks.find(id);
    return it != mHooks.end() && !it->second->mOriginCalled.empty() && it->second->mOriginCalled.back();
}

ll::Expected<HookManager::HookId> HookManager::hook(
    std::uintptr_t           target,
    std::string              signature,
    Callback                 callback,
    ll::memory::HookPriority priority,
    bool                     suspendThreads
) {
    collectRemoved();
    if (!target) return ll::makeStringError("Invalid hook target");
    if (!callback) return ll::makeStringError("Invalid hook callback");

    auto data        = std::make_unique<HookData>();
    data->mId        = mNextId++;
    data->mTarget    = target;
    data->mSignature = std::move(signature);
    data->mCcallback = std::move(callback);
    data->mDetour    = dcbNewCallback(data->mSignature.c_str(), &HookManager::dispatch, data.get());
    if (!data->mDetour) return ll::makeStringError("Failed to create dynamic hook callback");

    int result = ll::memory::hook(
        reinterpret_cast<ll::memory::FuncPtr>(data->mTarget),
        reinterpret_cast<ll::memory::FuncPtr>(data->mDetour),
        &data->mOriginal,
        priority,
        suspendThreads
    );
    if (result != 0) return ll::makeStringError(fmt::format("Failed to hook target: {}", result));

    auto id = data->mId;
    mHooks.emplace(id, std::move(data));
    return id;
}

bool HookManager::unhook(HookId id, bool suspendThreads) {
    collectRemoved();
    auto it = mHooks.find(id);
    if (it == mHooks.end()) return false;

    auto& data = *it->second;
    if (!data.mActive) return true;
    if (!ll::memory::unhook(
            reinterpret_cast<ll::memory::FuncPtr>(data.mTarget),
            reinterpret_cast<ll::memory::FuncPtr>(data.mDetour),
            suspendThreads
        )) {
        return false;
    }
    data.mActive = false;
    if (data.mExecuting == 0) mHooks.erase(it);
    return true;
}

bool HookManager::unhookAll(bool suspendThreads) {
    collectRemoved();
    auto result{true};
    for (auto it = mHooks.begin(); it != mHooks.end();) {
        auto& data = *it->second;
        if (!data.mActive
            || ll::memory::unhook(
                reinterpret_cast<ll::memory::FuncPtr>(data.mTarget),
                reinterpret_cast<ll::memory::FuncPtr>(data.mDetour),
                suspendThreads
            )) {
            data.mActive = false;
            if (data.mExecuting == 0) {
                it = mHooks.erase(it);
            } else {
                ++it;
            }
        } else {
            result = false;
            ++it;
        }
    }
    return result;
}

void HookManager::collectRemoved() {
    for (auto it = mHooks.begin(); it != mHooks.end();) {
        auto& data = *it->second;
        if (!data.mActive && data.mExecuting == 0) {
            it = mHooks.erase(it);
        } else {
            ++it;
        }
    }
}

DCsigchar HookManager::dispatch(DCCallback*, DCArgs* args, DCValue* result, void* userData) noexcept {
    auto& data = *static_cast<HookData*>(userData);
    ++data.mExecuting;
    data.mOriginCalled.push_back(false);
    DCsigchar resultType = data.mSignature.back();
    try {
        resultType = data.mCcallback(data.mId, reinterpret_cast<uintptr_t>(data.mOriginal), args, result);
    } catch (...) {
        std::memset(result, 0, sizeof(*result));
    }
    data.mOriginCalled.pop_back();
    --data.mExecuting;
    return resultType;
}

} // namespace mif::ila_lseexport
