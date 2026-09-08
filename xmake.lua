add_rules("mode.debug", "mode.release")
add_rules("plugin.vsxmake.autoupdate")
add_rules("plugin.compile_commands.autoupdate")

add_repositories("liteldev-repo https://github.com/LiteLDev/xmake-repo.git")

add_requires(
    "entt v3.15.0",
    "expected-lite v0.8.0",
    "fmt 11.2.0",
    "gsl v4.2.0",
    "glm 1.0.1",
    "leveldb 1.23",
    "magic_enum v0.9.7",
    "nlohmann_json v3.12.0",
    "rapidjson 2025.02.05",
    "type_safe v0.2.4",
    "pcg_cpp v1.0.0",
    "pfr 2.1.1",
    "symbolprovider v1.3.0",
    "parallel-hashmap v1.3.12",
    "concurrentqueue v1.0.4",
    "stb 2025.03.14",
    "preloader 1.16.2",
    "demangler v17.0.7",
    "dyncall 1.4"
)

if not has_config("vs_runtime") then
    set_runtimes("MD")
end

local internal_targets = {}
for _, dir in ipairs(os.dirs("sdk/*")) do
    local levilamina_version = path.filename(dir)
    table.insert(internal_targets, "Internal_" .. levilamina_version)
    target("Internal_" .. levilamina_version)
        set_kind("binary")
        set_default(false)
        set_policy("build.fence", true)
        add_cxflags(
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
            "_HAS_CXX23=1",
            "_SILENCE_CXX20_IS_ALWAYS_EQUAL_DEPRECATION_WARNING=1"
        )
        add_packages(
            "entt",
            "expected-lite",
            "fmt",
            "gsl",
            "glm",
            "leveldb",
            "magic_enum",
            "nlohmann_json",
            "rapidjson",
            "type_safe",
            "pcg_cpp",
            "pfr",
            "symbolprovider",
            "parallel-hashmap",
            "concurrentqueue",
            "stb",

            "dyncall"
        )
        set_optimize("aggressive")
        set_exceptions("none")
        set_kind("shared")
        set_languages("cxx20")
        set_symbols("debug")
        add_files("src/ila-lseexport/**.cpp")
        add_includedirs("src")
        add_shflags("/DELAYLOAD:bedrock_runtime.dll")

        add_includedirs(path.join(os.projectdir(), "sdk", levilamina_version, "include"))
        add_linkdirs(path.join(os.projectdir(), "sdk", levilamina_version, "lib"))
        add_links("LeviLamina")
        add_links("LegacyRemoteCall")

        on_load(function (target)
            import("core.base.semver")
            import("core.base.task")
            import("core.base.json")
            import("private.action.require.impl.package")
            import("private.action.require.impl.repository")
            import("private.action.require.impl.environment")

            -- 更新 cxflag 跟 define
            if semver.new(levilamina_version):ge("26.20.0") then
                target:set("toolchains", "clang-cl")
                target:add(
                    "cxflags",
                    "-Wno-microsoft-cast",
                    "-Wno-invalid-offsetof",
                    "-Wno-c++2b-extensions",
                    "-Wno-microsoft-include",
                    "-Wno-overloaded-virtual",
                    "-Wno-ignored-qualifiers",
                    "-Wno-missing-field-initializers",
                    "-Wno-potentially-evaluated-expression",
                    "-Wno-pragma-system-header-outside-header",
                    { tools = { "clang_cl" } }
                )
            else
                target:add("cxflags", "/EHa")
            end
            target:add("defines", "LevilaminaVersion=" .. levilamina_version:gsub("%.", "_"))

            -- 获取 bedrockdata 版本并添加依赖和宏定义
            if not repository.pulled() then task.run("repo", { update = true } ) end -- 更新 repo
            environment.enter()
            local instances = package.load_packages("levilamina " .. levilamina_version)
            for _, package in ipairs(instances) do
                if package:name() == "bedrockdata" or package:fullname() == "bedrockdata" then
                    local version = package:version()
                    local mcversion = version and version:shortstr() or (package:version_str():match("([%d%.]+)") or package:version_str())
                    target:add("defines", "MCVersion=" .. mcversion:gsub("%.", "_"))
                end
            end
            environment.leave()
        end)

        before_link(function(target)
            import("lib.detect.find_file")
            import("core.project.config")

            local libdir = path.join(config.builddir(), ".prelink", "lib")
            if os.exists(libdir) then os.rm(libdir) end
            os.mkdir(libdir)

            os.execv(path.join(os.projectdir(), "sdk", levilamina_version, "bin", "prelink.exe"), {
                string.format("%s-%s-%s", get_config("target_type"), target:plat(), target:arch()),
                path.join(config.builddir(), ".prelink"),
                path.join(os.projectdir(), "sdk", levilamina_version, "bin", "bedrock_runtime_data"),
                table.unpack(target:objectfiles())
            })

            target:add("linkdirs", libdir)
            target:add("links", "bedrock_runtime_api")
            target:add("links", "bedrock_runtime_var")
        end)
end

