@rem
@rem Copyright (c) 2025 Tony Chen Smith. All rights reserved.
@rem
@rem SPDX-License-Identifier: MIT
@rem
@echo off
call edksetup.bat

build -p AOS/AOS.dsc -a X64 -t CLANGPDB -Y COMPILE_INFO -y BuildReport.log
@echo on