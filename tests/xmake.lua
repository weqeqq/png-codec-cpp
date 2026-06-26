-- stylua: ignore start

target("tests")
    set_kind("binary")
    set_default(false)
    add_deps("weqeqq.png")
    add_files("sources/*.cppm")
    set_policy("build.c++.modules", true)
    add_packages("weqeqq.test", { components = { "core", "main" } })
    add_tests("default")
