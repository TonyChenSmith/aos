# 
# 内核恐慌模块脚本。
# @date 2026-04-06
# 
# Copyright (c) 2026 Tony Chen Smith
# 
# SPDX-License-Identifier: MIT
# 
cmake_minimum_required(VERSION 4.0)
project(aos.kernel.panic VERSION 0.0.1 LANGUAGES C ASM)

add_library(aos.kernel.panic OBJECT
    panic.c
)