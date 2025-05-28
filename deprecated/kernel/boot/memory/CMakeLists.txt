#
# Copyright (c) 2024-2025 Tony Chen Smith. All rights reserved.
#
# SPDX-License-Identifier: MIT
#

cmake_minimum_required(VERSION 3.20)

#启动核内存模块
project(aos.boot.memory VERSION 0.0.1 LANGUAGES C ASM_NASM)

#搜索源文件
list(APPEND SRC_PATHS ".")

list(APPEND SRC_LIST)

foreach(SRC_PATH ${SRC_PATHS})
	file(GLOB_RECURSE INPUTS LIST_DIRECTORIES false "${PROJECT_SOURCE_DIR}/${SRC_PATH}/*.c")
	list(APPEND SRC_LIST ${INPUTS})
	file(GLOB_RECURSE INPUTS LIST_DIRECTORIES false "${PROJECT_SOURCE_DIR}/${SRC_PATH}/*.nasm")
	list(APPEND SRC_LIST ${INPUTS})
endforeach()

#启动核内存目标
set(TARGET_FILE_NAME memory.aem)
set(TARGET_ENTRY boot_memory_table)

add_executable(${TARGET_FILE_NAME} ${SRC_LIST})

target_include_directories(${TARGET_FILE_NAME} PRIVATE ${INCLUDE_DIR_AOS_BOOT})
target_link_options(${TARGET_FILE_NAME} PRIVATE "--entry=${TARGET_ENTRY}" "--pie")

add_aos_target(${PROJECT_BINARY_DIR}/${TARGET_FILE_NAME})