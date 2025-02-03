#
# Copyright (c) 2024-2025 Tony Chen Smith. All rights reserved.
#
# SPDX-License-Identifier: MIT
#

cmake_minimum_required(VERSION 3.5.0)

project(aos.boot VERSION 0.0.1 LANGUAGES C ASM_NASM)

#头文件搜索路径
list(APPEND INCLUDE_DIR_AOS_BOOT ${INCLUDE_DIR_AOS} ${CMAKE_CURRENT_SOURCE_DIR}/include)

#启动核项目，下分几个组件/模块项目。 
list(APPEND BOOT_MODULES "init" "memory" "util")

foreach(BOOT_MODULE ${BOOT_MODULES})
	add_subdirectory(${BOOT_MODULE})
	#复制模块文件到整个包目录
	add_custom_target(
		copy_boot_${BOOT_MODULE}
		ALL
		COMMAND copy ${BOOT_MODULE}\\${BOOT_MODULE}.aem ${BOOT_MODULE}.aem "/Y" ">" "NUL"
		DEPENDS ${PROJECT_BINARY_DIR}/${BOOT_MODULE}/${BOOT_MODULE}.aem
		COMMENT "Copy ${BOOT_MODULE}.aem"
	)
endforeach()