#include "LseExport.h"
#include "event/LseEvent.h"
#include <RemoteCallAPI.h>
#include <ll/api/event/Emitter.h>
#include <ll/api/event/EventBus.h>
#include <ll/api/event/EventRefObjSerializer.h>
#include <ll/api/mod/ModManagerRegistry.h>
#include <ll/api/utils/ErrorUtils.h>

#define GET_INSTANCE_BASE_MACRO(EXPORT_NAME, CLASS_NAME, RETURN, ERROR_RETURN)                                         \
    RemoteCall::exportAs("iListenAttentively", EXPORT_NAME, [](uintptr_t info) -> CLASS_NAME {                         \
        try {                                                                                                          \
            RETURN;                                                                                                    \
        } catch (...) {                                                                                                \
            ll::error_utils::printCurrentException(LseExport::getInstance().getSelf().getLogger());                    \
            ERROR_RETURN;                                                                                              \
        }                                                                                                              \
    });

#define GET_INSTANCE_MACRO(CLASS_NAME)                                                                                 \
    GET_INSTANCE_BASE_MACRO("get" #CLASS_NAME, CLASS_NAME*, return reinterpret_cast<CLASS_NAME*>(info), return nullptr)

#define GET_ADDRESS_MACRO(CLASS_NAME)                                                                                  \
    RemoteCall::exportAs("iListenAttentively", "get" #CLASS_NAME "Address", [](CLASS_NAME* target) -> uintptr_t {      \
        try {                                                                                                          \
            return reinterpret_cast<uintptr_t>(target);                                                                \
        } catch (...) {                                                                                                \
            ll::error_utils::printCurrentException(LseExport::getInstance().getSelf().getLogger());                    \
            return 0;                                                                                                  \
        }                                                                                                              \
    });

