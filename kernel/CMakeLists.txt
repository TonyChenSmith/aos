# 
# 内核总脚本。
# @date 2025-12-02
# 
# Copyright (c) 2025 Tony Chen Smith
# 
# SPDX-License-Identifier: MIT
# 
cmake_minimum_required(VERSION 4.0)
project(aos.kernel VERSION 0.0.1 LANGUAGES C ASM)

include(script/target.cmake)

include_directories(include)

set(AOS_KERNEL aos.kernel)

# 模块列表
add_subdirectory(init)
add_subdirectory(klibc)

add_executable(${AOS_KERNEL}
    entry/main.c
    entry/trampoline.S

    entry/temp.S
)

# 模块链接

if(CMAKE_BUILD_TYPE MATCHES "Release")
    target_link_options(${AOS_KERNEL} PRIVATE --script=${CMAKE_CURRENT_SOURCE_DIR}/script/release.ld)
    set_target_properties(${AOS_KERNEL} PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/script/release.ld)
else()
    target_link_options(${AOS_KERNEL} PRIVATE --script=${CMAKE_CURRENT_SOURCE_DIR}/script/debug.ld)
    set_target_properties(${AOS_KERNEL} PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/script/debug.ld)
endif()

target_link_libraries(${AOS_KERNEL} PRIVATE aos.kernel.init)
target_link_libraries(${AOS_KERNEL} PRIVATE aos.kernel.klibc)

add_aos_target(${AOS_KERNEL})