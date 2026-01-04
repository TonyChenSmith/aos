# 
# 为Windows Clang/LLVM环境配置的x86-64目标架构测试工具链文件。
# 在该环境下，仅默认系统也处于x86-64架构下。
# @date 2025-12-28
# 
# Copyright (c) 2025 Tony Chen Smith
# 
# SPDX-License-Identifier: MIT
# 
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_ASM_COMPILER_ID Clang)
set(CMAKE_ASM_COMPILER_TARGET x86_64-pc-windows-msvc)

set(CMAKE_C_COMPILER_ID Clang)
set(CMAKE_C_COMPILER_TARGET x86_64-pc-windows-msvc)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(
    -march=x86-64
    -mno-mmx
    -mno-x87
    -mno-sse
    -mno-sse2
    -mno-fxsr

    -flto
    -funified-lto

    -fexec-charset=utf-8
    -mno-implicit-float
    -mno-red-zone
    -fno-common
    -fno-omit-frame-pointer
    -funsigned-char
    -fshort-wchar
    -mno-stack-arg-probe
    -fno-unwind-tables
    -mcmodel=small

    -Wall
    -Werror

    -fbinutils-version=none
    -fgnuc-version=0
)

add_link_options(
    -flto=full
)

if(CMAKE_BUILD_TYPE MATCHES "Release")
    add_link_options(-O3)
else()
    add_link_options(-O0)
endif()