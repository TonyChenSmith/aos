cmake_minimum_required(VERSION 3.5.0)
project(aos-kernel VERSION 0.1.0 LANGUAGES C ASM)

#搜索源文件
list(APPEND SRC_PATHS "kernel")

list(APPEND SRC_LIST)

foreach(SRC_PATH ${SRC_PATHS})
	file(GLOB_RECURSE INPUTS LIST_DIRECTORIES false "${CMAKE_SOURCE_DIR}/${SRC_PATH}/*.c")
	list(APPEND SRC_LIST ${INPUTS})
endforeach()

#内核主程序目标
add_executable(aoskernel ${SRC_LIST})
target_include_directories(aoskernel PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_compile_options(aoskernel PRIVATE "-m64" "-flto" "-fPIE" "-nostdlib" "-nostdinc")
target_link_options(aoskernel PRIVATE "--pie" "-nostdlib" "--entry=aos_kernel_entry")

#内核主程序分析目标
add_custom_target(
	aoskernel.d.txt
	ALL
	COMMAND llvm-objdump --all-headers --full-contents -l --x86-asm-syntax=intel -d ${CMAKE_BINARY_DIR}/aoskernel ">" ${CMAKE_BINARY_DIR}/aoskernel.d.txt
	DEPENDS aoskernel
	COMMENT "Disassemble aoskernel"
)

#内核主程序头分析目标
add_custom_target(
	aoskernel.h.txt
	ALL
	COMMAND llvm-readobj --all --program-headers -g --gnu-hash-table -d ${CMAKE_BINARY_DIR}/aoskernel ">" ${CMAKE_BINARY_DIR}/aoskernel.h.txt
	DEPENDS aoskernel
	COMMENT "Read the heads of aoskernel"
)