#include "LseExport.h"
#include "event/LseEvent.h"
#include <RemoteCallAPI.h>
#include <ll/api/event/Emitter.h>
#include <ll/api/event/EventBus.h>
#include <ll/api/event/EventRefObjSerializer.h>
#include <ll/api/mod/ModManagerRegistry.h>
#include <ll/api/service/Bedrock.h>
#include <ll/api/utils/ErrorUtils.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/dimension/Dimension.h>
#include <mc/world/level/dimension/VanillaDimensions.h>
#include <unordered_map>

#define EXPORT_NAMESPACE "iListenAttentively"

#define EXPORT_FUNCTION_MACRO(EXPORT_NAME, CLASS_NAME, RETURN, ERROR_RETURN, ...)                                      \
    RemoteCall::exportAs(EXPORT_NAMESPACE, EXPORT_NAME, [&](__VA_ARGS__) -> CLASS_NAME {                               \
        try {                                                                                                          \
            RETURN;                                                                                                    \
        } catch (...) {                                                                                                \
            getSelf().getLogger().error("Failed to call " EXPORT_NAMESPACE "::" EXPORT_NAME);                          \
            ll::error_utils::printCurrentException(getSelf().getLogger());                                             \
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

#define LLEventBus ll::event::EventBus::getInstance()

namespace ila {

void LseExport::exportEvent() {
    RemoteCall::exportAs(EXPORT_NAMESPACE, "getAllEventAlias", [&]() -> std::unordered_map<std::string, std::string> {
        return mEventNameAlias;
    });
    RemoteCall::exportAs(
        EXPORT_NAMESPACE,
        "getEventAlias",
        [&](std::string const& eventName) -> std::vector<std::string> {
            std::vector<std::string> result;
            for (auto& [eventAlias, modEventName] : mEventNameAlias) {
                if (modEventName == eventName) result.push_back(eventAlias);
            }
            return result;
        }
    );
    RemoteCall::exportAs(EXPORT_NAMESPACE, "getEventName", [&](std::string const& eventAlias) -> std::string {
        return mEventNameAlias.contains(eventAlias) ? mEventNameAlias[eventAlias] : "";
    });
    RemoteCall::exportAs(EXPORT_NAMESPACE, "getDimensionIdFromName", [](std::string const& dimensionName) -> int {
        auto id = VanillaDimensions::fromString(dimensionName).id;
        return id == VanillaDimensions::Undefined().id ? -1 : id;
    });
    RemoteCall::exportAs(EXPORT_NAMESPACE, "getDimensionNameFromId", [](int dimensionId) -> std::string {
        return ll::service::getLevel()
            .transform([&](Level& level) -> std::string {
                auto dim = level.getOrCreateDimension(VanillaDimensions::fromSerializedInt(dimensionId));
                return dim.expired() ? "" : dim.lock()->mName;
            })
            .value_or("");
    });
    RemoteCall::exportAs(EXPORT_NAMESPACE, "removeListener", [](ll::event::ListenerId eventId) -> bool {
        return LLEventBus.removeListener(eventId);
    });
    RemoteCall::exportAs(EXPORT_NAMESPACE, "hasListener", [](ll::event::ListenerId eventId) -> bool {
        return LLEventBus.hasListener(eventId);
    });
    RemoteCall::exportAs(
        EXPORT_NAMESPACE,
        "getAllEvent",
        []() -> std::vector<std::unordered_map<std::string, std::string>> {
            std::vector<std::unordered_map<std::string, std::string>> result;
            for (auto event : LLEventBus.events()) {
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
            for (auto event : LLEventBus.events(modName)) {
                result.push_back(std::string(event.name));
            }
            return result;
        }
    );
    RemoteCall::exportAs(EXPORT_NAMESPACE, "hasEvent", [](std::string eventName) -> bool {
        return LLEventBus.hasEvent(ll::event::EventIdView(eventName));
    });
    RemoteCall::exportAs(EXPORT_NAMESPACE, "getListenerCount", [](std::string eventName) -> size_t {
        return LLEventBus.getListenerCount(ll::event::EventIdView(eventName));
    });
    RemoteCall::exportAs(EXPORT_NAMESPACE, "RegisterEvent", [&](std::string pluginName, std::string eventName) -> bool {
        if (auto mod = ll::mod::ModManagerRegistry::getInstance().getMod(pluginName)) {
            getSelf().getLogger().debug("Register event {0} in {1} plugin", eventName, pluginName);
            return LLEventBus
                .setEventEmitter([](auto&&...) { return nullptr; }, ll::event::EventIdView(eventName), mod);
        }
        return false;
    });
    RemoteCall::exportAs(EXPORT_NAMESPACE, "publish", [&](std::string eventName, CompoundTag* data) -> void {
        if (!data) return;
        auto event = event::LseEvent(data);
        getSelf().getLogger().debug("Publish event {0}, data: {1}", eventName, data->toSnbt());
        LLEventBus.publish(event, ll::event::EventIdView(eventName));
    });
    RemoteCall::exportAs(
        EXPORT_NAMESPACE,
        "publishToMod",
        [&](std::string modName, std::string eventName, CompoundTag* data) -> void {
            if (!data) return;
            auto event = event::LseEvent(data);
            getSelf()
                .getLogger()
                .debug("Publish event {0} to {1} plugin, data: {1}", eventName, modName, data->toSnbt());
            LLEventBus.publish(modName, event, ll::event::EventIdView(eventName));
        }
    );
    RemoteCall::exportAs(
        EXPORT_NAMESPACE,
        "emplaceListener",
        [&](std::string const& pluginName, std::string eventName, int priority) -> ll::event::ListenerId {
            // 输出尝试注册提示
            // clang-format off
            static std::unordered_map<int, std::string> mPriorityNameMap = {
                {static_cast<int>(ll::event::EventPriority::Lowest), "Lowest"},
                {static_cast<int>(ll::event::EventPriority::Low), "Low"},
                {static_cast<int>(ll::event::EventPriority::Normal), "Normal"},
                {static_cast<int>(ll::event::EventPriority::High), "High"},
                {static_cast<int>(ll::event::EventPriority::Highest), "Highest"}
            };
            // clang-format on
            auto priorityName =
                mPriorityNameMap.contains(priority) ? mPriorityNameMap[priority] : std::to_string(priority);
            getSelf().getLogger().debug(
                "Try register {0} event listener in {1} plugin, priority: {2}",
                eventName,
                pluginName,
                priorityName
            );

            // 创建监听器
            auto listenerId = std::make_shared<ll::event::ListenerId>(ULLONG_MAX);
            auto listener   = ll::event::Listener<ll::event::Event>::create(
                [pluginName, eventName, listenerId, this](ll::event::Event& event) -> void {
                    auto funcName = eventName + "#" + std::to_string(*listenerId);
                    if (!RemoteCall::hasFunc(pluginName, funcName)) {
                        ll::event::EventBus::getInstance().removeListener(*listenerId);
                        return getSelf().getLogger().debug(
                            "Remove event listener for {0} in {1} plugin, id: {2}, reason: function not found",
                            eventName,
                            pluginName,
                            *listenerId
                        );
                    }
                    try {
                        CompoundTag nbt;
                        event.serialize(nbt);
                        // clang-format off
                        event.deserialize(*RemoteCall::importAs<CompoundTag*(CompoundTag*)>(pluginName, funcName)(&nbt));
                        // clang-format on
                    } catch (...) {
                        getSelf().getLogger().error(
                            "Failed to execute event callback for {0} in {1} plugin",
                            eventName,
                            pluginName
                        );
                        ll::error_utils::printCurrentException(getSelf().getLogger());
                    }
                },
                static_cast<ll::event::EventPriority>(priority)
            );

            // 注册监听器
            if (LLEventBus.addListener(listener, ll::event::EventIdView(eventName))) {
                getSelf().getLogger().debug(
                    "Register event listener for {0} in {1} plugin, id: {2}, priority: {3}",
                    eventName,
                    pluginName,
                    listener->getId(),
                    priorityName
                );
                return *listenerId = listener->getId();
            }
            listener.reset();
            return ULLONG_MAX;
        }
    );

    // clang-format off
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

    EXPORT_FUNCTION_MACRO("getBoolean", bool, return *reinterpret_cast<bool*>(info), return false, uintptr_t info);
    EXPORT_FUNCTION_MACRO("getString", std::string, return *reinterpret_cast<std::string*>(info), return "", uintptr_t info);
    EXPORT_FUNCTION_MACRO("getRawAddress", uintptr_t, return *reinterpret_cast<uintptr_t*>(info), return 0, uintptr_t info);

    GET_ADDRESS_MACRO(Player);
    GET_ADDRESS_MACRO(Actor);
    GET_ADDRESS_MACRO(ItemStack);
    GET_ADDRESS_BASE_MACRO("Block", Block const);
    GET_ADDRESS_MACRO(BlockActor);
    GET_ADDRESS_MACRO(Container);
    GET_ADDRESS_MACRO(CompoundTag);
    // clang-format on
}
} // namespace ila