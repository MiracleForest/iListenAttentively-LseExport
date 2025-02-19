#include "LseExport.h"
#include "event/LseEvent.h"
#include <RemoteCallAPI.h>
#include <ll/api/event/Emitter.h>
#include <ll/api/event/EventBus.h>
#include <ll/api/event/EventRefObjSerializer.h>
#include <ll/api/mod/ModManagerRegistry.h>
#include <ll/api/utils/ErrorUtils.h>

#define GET_INSTANCE_MACRO(EXPORT_NAME, CLASS_NAME)                                                                    \
    RemoteCall::exportAs("iListenAttentively", EXPORT_NAME, [](CompoundTag* info) -> CLASS_NAME* {                     \
        if (info && ll::event::isEventSerializedObj(*info) && (*info)["_pointer_"].getId() == Tag::Type::Int64) {      \
            return (CLASS_NAME*)((uintptr_t)((*info)["_pointer_"].get<Int64Tag>()));                                   \
        }                                                                                                              \
        return nullptr;                                                                                                \
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

    GET_INSTANCE_MACRO("getPlayer", Player);
    GET_INSTANCE_MACRO("getEntity", Actor);
    GET_INSTANCE_MACRO("getItem", ItemStack);
    GET_INSTANCE_MACRO("getBlock", Block);
    GET_INSTANCE_MACRO("getBlockEntity", BlockActor);
    GET_INSTANCE_MACRO("getContainer", Container);
}
} // namespace ila