target("FakeLeviLamina")
    add_files("src/fake_levilamina/**.cpp")
    add_includedirs("src")
    set_basename("LeviLamina")
    add_defines("LL_EXPORT")

    after_load(function (target)
        import("core.project.project")
        local internal = project.target(internal_targets[1])
        
        for _, name in ipairs({"cxflags", "defines"}) do
            local exists = {}
            for _, value in ipairs(table.wrap(target:get(name))) do exists[value] = true end
            local extraconf = internal:extraconf(name)
            for _, value in ipairs(table.wrap(internal:get(name))) do
                if not exists[value] then
                    local conf = extraconf and extraconf[value]
                    if conf then target:add(name, value, conf)
                    else target:add(name, value) end
                    exists[value] = true
                end
            end
        end

        for _, name in ipairs({"optimize", "exceptions", "symbols", "strip", "languages", "kind"}) do
            local values = table.wrap(internal:get(name))
            if #values > 0 then target:set(name, table.unpack(values)) end
        end
    end)

target("iListenAttentively-LseExport")
    if #internal_targets > 0 then add_deps(table.unpack(internal_targets), { inherit = false }) end
    add_deps("FakeLeviLamina")
    add_files("src/adapt/**.cpp")
    add_includedirs("src")
    add_shflags("/DELAYLOAD:LeviLamina.dll")
    add_packages("preloader", "demangler", "symbolprovider")

    after_load(function (target)
        import("core.project.project")
        local internal = project.target(internal_targets[1])
        
        for _, name in ipairs({"cxflags", "defines"}) do
            local exists = {}
            for _, value in ipairs(table.wrap(target:get(name))) do exists[value] = true end
            local extraconf = internal:extraconf(name)
            for _, value in ipairs(table.wrap(internal:get(name))) do
                if not exists[value] then
                    local conf = extraconf and extraconf[value]
                    if conf then target:add(name, value, conf)
                    else target:add(name, value) end
                    exists[value] = true
                end
            end
        end

        for _, name in ipairs({"optimize", "exceptions", "symbols", "strip", "languages", "kind"}) do
            local values = table.wrap(internal:get(name))
            if #values > 0 then target:set(name, table.unpack(values)) end
        end
    end)

    before_build(function (target)
        local outfile = path.join(os.projectdir(), "src", "adapt", "Internal.h")
        local arrays = {}
        local entries = {}
        local prefix = "Internal_"
        for _, name in ipairs(internal_targets) do
            local dep = target:dep(name)
            if not dep then raise("target(%s): dep(%s) not found!", target:name(), name) end

            local dllpath = dep:targetfile()
            local data = io.readfile(dllpath, { encoding = "binary" } )
            if not data then raise("target(%s): cannot read %s", target:name(), dllpath) end

            local version = name:sub(#prefix + 1)
            local symbol = "kInternalData_" .. version:gsub("%W", "_")
            local bytes = {}
            for i = 1, #data do
                table.insert(bytes, string.format("0x%02X", string.byte(data, i)))
            end
            table.insert(arrays, string.format("static constexpr uint8_t %s[] = {%s};", symbol, table.concat(bytes, ", ")))
            table.insert(entries, string.format("    {ll::data::Version{\"%s\"}, std::span<uint8_t const>{%s, %d}}", version, symbol, #data))
        end
        local header = {
            "#pragma once",
            "#include \"fake_levilamina/Version.h\"",
            "#include <cstdint>",
            "#include <map>",
            "#include <span>",
            "",
            "namespace mif::ila_lseexport {",
            ""
        }
        local content = table.concat(header, "\n")
        content = content .. table.concat(arrays, "\n") .. "\n\n"
        content = content .. "inline const std::map<ll::data::Version, std::span<std::uint8_t const>> internalBinaries{\n"
        content = content .. table.concat(entries, ",\n") .. "\n"
        content = content .. "};\n"
        content = content .. "\n} // namespace mif::ila_lseexport\n"
        if not os.isfile(outfile) or io.readfile(outfile) ~= content then
            io.writefile(outfile, content)
        end
    end)

    after_build(function (target)
        local output_dir = path.join(os.projectdir(), "bin", target:name())

        os.rm(output_dir)

        os.vcp(target:targetfile(), format("%s/", output_dir))
        os.vcp(target:symbolfile(), format("%s/", output_dir))
        os.vcp(path.join(os.projectdir(), "assets", "*"), format("%s/", output_dir))

        import("scripts.generate-manifest", { rootdir = os.projectdir() }).generate_manifest(
            format("%s/manifest.json", output_dir),
            {
                name = target:name(),
                entry = path.basename(target:targetfile()),
                author = "MiracleForest",
                version = import("scripts.get-version-info", { rootdir = os.projectdir() }).get_version_info().version_str,
                description = "Export events to LegacyScriptEngine",
                dependencies = {
                    { name = "LegacyRemoteCall" }
                },
                extraInfo = {
                    gitHub = "https://github.com/MiracleForest/iListenAttentively-LseExport"
                }
            }
        )
    end)
