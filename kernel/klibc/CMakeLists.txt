# 
# 内核C标准库模块脚本。
# @date 2025-12-04
# 
# Copyright (c) 2025 Tony Chen Smith
# 
# SPDX-License-Identifier: MIT
# 
cmake_minimum_required(VERSION 4.0)
project(aos.kernel.klibc VERSION 0.0.1 LANGUAGES C ASM)

add_library(aos.kernel.klibc OBJECT
    kstring.S
)