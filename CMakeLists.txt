cmake_minimum_required(VERSION 3.20)

project(PowerPlanRevealer LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)

add_library(ppr_core STATIC
        src/core/power_core.c
        src/core/power_attr.c
)

target_include_directories(ppr_core PUBLIC
        src/core
)

target_compile_definitions(ppr_core PUBLIC
        UNICODE
        _UNICODE
)

target_link_libraries(ppr_core PUBLIC
        PowrProf
        Ole32
)

add_executable(power_settings_tool
        src/cli/main_cli.c
)

target_link_libraries(power_settings_tool PRIVATE
        ppr_core
)

set_target_properties(power_settings_tool PROPERTIES
        OUTPUT_NAME "power-plan-revealer-cli"
)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(power_plan_revealer_gui
        src/gui/main.cpp
        src/gui/mainwindow.cpp
        src/gui/mainwindow_ui.cpp
        src/gui/mainwindow_tree.cpp
        src/gui/mainwindow_actions.cpp
        src/gui/mainwindow.h
)

target_link_libraries(power_plan_revealer_gui PRIVATE
        Qt6::Widgets
        ppr_core
)

set_target_properties(power_plan_revealer_gui PROPERTIES
        OUTPUT_NAME "power-plan-revealer"
        WIN32_EXECUTABLE ON
)

if (MSVC)
    target_compile_options(ppr_core PRIVATE
            /W4
            /utf-8
    )

    target_compile_options(power_settings_tool PRIVATE
            /W4
            /utf-8
    )

    target_compile_options(power_plan_revealer_gui PRIVATE
            /W4
            /utf-8
    )

    set_property(TARGET ppr_core PROPERTY
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    )

    set_property(TARGET power_settings_tool PROPERTY
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    )

    set_property(TARGET power_plan_revealer_gui PROPERTY
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    )

    target_link_options(power_plan_revealer_gui PRIVATE
            "/MANIFESTUAC:level='requireAdministrator' uiAccess='false'"
    )
endif()
