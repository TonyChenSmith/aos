# 
# 内核初始化模块脚本。
# @date 2025-12-02
# 
# Copyright (c) 2025 Tony Chen Smith
# 
# SPDX-License-Identifier: MIT
# 
cmake_minimum_required(VERSION 4.0)
project(aos.kernel.init VERSION 0.0.1 LANGUAGES C ASM)

add_library(aos.kernel.init OBJECT
    init.c
)