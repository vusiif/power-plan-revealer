cmake_minimum_required(VERSION 3.20)

project(PowerSettingsTool C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(power_settings_tool
        src/main.c
        src/power_enum.c
        src/power_attr.c
)

target_compile_definitions(power_settings_tool PRIVATE
        UNICODE
        _UNICODE
)

target_link_libraries(power_settings_tool PRIVATE
        PowrProf
)

if (MSVC)
    target_compile_options(power_settings_tool PRIVATE
            /W4
            /utf-8
    )

#    # 让程序启动时请求管理员权限。
#    # 如果你只想 list 时不弹 UAC，可以先注释掉这一行。
#    target_link_options(power_settings_tool PRIVATE
#            "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\""
#    )
endif()
