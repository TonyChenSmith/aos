cmake_minimum_required(VERSION 3.5.0)
project(aos.boot VERSION 0.0.1 LANGUAGES C ASM_NASM)

#引导器项目，下分几个组件/模块项目。
add_subdirectory(core)