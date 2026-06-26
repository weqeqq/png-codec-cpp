
-- stylua: ignore start

set_project("weqeqq.png")
set_version("0.1.0")

add_rules("plugin.compile_commands.autoupdate", "mode.debug", "mode.release")

set_policy("build.c++.modules.std", false)

set_languages("c++23")

add_repositories("weqeqq.repo https://github.com/weqeqq/xmake-repo.git")

add_requires("weqeqq.error ~0.2.0")
add_requires("wuffs ~0.4.0")
add_requires("fpng")
add_requires("weqeqq.color ~0.1.0")
add_requires("weqeqq.parallel")

option("tests")
    set_default(false)
    set_showmenu(true)
    set_description("Build tests")
option_end()

if has_config("tests") then
    add_requires("weqeqq.test ~0.3.6")
end

target("weqeqq.png")
    set_kind("static")
    add_files("sources/**.cppm", { public = true })
    add_packages("weqeqq.error", "weqeqq.color", "weqeqq.parallel", { public = true })
    add_packages("wuffs", "fpng")

if has_config("tests") then
    includes("tests")
end
