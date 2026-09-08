#include "ila-lseexport/LseExport.h"
#include "ila-lseexport/HookManager.h"
#include <Windows.h>
#include <ll/api/event/EventBus.h>
#include <ll/api/mod/RegisterHelper.h>
#include <ll/api/service/GamingStatus.h>
#include <ll/api/thread/GlobalThreadPauser.h>
#include <optional>

namespace mif::ila_lseexport {

LseExport& LseExport::getInstance() {
    static LseExport instance;
    return instance;
}

bool LseExport::load() {
    exportEvent();
    registerDefaultEventsAlias();
    return true;
}

bool LseExport::enable() { return true; }

bool LseExport::disable() {
    HookManager::getInstance().unhookAll();
    return true;
}

bool LseExport::unload() { return true; }

void LseExport::modify(void* ptr, size_t length, brstd::function_ref<void()> callback, bool pauseThread) {
    std::optional<ll::thread::GlobalThreadPauser> pauser;
    if (pauseThread && ll::getGamingStatus() != ll::GamingStatus::Default) {
        pauser.emplace();
    }
    DWORD oldProtect;
    VirtualProtect(ptr, length, PAGE_EXECUTE_READWRITE, &oldProtect);
    callback();
    VirtualProtect(ptr, length, oldProtect, &oldProtect);
}

LL_REGISTER_MOD(LseExport, LseExport::getInstance());

} // namespace mif::ila_lseexport