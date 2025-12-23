# AOS内核项目

## 目录
1. [简介](#简介)
2. [可移植性说明](#可移植性说明)

## 简介
本项目是运行在EFI系统分区（ESP）的单一ELF格式程序。程序使用System V ABI，通过CMake+Ninja调动Clang+LLVM进行编译链接。  
当前使用CMake版本4.0.2，LLVM版本为20.1.7。

## 可移植性说明
本项目的构建系统非常依赖Clang工具链特性，因此构建方面的可移植性需要在`script`目录下创建新的工具链文件，替代`toolchain-windows-clang-x86_64.cmake`这一工具链文件。  
本项目的语言特性目前的实现方案是依赖Clang语言特性的，但所有语言特性将约束在`lib`这一子目录下的`aos.kernel.lib`模块。如果需要进行移植，对`include/lib`下的头文件与`lib`目录下的实现进行修改，提供替代方案即可。其他模块只允许调用由`aos.kernel.lib`实现或包装的函数。