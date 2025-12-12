@rem
@rem 构造模块“aos.uefi”的调试版本。
@rem @date 2025-06-01
@rem
@rem Copyright (c) 2025 Tony Chen Smith
@rem
@rem SPDX-License-Identifier: MIT
@rem
@echo off
call edksetup.bat

build -p AOS/AOS.dsc -a X64 -t CLANGPDB -Y COMPILE_INFO -y BuildReport.log
@echo on