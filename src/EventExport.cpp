#include "LseExport.h"
#include "RemoteCallAPI.h"
#include "event/LseEvent.h"
#include <ll/api/event/Emitter.h>
#include <ll/api/event/EventBus.h>
#include <ll/api/mod/ModManagerRegistry.h>
#include <ll/api/service/Bedrock.h>
#include <ll/api/utils/ErrorUtils.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/dimension/Dimension.h>
#include <mc/world/level/dimension/VanillaDimensions.h>

#define LLEventBus ll::event::EventBus::getInstance()

namespace ila {

void LseExport::exportEvent() {
    RemoteCall::exportAs("getAllEventAlias", [&]() { return mEventNameAlias; });
    RemoteCall::exportAs("getEventAlias", [&](std::string const& eventName) -> std::vector<std::string> {
        return mEventNameAlias | std::views::filter([&](auto& pair) { return pair.second == eventName; })
             | std::views::keys | std::ranges::to<std::vector>();
    });
    RemoteCall::exportAs("getEventName", [&](std::string const& eventAlias) -> ll::Expected<std::string> {
        if (auto it = mEventNameAlias.find(eventAlias); it != mEventNameAlias.end()) {
            return it->second;
        } else {
            return ll::makeStringError("Event alias not found");
        }
    });
    RemoteCall::exportAs("getDimensionIdFromName", [](std::string const& dimensionName) -> ll::Expected<int> {
        auto id = VanillaDimensions::fromString(dimensionName).id;
        if (id == VanillaDimensions::Undefined().id) return ll::makeStringError("Dimension name not found");
        return id;
    });
    RemoteCall::exportAs("getDimensionNameFromId", [](int dimensionId) -> ll::Expected<std::string> {
        auto level = ll::service::getLevel();
        if (!level) return ll::makeStringError("Unable to obtain the Level");
        auto dim = level->getOrCreateDimension(VanillaDimensions::fromSerializedInt(dimensionId));
        if (dim.expired()) return ll::makeStringError("Dimension not found");
        return dim.lock()->mName;
    });
    RemoteCall::exportAs("removeListener", [](ll::event::ListenerId eventId) -> bool {
        return LLEventBus.removeListener(eventId);
    });
    RemoteCall::exportAs("hasListener", [](ll::event::ListenerId eventId) -> bool {
        return LLEventBus.hasListener(eventId);
    });
    RemoteCall::exportAs(
        "getAllEvent",
        [](std::vector<RemoteCall::ValueType> args) -> ll::Expected<RemoteCall::ValueType> {
            switch (args.size()) {
            case 0: {
                std::vector<std::unordered_map<std::string, std::string>> result;
                for (auto event : LLEventBus.events()) {
                    std::unordered_map<std::string, std::string> item;
                    item["modName"]   = event.first;
                    item["eventName"] = event.second.name;
                    result.emplace_back(std::move(item));
                }
                return RemoteCall::pack(result);
            }
            case 1: {
                auto                     modName = RemoteCall::extract<std::string>(std::move(args[0]));
                std::vector<std::string> result;
                for (auto event : LLEventBus.events(modName)) {
                    result.push_back(std::string(event.name));
                }
                return RemoteCall::pack(result);
            }
            default:
                return ll::makeStringError("Too many arguments");
            }
        }
    );
    RemoteCall::exportAs("hasEvent", [](std::string const& eventName) -> bool {
        return LLEventBus.hasEvent(ll::event::EventIdView(eventName));
    });
    RemoteCall::exportAs("getListenerCount", [](std::string const& eventName) -> size_t {
        return LLEventBus.getListenerCount(ll::event::EventIdView(eventName));
    });
    RemoteCall::exportAs(
        "RegisterEvent",
        [&](std::string const& pluginName, std::string const& eventName) -> ll::Expected<bool> {
            if (eventName.empty()) return ll::makeStringError("Event name cannot be empty");
            if (auto mod = ll::mod::ModManagerRegistry::getInstance().getMod(pluginName)) {
                return LLEventBus
                    .setEventEmitter([](auto&&...) { return nullptr; }, ll::event::EventIdView(eventName), mod);
            } else {
                return ll::makeStringError(fmt::format("The {0} Mod cannot be obtained", pluginName));
            }
        }
    );
    RemoteCall::exportAs("publish", [&](std::vector<RemoteCall::ValueType> args) -> ll::Expected<void> {
        if (args.size() > 3) return ll::makeStringError("Too many arguments");
        auto eventName = RemoteCall::extract<std::string>(std::move(args[0]));
        auto data      = RemoteCall::extract<CompoundTag*>(std::move(args[1]));
        if (!data) return ll::makeStringError("Data is null");
        auto event = event::LseEvent(eventName, data);
        if (args.size() == 3)
            LLEventBus.publish(
                RemoteCall::extract<std::string>(std::move(args[2])),
                event,
                ll::event::EventIdView(eventName)
            );
        else LLEventBus.publish(event, ll::event::EventIdView(eventName));
        return {};
    });
    RemoteCall::exportAs(
        "emplaceListener",
        [&](std::string const& pluginName, std::string eventName, int priority) -> ll::Expected<ll::event::ListenerId> {
            auto listenerId = std::make_shared<ll::event::ListenerId>(ULLONG_MAX);
            auto listener   = ll::event::Listener<ll::event::Event>::create(
                [pluginName, eventName, listenerId, this](ll::event::Event& event) -> void {
                    auto funcName = eventName + "#" + std::to_string(*listenerId);
                    if (!RemoteCall::hasFunc(pluginName, funcName)) {
                        return (void)ll::event::EventBus::getInstance().removeListener(*listenerId);
                    }
                    try {
                        CompoundTag nbt;
                        event.serialize(nbt);
                        // clang-format off
                        event.deserialize(*RemoteCall::importAs<CompoundTag*(CompoundTag*)>(pluginName,
                        funcName)(&nbt));
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

            if (LLEventBus.addListener(listener, ll::event::EventIdView(eventName))) {
                return *listenerId = listener->getId();
            }
            listener.reset();
            return ll::makeStringError("Failed to add listener");
        }
    );

    RemoteCall::exportAs("getPlayer", [&](uintptr_t info) { return reinterpret_cast<Player*>(info); });
    RemoteCall::exportAs("getActor", [&](uintptr_t info) { return reinterpret_cast<Actor*>(info); });
    RemoteCall::exportAs("getItemStack", [&](uintptr_t info) { return reinterpret_cast<ItemStack*>(info); });
    RemoteCall::exportAs("getBlock", [&](uintptr_t info) { return reinterpret_cast<Block const*>(info); });
    RemoteCall::exportAs("getBlockActor", [&](uintptr_t info) { return reinterpret_cast<BlockActor*>(info); });
    RemoteCall::exportAs("getContainer", [&](uintptr_t info) { return reinterpret_cast<Container*>(info); });
    RemoteCall::exportAs("getCompoundTag", [&](uintptr_t info) { return reinterpret_cast<CompoundTag*>(info); });

    RemoteCall::exportAs("getPlayerAddress", [&](Player* info) { return reinterpret_cast<uintptr_t>(info); });
    RemoteCall::exportAs("getActorAddress", [&](Actor* info) { return reinterpret_cast<uintptr_t>(info); });
    RemoteCall::exportAs("getItemStackAddress", [&](ItemStack* info) { return reinterpret_cast<uintptr_t>(info); });
    RemoteCall::exportAs("getBlockAddress", [&](Block const* info) { return reinterpret_cast<uintptr_t>(info); });
    RemoteCall::exportAs("getBlockActorAddress", [&](BlockActor* info) { return reinterpret_cast<uintptr_t>(info); });
    RemoteCall::exportAs("getContainerAddress", [&](Container* info) { return reinterpret_cast<uintptr_t>(info); });
    RemoteCall::exportAs("getCompoundTagAddress", [&](CompoundTag* info) { return reinterpret_cast<uintptr_t>(info); });

    RemoteCall::exportAs("getRawAddress", [&](uintptr_t info) { return *reinterpret_cast<uintptr_t*>(info); });
    RemoteCall::exportAs("getLongLong", [&](uintptr_t info) { return *reinterpret_cast<int64*>(&info); });
    RemoteCall::exportAs("getUnsignedLongLong", [&](uintptr_t info) { return *reinterpret_cast<uint64*>(&info); });
    RemoteCall::exportAs("getInt", [&](uintptr_t info) { return *reinterpret_cast<int32*>(&info); });
    RemoteCall::exportAs("getUnsignedInt", [&](uintptr_t info) { return *reinterpret_cast<uint32*>(&info); });
    RemoteCall::exportAs("getShort", [&](uintptr_t info) { return *reinterpret_cast<int16*>(&info); });
    RemoteCall::exportAs("getUnsignedShort", [&](uintptr_t info) { return *reinterpret_cast<uint16*>(&info); });
    RemoteCall::exportAs("getChar", [&](uintptr_t info) { return *reinterpret_cast<int8*>(&info); });
    RemoteCall::exportAs("getUnsignedChar", [&](uintptr_t info) { return *reinterpret_cast<uint8*>(&info); });
    RemoteCall::exportAs("getFloat", [&](uintptr_t info) { return *reinterpret_cast<float*>(&info); });
    RemoteCall::exportAs("getLongDouble", [&](uintptr_t info) {
        return static_cast<double>(*reinterpret_cast<ldouble*>(&info));
    });
    RemoteCall::exportAs("getDouble", [&](uintptr_t info) { return *reinterpret_cast<double*>(&info); });
    RemoteCall::exportAs("getBool", [&](uintptr_t info) { return *reinterpret_cast<bool*>(&info); });
    RemoteCall::exportAs("getString", [&](uintptr_t info) { return *reinterpret_cast<std::string*>(&info); });
}

} // namespace ila