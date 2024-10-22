cmake_minimum_required(VERSION 3.5.0)

#内核总项目
project(aos VERSION 0.1.0 LANGUAGES C ASM_NASM)

#添加下属模块
add_subdirectory(boot)