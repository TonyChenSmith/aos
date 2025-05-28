@rem
@rem Copyright (c) 2024-2025 Tony Chen Smith. All rights reserved.
@rem
@rem SPDX-License-Identifier: MIT
@rem
@echo off
build -p OvmfPkg\OvmfPkgx64.dsc -Y COMPILE_INFO -y BuildReport.log
@rem -D SOURCE_DEBUG_ENABLE=TRUE -D DEBUG_ON_SERIAL_PORT
@echo on