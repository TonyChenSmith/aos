cmake_minimum_required(VERSION 3.5.0)

#内核总项目
project(aos VERSION 0.1.0 LANGUAGES C ASM_NASM)

#头文件搜索路径
list(APPEND INCLUDE_DIR_AOS ${CMAKE_CURRENT_SOURCE_DIR}/include)

#添加下属模块
add_subdirectory(boot)