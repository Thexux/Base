add_rules("mode.release")

add_requires("cereal")

set_languages("c++23")
-- set_languages("c++20")
set_encodings("utf-8")

target("test")
    set_kind("binary")
    add_includedirs("src")
    add_includedirs("base")
    add_files("base/*.cpp")
    add_files("src/main.cpp")

    -- 为MSVC添加调试信息和堆栈跟踪支持
    if is_plat("windows") then
        add_cxflags("/Zi", "/FS")
        add_ldflags("/DEBUG")
    end
