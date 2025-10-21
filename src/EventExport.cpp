#define LL_MEMORY_OPERATORS

#include "LseExport.h"
#include "RemoteCallAPI.h"
#include "event/LseEvent.h"
#include <dyncall/dyncall.h>
#include <ll/api/Versions.h>
#include <ll/api/event/Emitter.h>
#include <ll/api/memory/Memory.h>
#include <ll/api/memory/MemoryOperators.h>
#include <ll/api/mod/ModManagerRegistry.h>
#include <ll/api/service/Bedrock.h>
#include <ll/api/utils/ErrorUtils.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/dimension/Dimension.h>
#include <mc/world/level/dimension/VanillaDimensions.h>
#include <windows.h>

#define LLEventBus ll::event::EventBus::getInstance()

namespace ll::event {
struct ListenerInfo {
    std::weak_ptr<ListenerBase> weak;
    SmallDenseSet<EventId>      attachedEvents;
};
} // namespace ll::event

namespace ila {

void LseExport::exportEvent() {
    RemoteCall::exportAs("toSnbt", [&](CompoundTag* nbt, uint snbtFormat, uchar indent) -> ll::Expected<std::string> {
        if (!nbt) return ll::makeStringError("NBT is null");
        if (!nbt->contains("value")) return ll::makeStringError("NBT is empty");
        return (*nbt)["value"].toSnbt(static_cast<SnbtFormat>(snbtFormat), indent);
    });
    RemoteCall::exportAs("fromSnbt", [&](std::string const& snbt) {
        return CompoundTagVariant::parse(snbt).transform(
            [](CompoundTagVariant const& nbt) -> std::unique_ptr<CompoundTag> {
                auto result = std::make_unique<CompoundTag>();
                (*result)["value"] = nbt;
                return std::move(result);
            }
        );
    });
    RemoteCall::exportAs("getAllEventAlias", [&]() { return mEventNameAlias; });
    RemoteCall::exportAs("getEventAlias", [&](std::string const& eventName) {
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
        auto dimid = VanillaDimensions::fromSerializedInt(Bedrock::Result<int>{dimensionId});
        if (!dimid) return ll::makeStringError("Dimension id not found");
        if (*dimid == VanillaDimensions::Undefined()) return ll::makeStringError("Dimension id not found");
        // 使用符号获取防止虚表变化
        auto dim = level->$getOrCreateDimension(*dimid);
        if (dim.expired()) return ll::makeStringError("Dimension not found");
        return dim.lock()->mName;
    });
    RemoteCall::exportAs("removeListener", [](std::vector<RemoteCall::ValueType> args) {
        return LLEventBus.removeListener(
            LLEventBus.getListener(RemoteCall::extract<ll::event::ListenerId>(std::move(args[0]))),
            args.size() >= 2 ? ll::event::EventIdView{RemoteCall::extract<std::string>(std::move(args[1]))}
                             : ll::event::EmptyEventId
        );
    });
    RemoteCall::exportAs("hasListener", [](std::vector<RemoteCall::ValueType> args) {
        return LLEventBus.hasListener(
            RemoteCall::extract<ll::event::ListenerId>(std::move(args[0])),
            args.size() >= 2 ? ll::event::EventIdView{RemoteCall::extract<std::string>(std::move(args[1]))}
                             : ll::event::EmptyEventId
        );
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
    RemoteCall::exportAs("hasEvent", [](std::string const& eventName) {
        return LLEventBus.hasEvent(ll::event::EventId(eventName));
    });
    RemoteCall::exportAs("getListenerCount", [](std::string const& eventName) {
        return LLEventBus.getListenerCount(ll::event::EventId(eventName));
    });
    RemoteCall::exportAs(
        "registerEvent",
        [&](std::string const& pluginName, std::string const& eventName) -> ll::Expected<bool> {
            if (eventName.empty()) return ll::makeStringError("Event name cannot be empty");
            if (auto mod = ll::mod::ModManagerRegistry::getInstance().getMod(pluginName); mod) {
                return LLEventBus
                    .setEventEmitter([](auto&&...) { return nullptr; }, ll::event::EventId(eventName), mod);
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
        if (args.size() == 3){
            LLEventBus.publish(
                RemoteCall::extract<std::string>(std::move(args[2])),
                event,
                ll::event::EventId(eventName)
            );
        } else {
            LLEventBus.publish(event, ll::event::EventId(eventName));
        }
        return {};
    });
    RemoteCall::exportAs(
        "emplaceListener",
        [&](std::string const& pluginName, std::string const& eventName, int priority
        ) -> ll::Expected<ll::event::ListenerId> {
            auto listenerId = std::make_shared<ll::event::ListenerId>(ULLONG_MAX);
            auto listener   = ll::event::Listener<ll::event::Event>::create(
                [pluginName, eventName, listenerId, this](ll::event::Event& event) -> void {
                    auto funcName = eventName + "#" + std::to_string(*listenerId);
                    if (!RemoteCall::hasFunc(pluginName, funcName)) {
                        return (void)ll::event::EventBus::getInstance().removeListener(*listenerId);
                    }
                    try {
                        CompoundTag nbt;
                        nbt["eventPtr"] = reinterpret_cast<uintptr_t>(&event);
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

            if (LLEventBus.addListener(listener, ll::event::EventId(eventName))) {
                return *listenerId = listener->getId();
            }
            listener.reset();
            return ll::makeStringError("Failed to add listener");
        }
    );
    RemoteCall::exportAs(
        "getListenerInfo",
        [&](ll::event::ListenerId listenerId) -> ll::Expected<RemoteCall::ValueType> {
            static auto& listenerInfos =
                ll::memory::dAccess<ll::DenseMap<ll::event::ListenerId, ll::event::ListenerInfo>>(
                    ll::memory::dAccess<std::unique_ptr<void*>>(&LLEventBus, 0).get(),
                    2000
                );
            if (auto listenerInfo = listenerInfos.find(listenerId); listenerInfo != listenerInfos.end()) {
                auto listener = listenerInfo->second.weak.lock();
                // clang-format off
                return std::unordered_map<std::string, RemoteCall::ValueType>{
                    {"id",       RemoteCall::NumberType{listener->getId()}                        },
                    {"priority", RemoteCall::NumberType{static_cast<int>(listener->getPriority())}},
                    {
                        "attachedEvents",
                        listenerInfo->second.attachedEvents
                            | std::views::transform([](ll::event::EventId const& eventId) {
                                return RemoteCall::ValueType{eventId.name};
                            })
                            | std::ranges::to<std::vector>()
                    },
                    {
                        "mod",
                        optional_ref{listener->modPtr.lock().get()}
                            .transform([](auto&& manifest) { return manifest.getManifest().name; })
                            .value_or("Unknown")
                    }
                };
                // clang-format on
            } else {
                return ll::makeStringError("Listener not found");
            }
        }
    );

#define EXPORT_AS(NAME, TYPE)                                                                                          \
    RemoteCall::exportAs("get" NAME, [&](uintptr_t info) { return reinterpret_cast<TYPE*>(info); });                   \
    RemoteCall::exportAs("get" NAME "Address", [&](TYPE* info) { return reinterpret_cast<uintptr_t>(info); })

    EXPORT_AS("Player", Player);
    EXPORT_AS("Actor", Actor);
    EXPORT_AS("ItemStack", ItemStack);
    EXPORT_AS("Block", Block const);
    EXPORT_AS("BlockActor", BlockActor);
    EXPORT_AS("Container", Container);
    EXPORT_AS("CompoundTag", CompoundTag);

#undef EXPORT_AS

#define EXPORT_GETTER_SETTER(NAME, TYPE)                                                                               \
    RemoteCall::exportAs("get" NAME, [&](uintptr_t info) {                                                             \
        TYPE result = {};                                                                                              \
        ll::memory::modify(reinterpret_cast<void*>(info), sizeof(TYPE), [&]() {                                        \
            result = *reinterpret_cast<TYPE*>(info);                                                                   \
        });                                                                                                            \
        return result;                                                                                                 \
    });                                                                                                                \
    RemoteCall::exportAs("set" NAME, [&](uintptr_t info, TYPE value) {                                                 \
        ll::memory::modify(reinterpret_cast<void*>(info), sizeof(TYPE), [&]() {                                        \
            *reinterpret_cast<TYPE*>(info) = value;                                                                    \
        });                                                                                                            \
    })

    EXPORT_GETTER_SETTER("RawAddress", uintptr_t);
    EXPORT_GETTER_SETTER("LongLong", int64);
    EXPORT_GETTER_SETTER("UnsignedLongLong", uint64);
    EXPORT_GETTER_SETTER("Int", int32);
    EXPORT_GETTER_SETTER("UnsignedInt", uint32);
    EXPORT_GETTER_SETTER("Short", int16);
    EXPORT_GETTER_SETTER("UnsignedShort", uint16);
    EXPORT_GETTER_SETTER("Char", int8);
    EXPORT_GETTER_SETTER("UnsignedChar", uint8);
    EXPORT_GETTER_SETTER("Float", float);
    EXPORT_GETTER_SETTER("Double", double);
    EXPORT_GETTER_SETTER("LongDouble", ldouble);
    EXPORT_GETTER_SETTER("Bool", bool);
    EXPORT_GETTER_SETTER("String", std::string);

#undef EXPORT_GETTER_SETTER

    RemoteCall::exportAs(
        "getAddressFromSymbol",
        [&](std::vector<RemoteCall::ValueType> args) -> ll::Expected<uintptr_t> {
            switch (args.size()) {
            case 1:
                return reinterpret_cast<uintptr_t>(
                    ll::memory::Symbol(RemoteCall::extract<std::string>(std::move(args[0]))).view().resolve(true)
                );
            case 2:
                return reinterpret_cast<uintptr_t>(GetProcAddress(
                    GetModuleHandle(
                        ll::string_utils::str2wstr(RemoteCall::extract<std::string>(std::move(args[0]))).c_str()
                    ),
                    RemoteCall::extract<std::string>(std::move(args[1])).c_str()
                ));
            default:
                return ll::makeStringError("Too many arguments");
            }
        }
    );
    RemoteCall::exportAs(
        "getAddressFromSignature",
        [&](std::vector<RemoteCall::ValueType> args) -> ll::Expected<uintptr_t> {
            switch (args.size()) {
            case 1:
                return reinterpret_cast<uintptr_t>(
                    ll::memory::Signature::parse(RemoteCall::extract<std::string>(std::move(args[0])))
                        .view()
                        .resolve(true)
                );
            case 2:
                return reinterpret_cast<uintptr_t>(
                    ll::memory::Signature::parse(RemoteCall::extract<std::string>(std::move(args[1])))
                        .view()
                        .resolve(
                            ll::sys_utils::getImageRange(RemoteCall::extract<std::string>(std::move(args[0]))),
                            true
                        )
                );
            default:
                return ll::makeStringError("Too many arguments");
            }
        }
    );
    RemoteCall::exportAs(
        "getImageRange",
        [&](std::string const& moduleName) -> ll::Expected<std::unordered_map<std::string, uintptr_t>> {
            if (auto image = ll::sys_utils::getImageRange(moduleName); !image.empty()) {
                return std::unordered_map<std::string, uintptr_t>{
                    {"start", reinterpret_cast<uintptr_t>(image.begin()._Myptr)},
                    {"end",   reinterpret_cast<uintptr_t>(image.end()._Myptr)  },
                    {"size",  image.size()                                     }
                };
            } else {
                return ll::makeStringError("Module not found");
            }
        }
    );

    RemoteCall::exportAs("mallocMemory", [&](size_t size) {
        return reinterpret_cast<uintptr_t>(ll::memory::getDefaultAllocator().allocate(size));
    });
    RemoteCall::exportAs("freeMemory", [&](uintptr_t address) {
        ll::memory::getDefaultAllocator().release(reinterpret_cast<void*>(address));
    });
    RemoteCall::exportAs("alignedMallocMemory", [&](size_t size, size_t alignment) {
        return reinterpret_cast<uintptr_t>(ll::memory::getDefaultAllocator().alignedAllocate(size, alignment));
    });
    RemoteCall::exportAs("alignedFreeMemory", [&](uintptr_t address) {
        ll::memory::getDefaultAllocator().alignedRelease(reinterpret_cast<void*>(address));
    });
    RemoteCall::exportAs("getUsableMemorySize", [&](uintptr_t address) {
        auto& allocator = ll::memory::getDefaultAllocator();
        if (ll::getGameVersion() >= ll::data::Version{1, 21, 70}) {
            return ll::memory::virtualCall<uint64, void*, bool>(&allocator, 5, reinterpret_cast<void*>(address), false);
        } else {
            return ll::memory::virtualCall<uint64, void*>(&allocator, 5, reinterpret_cast<void*>(address));
        }
    });
    RemoteCall::exportAs("memcpyMemory", [&](uintptr_t dest, uintptr_t src, size_t size) {
        ll::memory::modify(reinterpret_cast<void*>(dest), size, [&]() {
            ll::memory::modify(reinterpret_cast<void*>(src), size, [&]() {
                std::memcpy(reinterpret_cast<void*>(dest), reinterpret_cast<void*>(src), size);
            });
        });
    });
    RemoteCall::exportAs("memsetMemory", [&](uintptr_t dest, int value, size_t size) {
        ll::memory::modify(reinterpret_cast<void*>(dest), size, [&]() {
            std::memset(reinterpret_cast<void*>(dest), value, size);
        });
    });
    RemoteCall::exportAs("memcmpMemory", [&](uintptr_t dest, uintptr_t src, size_t size) {
        ll::memory::modify(reinterpret_cast<void*>(dest), size, [&]() {
            ll::memory::modify(reinterpret_cast<void*>(src), size, [&]() {
                return std::memcmp(reinterpret_cast<void*>(dest), reinterpret_cast<void*>(src), size);
            });
        });
    });
    RemoteCall::exportAs("memmoveMemory", [&](uintptr_t dest, uintptr_t src, size_t size) {
        ll::memory::modify(reinterpret_cast<void*>(dest), size, [&]() {
            ll::memory::modify(reinterpret_cast<void*>(src), size, [&]() {
                std::memmove(reinterpret_cast<void*>(dest), reinterpret_cast<void*>(src), size);
            });
        });
    });

    enum class NativeTypes : uchar {
        Void,
        Bool,
        Char,
        UnsignedChar,
        Short,
        UnsignedShort,
        Int,
        UnsignedInt,
        Long,
        UnsignedLong,
        LongLong,
        UnsignedLongLong,
        Float,
        Double,
        LongDouble,
        Pointer
    };
    RemoteCall::exportAs(
        "dynamicCall",
        [&](std::vector<RemoteCall::ValueType> args) -> ll::Expected<RemoteCall::ValueType> {
            auto* dynamicCallVM = dcNewCallVM(4096);
            dcMode(dynamicCallVM, DC_CALL_C_DEFAULT);
            struct Remover {
                DCCallVM* dynamicCallVM;
                Remover(DCCallVM* dynamicCallVM) : dynamicCallVM(dynamicCallVM) {}
                ~Remover() {
                    if (dynamicCallVM) dcFree(dynamicCallVM);
                }
            } r(dynamicCallVM);
            if (args.size() < 2) return ll::makeStringError("Too few arguments");
            auto address = reinterpret_cast<void*>(RemoteCall::extract<uintptr_t>(std::move(args[0])));
            if (!address) return ll::makeStringError("Invalid address");
            auto resultType = RemoteCall::extract<NativeTypes>(std::move(args[1]));
            if (!magic_enum::enum_contains<NativeTypes>(resultType)) return ll::makeStringError("Invalid result type");
            auto paramsTypes =
                args.size() > 2 ? RemoteCall::extract<std::vector<NativeTypes>>(std::move(args[2])) : std::vector<NativeTypes>{};
            auto params = args.size() > 3 ? RemoteCall::extract<std::vector<RemoteCall::ValueType>>(std::move(args[3]))
                                          : std::vector<RemoteCall::ValueType>{};
            if (params.size() != paramsTypes.size()) return ll::makeStringError("Wrong number of parameters");
            dcReset(dynamicCallVM);

            for (size_t i = 0; i < params.size(); ++i) {
                switch (paramsTypes[i]) {
                case NativeTypes::Void:
                    return ll::makeStringError("Parameter types are not allowed to be void");
                case NativeTypes::Bool:
                    dcArgBool(dynamicCallVM, RemoteCall::extract<bool>(std::move(params[i])));
                    break;
                case NativeTypes::Char:
                    dcArgChar(dynamicCallVM, RemoteCall::extract<char>(std::move(params[i])));
                    break;
                case NativeTypes::UnsignedChar:
                    dcArgChar(dynamicCallVM, std::bit_cast<char>(RemoteCall::extract<uchar>(std::move(params[i]))));
                    break;
                case NativeTypes::Short:
                    dcArgShort(dynamicCallVM, RemoteCall::extract<short>(std::move(params[i])));
                    break;
                case NativeTypes::UnsignedShort:
                    dcArgShort(dynamicCallVM, std::bit_cast<short>(RemoteCall::extract<ushort>(std::move(params[i]))));
                    break;
                case NativeTypes::Int:
                    dcArgInt(dynamicCallVM, RemoteCall::extract<int>(std::move(params[i])));
                    break;
                case NativeTypes::UnsignedInt:
                    dcArgInt(dynamicCallVM, std::bit_cast<int>(RemoteCall::extract<uint>(std::move(params[i]))));
                    break;
                case NativeTypes::Long:
                    dcArgLong(dynamicCallVM, RemoteCall::extract<long>(std::move(params[i])));
                    break;
                case NativeTypes::UnsignedLong:
                    dcArgLong(dynamicCallVM, std::bit_cast<long>(RemoteCall::extract<ulong>(std::move(params[i]))));
                    break;
                case NativeTypes::LongLong:
                    dcArgLongLong(dynamicCallVM, RemoteCall::extract<llong>(std::move(params[i])));
                    break;
                case NativeTypes::UnsignedLongLong:
                    dcArgLongLong(
                        dynamicCallVM,
                        std::bit_cast<llong>(RemoteCall::extract<ullong>(std::move(params[i])))
                    );
                    break;
                case NativeTypes::Float:
                    dcArgFloat(dynamicCallVM, RemoteCall::extract<float>(std::move(params[i])));
                    break;
                case NativeTypes::Double:
                    dcArgDouble(dynamicCallVM, RemoteCall::extract<double>(std::move(params[i])));
                    break;
                case NativeTypes::LongDouble:
                    dcArgDouble(
                        dynamicCallVM,
                        std::bit_cast<double>(RemoteCall::extract<ldouble>(std::move(params[i])))
                    );
                    break;
                case NativeTypes::Pointer: {
                    auto addr = RemoteCall::extract<uintptr_t>(std::move(params[i]));
                    dcArgPointer(dynamicCallVM, reinterpret_cast<void*>(addr));
                    break;
                }
                default:
                    return ll::makeStringError("Unknown parameter type");
                }
            }
            switch (resultType) {
            case NativeTypes::Void:
                dcCallVoid(dynamicCallVM, address);
                return {std::nullptr_t{}};
            case NativeTypes::Bool:
                return {dcCallBool(dynamicCallVM, address)};
            case NativeTypes::Char:
                return {dcCallChar(dynamicCallVM, address)};
            case NativeTypes::UnsignedChar:
                return {std::bit_cast<uchar>(dcCallChar(dynamicCallVM, address))};
            case NativeTypes::Short:
                return {dcCallShort(dynamicCallVM, address)};
            case NativeTypes::UnsignedShort:
                return {std::bit_cast<ushort>(dcCallShort(dynamicCallVM, address))};
            case NativeTypes::Int:
                return {dcCallInt(dynamicCallVM, address)};
            case NativeTypes::UnsignedInt:
                return {std::bit_cast<uint>(dcCallInt(dynamicCallVM, address))};
            case NativeTypes::Long:
                return {dcCallLong(dynamicCallVM, address)};
            case NativeTypes::UnsignedLong:
                return {std::bit_cast<ulong>(dcCallLong(dynamicCallVM, address))};
            case NativeTypes::LongLong:
                return {dcCallLongLong(dynamicCallVM, address)};
            case NativeTypes::UnsignedLongLong:
                return {std::bit_cast<ullong>(dcCallLongLong(dynamicCallVM, address))};
            case NativeTypes::Float:
                return {dcCallFloat(dynamicCallVM, address)};
            case NativeTypes::Double:
                return {dcCallDouble(dynamicCallVM, address)};
            case NativeTypes::LongDouble:
                return {std::bit_cast<ldouble>(dcCallDouble(dynamicCallVM, address))};
            case NativeTypes::Pointer:
                return {reinterpret_cast<uintptr_t>(dcCallPointer(dynamicCallVM, address))};
            default:
                return ll::makeStringError("Unknown result type");
            }
        }
    );
}

} // namespace ila