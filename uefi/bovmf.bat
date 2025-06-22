@rem
@rem Copyright (c) 2025 Tony Chen Smith
@rem
@rem SPDX-License-Identifier: MIT
@rem
@echo off
call edksetup.bat

build -p OvmfPkg/OvmfPkgX64.dsc -a X64 -t CLANGPDB -Y COMPILE_INFO -y BuildReport.log
@echo on