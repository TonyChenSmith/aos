# 
# 内核总脚本。
# @date 2025-12-02
# 
# Copyright (c) 2025-2026 Tony Chen Smith
# 
# SPDX-License-Identifier: MIT
# 
cmake_minimum_required(VERSION 4.0)
project(aos.kernel VERSION 0.0.1 LANGUAGES C ASM)

include(script/target.cmake)

include_directories(include)

# 模块列表
add_subdirectory(init)
add_subdirectory(lib)

add_executable(aos.kernel
    entry/main.c
    entry/trampoline.S

    entry/temp.S
)

# 模块链接
if(CMAKE_BUILD_TYPE MATCHES "Release")
    target_link_options(aos.kernel PRIVATE --script=${CMAKE_CURRENT_SOURCE_DIR}/script/release.ld)
    set_target_properties(aos.kernel PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/script/release.ld)
else()
    target_link_options(aos.kernel PRIVATE --script=${CMAKE_CURRENT_SOURCE_DIR}/script/debug.ld)
    set_target_properties(aos.kernel PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/script/debug.ld)
endif()

target_link_libraries(aos.kernel PRIVATE aos.kernel.init)
target_link_libraries(aos.kernel PRIVATE aos.kernel.lib)

add_aos_target(aos.kernel $<TARGET_FILE:aos.kernel>)

# 部署构建测试程序
enable_testing()
add_test(NAME kernel_test_config COMMAND ${CMAKE_COMMAND} -S ${aos.kernel_SOURCE_DIR}/test --preset ${CMAKE_BUILD_TYPE})
add_test(NAME kernel_test_build COMMAND ${CMAKE_COMMAND} 
    --build ${aos.kernel_SOURCE_DIR}/out/test/${CMAKE_BUILD_TYPE}/build)