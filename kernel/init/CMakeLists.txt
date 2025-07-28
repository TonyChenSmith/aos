# 
# Copyright (c) 2025 Tony Chen Smith
# 
# SPDX-License-Identifier: MIT
# 
cmake_minimum_required(VERSION 4.0)
project(aos.kernel.init VERSION 0.0.1 LANGUAGES C ASM)

add_library(aos.kernel.init OBJECT
    test.c
    ttc.S
)