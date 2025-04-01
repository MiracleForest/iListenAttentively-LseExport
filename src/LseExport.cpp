#include "LseExport.h"
#include <ll/api/event/EventBus.h>
#include <ll/api/mod/RegisterHelper.h>
#include <ll/api/utils/StringUtils.h>

namespace ila {

LseExport& LseExport::getInstance() {
    static LseExport instance;
    return instance;
}

bool LseExport::load() {
    registerDefaultEventsAlias();
    exportEvent();
    return true;
}

void LseExport::registerDefaultEventsAlias() {
    // regitser iLiListenAttentively event alias
    for (auto event : ll::event::EventBus::getInstance().events("iListenAttentively")) {
        if (!event.name.starts_with("ila::mc::")) continue;
        constexpr static auto prefixLen = sizeof("ila::mc::") - 1;

        size_t endPos    = event.name.rfind("::");
        auto   eventName = event.name.substr(endPos + 2);
        auto   path      = ll::string_utils::splitByPattern(event.name.substr(prefixLen, endPos - prefixLen), "::");

        size_t length = path.size();

        mEventNameAlias[fmt::format("ila::mc::{0}", eventName)] = event.name;

        for (size_t mask = 1; mask < (1ull << length); ++mask) {
            std::vector<std::string> parts;
            for (size_t i = 0; i < length; ++i) {
                if (mask & (1ull << i)) parts.emplace_back(path[i]);
            }
            std::string alias      = fmt::format("ila::mc::{0}::{1}", fmt::join(parts, "::"), eventName);
            mEventNameAlias[alias] = event.name;
        }
    }

    // register LeviLamina event alias
    for (auto event : ll::event::EventBus::getInstance().events("LeviLamina")) {
        if (!event.name.starts_with("ll::event")) continue;
        mEventNameAlias[fmt::format("ll::event::{0}", event.name.substr(event.name.rfind("::") + 2))] = event.name;
    }
}

bool LseExport::enable() { return true; }

bool LseExport::disable() { return true; }

} // namespace ila

LL_REGISTER_MOD(ila::LseExport, ila::LseExport::getInstance());