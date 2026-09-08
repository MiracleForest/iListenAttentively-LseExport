#include "adapt/Internal.h"
#include "adapt/utils/CompilerPredefine.h"
#include "adapt/utils/DynamicLibrary.h"
#include "adapt/utils/MemoryUtils.h"
#include "adapt/utils/StringUtils.h"
#include "adapt/utils/TypeName.h"
#include "fake_levilamina/LeviLamina.h"
#include <Windows.h>
#include <demangler/Demangle.h>
#include <pl/Hook.h>
#include <pl/dependency/DependencyWalker.h>
#include <ranges>
#include <psapi.h>
#include <winscard.h>

namespace mif::ila_lseexport {

static DynamicLibrary library;

decltype(internalBinaries)::const_iterator
findBestBinary(decltype(internalBinaries) const& binaries, ll::data::Version const& loader) noexcept {
    if (binaries.empty()) return binaries.end();
    auto upper = binaries.upper_bound(loader);
    if (upper == binaries.begin()) return binaries.end();
    return std::prev(upper);
}

template <typename T>
void diagnosticDependency(T& self, pl::dependency_walker::DependencyIssueItem const& item, size_t depth = 0) {
    std::string indent(depth * 3 + 3, ' ');
    if (item.mContainsError) {
        self.fatal("{0}module: {1}", indent, string_utils::u8str2str(item.mPath.filename().u8string()));
        if (!item.mMissingModule.empty()) {
            self.fatal("{0}missing module:", indent);
            for (auto const& missingModule : item.mMissingModule) {
                self.fatal("{0}|- {1}", indent, missingModule);
            }
        }
        if (!item.mMissingProcedure.empty()) {
            self.fatal("{0}missing content:", indent);
            for (auto const& [module, missingProcedure] : item.mMissingProcedure) {
                self.fatal("{0}|- {1}", indent, module);
                for (auto const& procedure : missingProcedure) {
                    std::string res;
                    if (!demangler::nonMicrosoftDemangle(procedure, res)
                        && procedure.starts_with("_")) { // some platform's external sym style...
                        demangler::nonMicrosoftDemangle(procedure.substr(1), res);
                    } else {
                        if (char* demangled = demangler::microsoftDemangle(
                                procedure,
                                nullptr,
                                nullptr,
                                static_cast<demangler::MSDemangleFlags>(
                                    demangler::MSDF_NoAccessSpecifier | demangler::MSDF_NoCallingConvention
                                )
                            )) {
                            res = demangled;
                            std::free(demangled);
                        } else {
                            res = procedure;
                        }
                    }
                    self.fatal("{0}|---- {1}", indent, res);
                }
            }
        }
        if (!item.mDependencies.empty()) {
            for (auto const& [_, subItem] : item.mDependencies) {
                diagnosticDependency(self, *subItem, depth + 1);
            }
        }
    }
}

template <typename T>
std::shared_ptr<T> (*originCurrentFunc)(void*);

template <typename T>
__declspec(noinline) std::shared_ptr<T> current(void* handle) {
    return library.handle() == handle ? originCurrentFunc<T>(internal::getCurrentModuleHandle())
                                      : originCurrentFunc<T>(handle);
}

template <typename T>
bool main(T& self) {
    auto loaderVersion = ll::getLoaderVersion();
    self.debug("loader version: {0}", loaderVersion.to_string());
    self.debug("loader type name: {0}", ll::reflection::type_unprefix_name_v<T>);
    if (internalBinaries.empty()) {
        self.fatal("internalBinaries is empty");
        return false;
    }
    auto it = findBestBinary(internalBinaries, loaderVersion);
    if (it == internalBinaries.end()) {
        self.fatal(
            "no embedded internal binary compatible with loader {0} (oldest embedded requires {1})",
            loaderVersion.to_string(),
            internalBinaries.begin()->first.to_string() 
        );
        return false;
    }

    // hack get handle
    T::getByHandle(nullptr);
    if (auto errorCode = pl::pl_hook(
            memory_utils::toFuncPtr(&T::getByHandle),
            memory_utils::toFuncPtr(&current<T>),
            reinterpret_cast<memory_utils::FuncPtr*>(&originCurrentFunc<T>),
            pl::PriorityNormal
        );
        errorCode != 0) {
        LPSTR  messageBuffer{nullptr};
        size_t size = FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            errorCode,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            reinterpret_cast<LPSTR>(&messageBuffer),
            0,
            NULL
        );
        std::string message(messageBuffer, size);
        LocalFree(messageBuffer);

        self.fatal("failed to hook {0}::getByHandle: {0}", ll::reflection::type_unprefix_name_v<T>, message);
        return false;
    }
    static struct Guard {
        ~Guard() { pl::pl_unhook(memory_utils::toFuncPtr(&T::getByHandle), memory_utils::toFuncPtr(&current<T>)); }
    } guard;

    // load internal binary
    self.debug("try loading internal built for {0}", it->first.to_string());
    if (auto error = library.load(it->second); error) {
        self.fatal("failed to load internal binary: {0}", error->what());
        if (!library.tempFile.empty() && (error->code().value() == 126 || error->code().value() == 127)) {
            auto result = pl::dependency_walker::pl_diagnostic_dependency_new(library.tempFile);
            self.fatal("Dependency diagnostic:");
            diagnosticDependency(self, *result);
        }
        // clean up the temp dll left behind by the failed load
        (void)library.free();
        return false;
    }
    auto loadFunc = library.getAddress<typename T::callback_t*>("ll_mod_load");
    if (!loadFunc) {
        self.fatal("failed to find ll_mod_load function");
        return false;
    }
    return loadFunc(self);
}

extern "C" {
__declspec(dllexport) bool ll_mod_load(ll::mod::NativeMod& self) { return main(self); }
__declspec(dllexport) bool ll_plugin_load(ll::plugin::NativePlugin& self) { return main(self); }
}

} // namespace mif::ila_lseexport