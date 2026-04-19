#define LL_MEMORY_OPERATORS
#include "ila-lseexport/LseExport.h"
#include <ll/api/Versions.h>
#include <ll/api/event/EventBus.h>
#include <ll/api/io/FileUtils.h>
#include <ll/api/memory/MemoryOperators.h>
#include <ll/api/mod/ModManagerRegistry.h>
#include <ll/api/mod/RegisterHelper.h>
#include <ll/api/reflection/Deserialization.h>
#include <nlohmann/json.hpp>

namespace ila {

LseExport& LseExport::getInstance() {
    static LseExport instance;
    return instance;
}

bool LseExport::load() {
    // clang-format off
    using ll::data::Version;
    auto remoteVersion = ll::reflection::deserialize_to<ll::mod::Manifest>(
        nlohmann::ordered_json::parse(
            *ll::file_utils::readFile(
                ll::mod::ModManagerRegistry::getInstance().getMod("LegacyRemoteCall")->getModDir() / "manifest.json"
            ),
            nullptr,
            false,
            true
        )
    )->version.value_or({});
    auto levilaminaVersion = ll::getLoaderVersion();
    // clang-format on

    if (remoteVersion < Version{0, 17, 1} && levilaminaVersion < Version{26, 10, 5}) {
        exportEventV1();
        registerDefaultEventsAliasV1();
    } else if (remoteVersion >= Version{0, 17, 1} && levilaminaVersion <= Version{26, 10, 5}) {
        exportEventV2();
        registerDefaultEventsAliasV2();
    } else if (remoteVersion >= Version{0, 17, 1} && levilaminaVersion >= Version{26, 10, 5}) {
        exportEventV3();
        registerDefaultEventsAliasV3();
    } else {
        // 不是，什么情况？？？
        getSelf().getLogger().fatal(
            "Invalid version, remote: {0}, levilamina: {1}",
            remoteVersion.to_string(),
            levilaminaVersion.to_string()
        );
        return false;
    }
    return true;
}

bool LseExport::enable() { return true; }

bool LseExport::disable() { return true; }

} // namespace ila

LL_REGISTER_MOD(ila::LseExport, ila::LseExport::getInstance());