/**
 * 模块恐慌函数声明。
 * @date 2026-07-16
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_UEFI_PANIC_H__
#define __AOS_UEFI_PANIC_H__

#include <Uefi.h>

/**
 * BSP期间恐慌函数。
 */
#define panic(status,format,...) bsp_panic(status,__FILE__,__LINE__,format,##__VA_ARGS__)

/**
 * BSP期间错误状态恐慌函数。
 */
#define panic_if_error(expression,format,...)\
    do\
    {\
        EFI_STATUS _status=(expression);\
        if(EFI_ERROR(_status))\
        {\
            bsp_panic(_status,__FILE__,__LINE__,format,##__VA_ARGS__);\
        }\
    }while(0)

/**
 * 在UEFI BSP出错时显示出错屏幕并进入待机循环。该方案仅限启动服务结束前使用。换行到新行起始需要\r\n。
 * 
 * @param format 格式化字符串。按EDK2规则。
 * 
 * @return 无法返回。
 */
VOID NORETURN EFIAPI bsp_panic(EFI_STATUS status,CONST CHAR8* file,UINTN line,CONST CHAR8* format,...);

#endif /*__AOS_UEFI_PANIC_H__*/