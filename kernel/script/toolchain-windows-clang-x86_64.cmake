# 
# 为Windows Clang/LLVM环境配置的x86-64目标架构工具链文件。
# 在该环境下，仅默认系统也处于x86-64架构下。
# @date 2025-12-02
# 
# Copyright (c) 2025 Tony Chen Smith
# 
# SPDX-License-Identifier: MIT
# 
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_ASM_COMPILER_ID Clang)
set(CMAKE_ASM_COMPILER_TARGET x86_64-pc-unknown-unknown)

set(CMAKE_C_COMPILER_ID Clang)
set(CMAKE_C_COMPILER_TARGET x86_64-pc-unknown-unknown)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_ASM_LINK_EXECUTABLE "<CMAKE_LINKER> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_LINKER> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

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
    -ffreestanding
    -mno-implicit-float
    -nostdinc
    -nostdlib
    -mno-red-zone
    -fno-common
    -fno-omit-frame-pointer
    -fno-builtin
    -funsigned-char
    -mno-stack-arg-probe
    -fno-unwind-tables
    -mcmodel=small

    -Wall
    -Werror

    -fbinutils-version=none
    -fgnuc-version=0
)

# --gc-sections 负责回收未使用代码。
add_link_options(
    --lto=full
    -pie
    --nostdlib
)

if(CMAKE_BUILD_TYPE MATCHES "Release")
    add_link_options(-O3)
else()
    add_link_options(-O0)
endif()