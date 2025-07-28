# 
# Copyright (c) 2025 Tony Chen Smith
# 
# SPDX-License-Identifier: MIT
# 
cmake_minimum_required(VERSION 4.0)
project(aos.kernel VERSION 0.0.1 LANGUAGES C ASM)

include_directories(include)

add_subdirectory(init)
add_executable(aoskernel
    trampoline/main.c
)
target_link_libraries(aoskernel PRIVATE aos.kernel.init)
target_link_options(aoskernel PRIVATE --entry=tt)