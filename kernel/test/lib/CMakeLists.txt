# 
# 内核库测试脚本。
# @date 2026-01-04
# 
# Copyright (c) 2026 Tony Chen Smith
# 
# SPDX-License-Identifier: MIT
# 
cmake_minimum_required(VERSION 4.0)
project(aos.kernel.test.lib VERSION 0.0.1 LANGUAGES C ASM)

add_executable(aos.kernel.test.lib
    memory.c
    test.c
    ../../lib/memory.c
)

add_aos_target(aos.kernel.test.lib $<TARGET_FILE:aos.kernel.test.lib>)