#
# Copyright (c) 2024-2025 Tony Chen Smith. All rights reserved.
#
# SPDX-License-Identifier: MIT
#

cmake_minimum_required(VERSION 3.5.0)

#启动核工具模块
project(aos.boot.util VERSION 0.0.1 LANGUAGES C ASM_NASM)

#搜索源文件
list(APPEND SRC_PATHS ".")

list(APPEND SRC_LIST)

foreach(SRC_PATH ${SRC_PATHS})
	file(GLOB_RECURSE INPUTS LIST_DIRECTORIES false "${PROJECT_SOURCE_DIR}/${SRC_PATH}/*.c")
	list(APPEND SRC_LIST ${INPUTS})
	file(GLOB_RECURSE INPUTS LIST_DIRECTORIES false "${PROJECT_SOURCE_DIR}/${SRC_PATH}/*.nasm")
	list(APPEND SRC_LIST ${INPUTS})
endforeach()

#启动核工具目标
set(TARGET_FILE_NAME util.aem)
set(TARGET_ENTRY boot_util_table)
add_executable(${TARGET_FILE_NAME} ${SRC_LIST})
target_include_directories(${TARGET_FILE_NAME} PRIVATE ${INCLUDE_DIR_AOS_BOOT})
target_link_options(${TARGET_FILE_NAME} PRIVATE "--entry=${TARGET_ENTRY}")

#启动核工具反汇编目标
add_custom_target(
	${TARGET_FILE_NAME}.d.txt
	ALL
	COMMAND llvm-objdump --all-headers --full-contents -l --x86-asm-syntax=intel -d ${PROJECT_BINARY_DIR}/${TARGET_FILE_NAME} ">" ${PROJECT_BINARY_DIR}/${TARGET_FILE_NAME}.d.txt
	DEPENDS ${TARGET_FILE_NAME}
	COMMENT "Disassemble ${TARGET_FILE_NAME}"
)

#启动核工具文件头目标
add_custom_target(
	${TARGET_FILE_NAME}.h.txt
	ALL
	COMMAND llvm-readobj --all --program-headers -g --gnu-hash-table -d ${PROJECT_BINARY_DIR}/${TARGET_FILE_NAME} ">" ${PROJECT_BINARY_DIR}/${TARGET_FILE_NAME}.h.txt
	DEPENDS ${TARGET_FILE_NAME}
	COMMENT "Read the ELF heads of ${TARGET_FILE_NAME}"
)