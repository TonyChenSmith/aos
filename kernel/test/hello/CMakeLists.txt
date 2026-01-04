# 
# 内核欢迎测试脚本。
# @date 2025-12-27
# 
# Copyright (c) 2025-2026 Tony Chen Smith
# 
# SPDX-License-Identifier: MIT
# 
cmake_minimum_required(VERSION 4.0)
project(aos.kernel.test.hello VERSION 0.0.1 LANGUAGES C ASM)

add_executable(aos.kernel.test.hello
    hello.c
)

add_aos_target(aos.kernel.test.hello $<TARGET_FILE:aos.kernel.test.hello>)