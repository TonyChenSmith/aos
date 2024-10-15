@echo off
build -p OvmfPkg\OvmfPkgx64.dsc -Y COMPILE_INFO -y BuildReport.log
@rem -D SOURCE_DEBUG_ENABLE=TRUE -D DEBUG_ON_SERIAL_PORT
@echo on