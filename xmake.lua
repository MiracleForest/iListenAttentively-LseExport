add_rules("mode.debug", "mode.release")

add_repositories("liteldev-repo https://github.com/LiteLDev/xmake-repo.git")

add_requires("levilamina 1.1.1", {configs = {target_type = "server"}})
add_requires("levibuildscript 0.3.0")
add_requires("legacyremotecall 0.10.0")

if not has_config("vs_runtime") then
    set_runtimes("MD")
end

target("iListenAttentively-LseExport")
    add_rules("@levibuildscript/linkrule")
    add_rules("@levibuildscript/modpacker")
    add_cxflags(
        "/EHa",
        "/utf-8",
        "/W4",
        "/w44265",
        "/w44289",
        "/w44296",
        "/w45263",
        "/w44738",
        "/w45204"
    )
    add_defines(
        "NOMINMAX",
        "UNICODE",
        "_HAS_CXX23=1"
    )
    add_packages(
        "levilamina",
        "legacyremotecall"
    )
    set_optimize("fastest")
    set_exceptions("none")
    set_kind("shared")
    set_languages("cxx20")
    set_symbols("debug")
    add_files("src/**.cpp")
    add_includedirs("src")
    after_build(function (target)
        local lib_path = path.join(os.projectdir(), "lib")
        local output_path = path.join(os.projectdir(), "bin", target:name(), "lib")
        os.rm(output_path)
        os.cp(lib_path, output_path)
    end)