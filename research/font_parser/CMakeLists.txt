# 
# 字体解析器脚本。
# @date 2026-02-05
# 
# Copyright (c) 2026 Tony Chen Smith
# 
# SPDX-License-Identifier: MIT
# 
cmake_minimum_required(VERSION 4.0)
project(aos.research.font_parser VERSION 0.0.1 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(aos.research.font_parser
    src/window.c
)

target_include_directories(aos.research.font_parser PRIVATE ${PROJECT_SOURCE_DIR}/lib/SDL3/include)

if(WIN32)
    set_target_properties(aos.research.font_parser PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
    target_link_libraries(aos.research.font_parser PRIVATE ${PROJECT_SOURCE_DIR}/lib/SDL3/lib/SDL3-static.lib)
    target_link_libraries(aos.research.font_parser PRIVATE
        kernel32
        user32
        gdi32
        winmm
        imm32
        ole32
        oleaut32
        version
        uuid
        advapi32
        setupapi
        shell32
    )
endif()