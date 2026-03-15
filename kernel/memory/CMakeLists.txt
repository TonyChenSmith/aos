# 
# 内核内存管理模块脚本。
# @date 2026-03-09
# 
# Copyright (c) 2026 Tony Chen Smith
# 
# SPDX-License-Identifier: MIT
# 
cmake_minimum_required(VERSION 4.0)
project(aos.kernel.memory VERSION 0.0.1 LANGUAGES C ASM)

add_library(aos.kernel.memory OBJECT
    pcp_pool.c
)