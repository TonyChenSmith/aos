# 
# 内核测试总脚本。
# @date 2025-12-27
# 
# Copyright (c) 2025-2026 Tony Chen Smith
# 
# SPDX-License-Identifier: MIT
# 
cmake_minimum_required(VERSION 4.0)
project(aos.kernel.test VERSION 0.0.1 LANGUAGES C ASM)

include(../script/target.cmake)

file(REAL_PATH ../include INCLUD_ABS_PATH)
include_directories(${INCLUD_ABS_PATH})

add_subdirectory(hello)
add_subdirectory(lib)