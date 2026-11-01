# 
# 内核控制台模块脚本。
# @date 2026-10-20
# 
# Copyright (c) 2026 Tony Chen Smith
# 
# SPDX-License-Identifier: MIT
# 
cmake_minimum_required(VERSION 4.0)
project(aos.kernel.console VERSION 0.0.1 LANGUAGES C ASM)

set(AOS_CONSOLE_FONTS "${CMAKE_CURRENT_SOURCE_DIR}/../../research/kfonts/out/kfonts")
set_source_files_properties(fonts.S PROPERTIES OBJECT_DEPENDS "${AOS_CONSOLE_FONTS}")

add_library(aos.kernel.console OBJECT
    debugcon.c
    fonts.S
    graphics.c
    output.c
)