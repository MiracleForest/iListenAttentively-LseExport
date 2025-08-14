#define LL_MEMORY_OPERATORS

#include "LseExport.h"
#include "RemoteCallAPI.h"
#include "event/LseEvent.h"
#include <climits>
#include <dyncall/dyncall.h>
#include <ll/api/event/Emitter.h>
#include <ll/api/event/EventBus.h>
#include <ll/api/memory/Memory.h>
#include <ll/api/memory/MemoryOperators.h>
#include <ll/api/mod/ModManagerRegistry.h>
#include <ll/api/service/Bedrock.h>
#include <ll/api/utils/ErrorUtils.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/dimension/Dimension.h>
#include <mc/world/level/dimension/VanillaDimensions.h>
#include <unordered_map>
#include <windows.h>

#define LLEventBus ll::event::EventBus::getInstance()

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
        auto dim = level->getOrCreateDimension(VanillaDimensions::fromSerializedInt(dimensionId));
        if (dim.expired()) return ll::makeStringError("Dimension not found");
        return dim.lock()->mName;
    });
    RemoteCall::exportAs("removeListener", [](ll::event::ListenerId eventId) {
        return LLEventBus.removeListener(eventId);
    });
    RemoteCall::exportAs("hasListener", [](ll::event::ListenerId eventId) { return LLEventBus.hasListener(eventId); });
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
        [&](std::string const& pluginName, std::string const& eventName, int priority) -> ll::Expected<ll::event::ListenerId> {
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
    RemoteCall::exportAs("getLongLong", [&](uintptr_t info) { return *reinterpret_cast<int64*>(info); });
    RemoteCall::exportAs("getUnsignedLongLong", [&](uintptr_t info) { return *reinterpret_cast<uint64*>(info); });
    RemoteCall::exportAs("getInt", [&](uintptr_t info) { return *reinterpret_cast<int32*>(info); });
    RemoteCall::exportAs("getUnsignedInt", [&](uintptr_t info) { return *reinterpret_cast<uint32*>(info); });
    RemoteCall::exportAs("getShort", [&](uintptr_t info) { return *reinterpret_cast<int16*>(info); });
    RemoteCall::exportAs("getUnsignedShort", [&](uintptr_t info) { return *reinterpret_cast<uint16*>(info); });
    RemoteCall::exportAs("getChar", [&](uintptr_t info) { return *reinterpret_cast<int8*>(info); });
    RemoteCall::exportAs("getUnsignedChar", [&](uintptr_t info) { return *reinterpret_cast<uint8*>(info); });
    RemoteCall::exportAs("getFloat", [&](uintptr_t info) { return *reinterpret_cast<float*>(info); });
    RemoteCall::exportAs("getLongDouble", [&](uintptr_t info) {
        return static_cast<double>(*reinterpret_cast<ldouble*>(&info));
    });
    RemoteCall::exportAs("getDouble", [&](uintptr_t info) { return *reinterpret_cast<double*>(&info); });
    RemoteCall::exportAs("getBool", [&](uintptr_t info) { return *reinterpret_cast<bool*>(&info); });
    RemoteCall::exportAs("getString", [&](uintptr_t info) { return *reinterpret_cast<std::string*>(&info); });

    RemoteCall::exportAs("setRawAddress", [&](uintptr_t info, uintptr_t value) {
        ll::memory::modify(reinterpret_cast<void*>(info), sizeof(uintptr_t), [&]() {
            *reinterpret_cast<uintptr_t*>(info) = value;
        });
    });
    RemoteCall::exportAs("setLongLong", [&](uintptr_t info, uint64 value) {
        ll::memory::modify(reinterpret_cast<void*>(info), sizeof(uint64), [&]() {
            *reinterpret_cast<uint64*>(info) = value;
        });
    });
    RemoteCall::exportAs("setUnsignedLongLong", [&](uintptr_t info, uint64 value) {
        ll::memory::modify(reinterpret_cast<void*>(info), sizeof(uint64), [&]() {
            *reinterpret_cast<uint64*>(info) = value;
        });
    });
    RemoteCall::exportAs("setInt", [&](uintptr_t info, int32 value) {
        ll::memory::modify(reinterpret_cast<void*>(info), sizeof(int32), [&]() {
            *reinterpret_cast<int32*>(info) = value;
        });
    });
    RemoteCall::exportAs("setUnsignedInt", [&](uintptr_t info, uint32 value) {
        ll::memory::modify(reinterpret_cast<void*>(info), sizeof(uint32), [&]() {
            *reinterpret_cast<uint32*>(info) = value;
        });
    });
    RemoteCall::exportAs("setShort", [&](uintptr_t info, int16 value) {
        ll::memory::modify(reinterpret_cast<void*>(info), sizeof(int16), [&]() {
            *reinterpret_cast<int16*>(info) = value;
        });
    });
    RemoteCall::exportAs("setUnsignedShort", [&](uintptr_t info, uint16 value) {
        ll::memory::modify(reinterpret_cast<void*>(info), sizeof(uint16), [&]() {
            *reinterpret_cast<uint16*>(info) = value;
        });
    });
    RemoteCall::exportAs("setChar", [&](uintptr_t info, int8 value) {
        ll::memory::modify(reinterpret_cast<void*>(info), sizeof(int8), [&]() {
            *reinterpret_cast<int8*>(info) = value;
        });
    });
    RemoteCall::exportAs("setUnsignedChar", [&](uintptr_t info, uint8 value) {
        ll::memory::modify(reinterpret_cast<void*>(info), sizeof(uint8), [&]() {
            *reinterpret_cast<uint8*>(info) = value;
        });
    });
    RemoteCall::exportAs("setFloat", [&](uintptr_t info, float value) {
        ll::memory::modify(reinterpret_cast<void*>(info), sizeof(float), [&]() {
            *reinterpret_cast<float*>(info) = value;
        });
    });
    RemoteCall::exportAs("setLongDouble", [&](uintptr_t info, double value) {
        ll::memory::modify(reinterpret_cast<void*>(info), sizeof(ldouble), [&]() {
            *reinterpret_cast<ldouble*>(info) = static_cast<ldouble>(value);
        });
    });
    RemoteCall::exportAs("setDouble", [&](uintptr_t info, double value) {
        ll::memory::modify(reinterpret_cast<void*>(info), sizeof(double), [&]() {
            *reinterpret_cast<double*>(info) = value;
        });
    });
    RemoteCall::exportAs("setBool", [&](uintptr_t info, bool value) {
        ll::memory::modify(reinterpret_cast<void*>(info), sizeof(bool), [&]() {
            *reinterpret_cast<bool*>(info) = value;
        });
    });
    RemoteCall::exportAs("setString", [&](uintptr_t info, std::string const& value) {
        ll::memory::modify(reinterpret_cast<void*>(info), sizeof(std::string), [&]() {
            *reinterpret_cast<std::string*>(info) = value;
        });
    });

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
                    ll::memory::Signature::parse(RemoteCall::extract<std::string>(std::move(args[0])))
                        .view()
                        .resolve(
                            ll::sys_utils::getImageRange(RemoteCall::extract<std::string>(std::move(args[1]))),
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
        return ll::memory::getDefaultAllocator().getUsableSize(reinterpret_cast<void*>(address));
    });
    RemoteCall::exportAs("memcpyMemory", [&](uintptr_t dest, uintptr_t src, size_t size) {
        ll::memory::modify(reinterpret_cast<void*>(dest), size, [&]() {
            ll::memory::modify(reinterpret_cast<void*>(src), size, [&]() {
                std::memcpy(reinterpret_cast<void*>(dest), reinterpret_cast<void*>(src), size);
            });
        });
    });
    RemoteCall::exportAs("memsetMemory", [&](uintptr_t dest, int8 value, size_t size) {
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