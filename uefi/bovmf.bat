@rem
@rem 构造OVMF固件的发布版本。
@rem @date 2025-06-01
@rem
@rem Copyright (c) 2025 Tony Chen Smith
@rem
@rem SPDX-License-Identifier: MIT
@rem
@echo off
call edksetup.bat

build -p OvmfPkg/OvmfPkgX64.dsc -a X64 -b RELEASE -t CLANGPDB -Y COMPILE_INFO -y BuildReport.log
@echo on