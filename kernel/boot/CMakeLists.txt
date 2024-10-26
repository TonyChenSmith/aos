cmake_minimum_required(VERSION 3.5.0)
project(aos.boot VERSION 0.0.1 LANGUAGES C ASM_NASM)

#引导器项目，下分几个组件/模块项目。
list(APPEND BOOT_MODULES "base" "core")

foreach(BOOT_MODULE ${BOOT_MODULES})
	add_subdirectory(${BOOT_MODULE})
	#复制模块文件到整个包目录
	add_custom_target(
		copy_boot_${BOOT_MODULE}
		ALL
		COMMAND copy ${BOOT_MODULE}\\${BOOT_MODULE}.aem ${BOOT_MODULE}.aem "/Y"
		DEPENDS ${PROJECT_BINARY_DIR}/${BOOT_MODULE}/${BOOT_MODULE}.aem
		COMMENT "Copy ${BOOT_MODULE}.aem"
	)
endforeach()