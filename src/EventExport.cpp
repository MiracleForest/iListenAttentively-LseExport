#include "LseExport.h"
#include "event/LseEvent.h"
#include <RemoteCallAPI.h>
#include <ll/api/event/Emitter.h>
#include <ll/api/event/EventBus.h>
#include <ll/api/event/EventRefObjSerializer.h>
#include <ll/api/mod/ModManagerRegistry.h>
#include <ll/api/utils/ErrorUtils.h>

#define EXPORT_NAMESPACE "iListenAttentively"

#define EXPORT_FUNCTION_MACRO(EXPORT_NAME, CLASS_NAME, RETURN, ERROR_RETURN, ...)                                      \
    RemoteCall::exportAs(EXPORT_NAMESPACE, EXPORT_NAME, [](__VA_ARGS__) -> CLASS_NAME {                                \
        try {                                                                                                          \
            RETURN;                                                                                                    \
        } catch (...) {                                                                                                \
            LseExport::getInstance().getSelf().getLogger().error("Failed to call " EXPORT_NAMESPACE "::" EXPORT_NAME); \
            ll::error_utils::printCurrentException(LseExport::getInstance().getSelf().getLogger());                    \
            ERROR_RETURN;                                                                                              \
        }                                                                                                              \
    });

#define GET_INSTANCE_MACRO(CLASS_NAME)                                                                                 \
    EXPORT_FUNCTION_MACRO(                                                                                             \
        "get" #CLASS_NAME,                                                                                             \
        CLASS_NAME*,                                                                                                   \
        return reinterpret_cast<CLASS_NAME*>(info),                                                                    \
        return nullptr,                                                                                                \
        uintptr_t info                                                                                                 \
    )

#define GET_NUMBER_MACRO(TYPE_NAME, TYPE)                                                                              \
    EXPORT_FUNCTION_MACRO(                                                                                             \
        "get" TYPE_NAME,                                                                                               \
        int64,                                                                                                         \
        return static_cast<int64>(*reinterpret_cast<TYPE*>(info)),                                                     \
        return 0ull,                                                                                                   \
        uintptr_t info                                                                                                 \
    )

#define GET_FLOAT_MACRO(TYPE_NAME, TYPE)                                                                               \
    EXPORT_FUNCTION_MACRO(                                                                                             \
        "get" TYPE_NAME,                                                                                               \
        double,                                                                                                        \
        return static_cast<double>(*reinterpret_cast<TYPE*>(info)),                                                    \
        return 0ull,                                                                                                   \
        uintptr_t info                                                                                                 \
    )

#define GET_ADDRESS_BASE_MACRO(CLASS_NAME, TYPE)                                                                       \
    EXPORT_FUNCTION_MACRO(                                                                                             \
        "get" CLASS_NAME "Address",                                                                                    \
        uintptr_t,                                                                                                     \
        return reinterpret_cast<uintptr_t>(info),                                                                      \
        return 0ull,                                                                                                   \
        TYPE * info                                                                                                    \
    )

#define GET_ADDRESS_MACRO(TYPE) GET_ADDRESS_BASE_MACRO(#TYPE, TYPE)