namespace ila {

void exportEvent() {
    RemoteCall::exportAs("iListenAttentively", "removeListener", [](ll::event::ListenerId eventId) -> bool {
        return ll::event::EventBus::getInstance().removeListener(eventId);
    });
    RemoteCall::exportAs("iListenAttentively", "hasListener", [](ll::event::ListenerId eventId) -> bool {
        return ll::event::EventBus::getInstance().hasListener(eventId);
    });
    RemoteCall::exportAs(
        "iListenAttentively",
        "getAllEvent",
        []() -> std::vector<std::unordered_map<std::string, std::string>> {
            std::vector<std::unordered_map<std::string, std::string>> result;
            for (auto event : ll::event::EventBus::getInstance().events()) {
                std::unordered_map<std::string, std::string> item;
                item["modName"]   = event.first;
                item["eventName"] = event.second.name;
                result.push_back(item);
            }
            return result;
        }
    );
    RemoteCall::exportAs(
        "iListenAttentively",
        "getAllEventFromModName",
        [](std::string modName) -> std::vector<std::string> {
            std::vector<std::string> result;
            for (auto event : ll::event::EventBus::getInstance().events(modName)) {
                result.push_back(std::string(event.name));
            }
            return result;
        }
    );
    RemoteCall::exportAs("iListenAttentively", "hasEvent", [](std::string eventName) -> bool {
        return ll::event::EventBus::getInstance().hasEvent(ll::event::EventIdView(eventName));
    });
    RemoteCall::exportAs("iListenAttentively", "getListenerCount", [](std::string eventName) -> size_t {
        return ll::event::EventBus::getInstance().getListenerCount(ll::event::EventIdView(eventName));
    });
    RemoteCall::exportAs(
        "iListenAttentively",
        "RegisterEvent",
        [](std::string pluginName, std::string eventName) -> bool {
            if (auto mod = ll::mod::ModManagerRegistry::getInstance().getMod(pluginName)) {
                return ll::event::EventBus::getInstance()
                    .setEventEmitter([](auto&&...) { return nullptr; }, ll::event::EventIdView(eventName), mod);
            }
            return false;
        }
    );
    RemoteCall::exportAs(
        "iListenAttentively",
        "publish",
        [](std::string eventName, CompoundTag* data) -> void {
            if (!data) return;
            auto event = event::LseEvent(data);
            ll::event::EventBus::getInstance().publish(event, ll::event::EventIdView(eventName));
        }
    );
    RemoteCall::exportAs(
        "iListenAttentively",
        "publishToMod",
        [](std::string modName, std::string eventName, CompoundTag* data) -> void {
            if (!data) return;
            auto event = event::LseEvent(data);
            ll::event::EventBus::getInstance().publish(modName, event, ll::event::EventIdView(eventName));
        }
    );
    RemoteCall::exportAs(
        "iListenAttentively",
        "emplaceListener",
        [](std::string const& pluginName, std::string const& eventName, int priority) -> ll::event::ListenerId {
            if(!ll::event::EventBus::getInstance().hasEvent(ll::event::EventIdView(eventName))) return ULLONG_MAX;
            auto                   listenerId = std::make_shared<ll::event::ListenerId>(ULLONG_MAX);
            ll::event::ListenerPtr listener   = ll::event::Listener<ll::event::Event>::create(
                [pluginName, eventName, listenerId](ll::event::Event& event) -> void {
                    try {
                        auto funcName = eventName + "_" + std::to_string(*listenerId);
                        if (!RemoteCall::hasFunc(pluginName, funcName)) {
                            ll::event::EventBus::getInstance().removeListener(*listenerId);
                            return;
                        }
                        std::unique_ptr<CompoundTag> eventData = std::make_unique<CompoundTag>();
                        event.serialize(*eventData);
                        if (auto* result =
                                RemoteCall::importAs<CompoundTag*(std::unique_ptr<CompoundTag>)>(pluginName, funcName)(
                                    std::move(eventData)
                                )) {
                            event.deserialize(*result);
                        }
                    } catch (...) {
                        static auto& logger = LseExport::getInstance().getSelf().getLogger();
                        logger.error("Failed to execute event callback for {} in {} plugin", eventName, pluginName);
                        ll::error_utils::printCurrentException(logger);
                    }
                },
                ll::event::EventPriority(priority)
            );
            if (ll::event::EventBus::getInstance().addListener(listener, ll::event::EventIdView(eventName))) {
                *listenerId = listener->getId();
                return listener->getId();
            }
            listener.reset();
            return ULLONG_MAX;
        }
    );

    GET_INSTANCE_MACRO(Player);
    GET_INSTANCE_MACRO(Actor);
    GET_INSTANCE_MACRO(ItemStack);
    GET_INSTANCE_MACRO(Block);
    GET_INSTANCE_MACRO(BlockActor);
    GET_INSTANCE_MACRO(Container);
    GET_INSTANCE_MACRO(CompoundTag);
    GET_INSTANCE_BASE_MACRO("getNumber", int64_t, return *reinterpret_cast<int64_t*>(info), return 0);
    GET_INSTANCE_BASE_MACRO("getFloat", double, return *reinterpret_cast<double*>(info), return 0);
    GET_INSTANCE_BASE_MACRO("getBoolean", bool, return *reinterpret_cast<bool*>(info), return 0);
    GET_INSTANCE_BASE_MACRO("getString", std::string, return *reinterpret_cast<std::string*>(info), return "");
    GET_INSTANCE_BASE_MACRO("getRawAddress", uintptr_t, return *reinterpret_cast<uintptr_t*>(info), return 0);

    GET_ADDRESS_MACRO(Player);
    GET_ADDRESS_MACRO(Actor);
    GET_ADDRESS_MACRO(ItemStack);
    GET_ADDRESS_MACRO(Block const);
    GET_ADDRESS_MACRO(BlockActor);
    GET_ADDRESS_MACRO(Container);
    GET_ADDRESS_MACRO(CompoundTag);
}
} // namespace ila