namespace ila {

extern std::unordered_map<std::string, std::string> mEventNameAlias;

void exportEvent() {
    RemoteCall::exportAs(EXPORT_NAMESPACE, "removeListener", [](ll::event::ListenerId eventId) -> bool {
        return ll::event::EventBus::getInstance().removeListener(eventId);
    });
    RemoteCall::exportAs(EXPORT_NAMESPACE, "hasListener", [](ll::event::ListenerId eventId) -> bool {
        return ll::event::EventBus::getInstance().hasListener(eventId);
    });
    RemoteCall::exportAs(
        EXPORT_NAMESPACE,
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
        EXPORT_NAMESPACE,
        "getAllEventFromModName",
        [](std::string modName) -> std::vector<std::string> {
            std::vector<std::string> result;
            for (auto event : ll::event::EventBus::getInstance().events(modName)) {
                result.push_back(std::string(event.name));
            }
            return result;
        }
    );
    RemoteCall::exportAs(EXPORT_NAMESPACE, "hasEvent", [](std::string eventName) -> bool {
        return ll::event::EventBus::getInstance().hasEvent(ll::event::EventIdView(eventName));
    });
    RemoteCall::exportAs(EXPORT_NAMESPACE, "getListenerCount", [](std::string eventName) -> size_t {
        return ll::event::EventBus::getInstance().getListenerCount(ll::event::EventIdView(eventName));
    });
    RemoteCall::exportAs(EXPORT_NAMESPACE, "RegisterEvent", [](std::string pluginName, std::string eventName) -> bool {
        if (auto mod = ll::mod::ModManagerRegistry::getInstance().getMod(pluginName)) {
            return ll::event::EventBus::getInstance()
                .setEventEmitter([](auto&&...) { return nullptr; }, ll::event::EventIdView(eventName), mod);
        }
        return false;
    });
    RemoteCall::exportAs(EXPORT_NAMESPACE, "publish", [](std::string eventName, CompoundTag* data) -> void {
        if (!data) return;
        auto event = event::LseEvent(data);
        ll::event::EventBus::getInstance().publish(event, ll::event::EventIdView(eventName));
    });
    RemoteCall::exportAs(
        EXPORT_NAMESPACE,
        "publishToMod",
        [](std::string modName, std::string eventName, CompoundTag* data) -> void {
            if (!data) return;
            auto event = event::LseEvent(data);
            ll::event::EventBus::getInstance().publish(modName, event, ll::event::EventIdView(eventName));
        }
    );
    RemoteCall::exportAs(
        EXPORT_NAMESPACE,
        "emplaceListener",
        [](std::string const& pluginName, std::string eventName, int priority) -> ll::event::ListenerId {
            // clang-format off
            if (
                !ll::event::EventBus::getInstance().hasEvent(ll::event::EventIdView(eventName))
                && !mEventNameAlias.contains(eventName)
                && !ll::event::EventBus::getInstance().hasEvent(ll::event::EventIdView(mEventNameAlias[eventName]))
            ) return ULLONG_MAX;
            // clang-format on
            if (!ll::event::EventBus::getInstance().hasEvent(ll::event::EventIdView(eventName))) {
                eventName = mEventNameAlias[eventName];
            }
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

    GET_NUMBER_MACRO("LongLong", int64);
    GET_NUMBER_MACRO("UnsignedLongLong", uint64);
    GET_NUMBER_MACRO("Int", int32);
    GET_NUMBER_MACRO("UnsignedInt", uint32);
    GET_NUMBER_MACRO("Short", int16);
    GET_NUMBER_MACRO("UnsignedShort", uint16);
    GET_NUMBER_MACRO("Char", int8);
    GET_NUMBER_MACRO("UnsignedChar", uint8);

    GET_NUMBER_MACRO("Float", float);
    GET_NUMBER_MACRO("Double", double);
    GET_NUMBER_MACRO("LongDouble", ldouble);

    EXPORT_FUNCTION_MACRO("getBoolean", bool, return *reinterpret_cast<bool*>(info), return false, bool info);
    EXPORT_FUNCTION_MACRO("getString", std::string, return *reinterpret_cast<std::string*>(info), return "", bool info);
    EXPORT_FUNCTION_MACRO("getRawAddress", uintptr_t, return *reinterpret_cast<uintptr_t*>(info), return 0, bool info);

    GET_ADDRESS_MACRO(Player);
    GET_ADDRESS_MACRO(Actor);
    GET_ADDRESS_MACRO(ItemStack);
    GET_ADDRESS_BASE_MACRO("Block", Block const);
    GET_ADDRESS_MACRO(BlockActor);
    GET_ADDRESS_MACRO(Container);
    GET_ADDRESS_MACRO(CompoundTag);
}
} // namespace ila