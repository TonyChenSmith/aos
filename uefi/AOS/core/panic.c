/**
 * 模块恐慌输出管理。
 * @date 2026-06-29
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include <Uefi.h>
#include <Library/CpuLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>

/**
 * 输出静态缓存。
 */
STATIC CHAR16 buffer[1024];

/**
 * 在UEFI BSP出错时显示出错屏幕并进入待机循环。该方案仅限启动服务结束前使用。换行到新行起始需要\r\n。
 * 
 * @param format 格式化字符串。按EDK2规则。
 * 
 * @return 无法返回。
 */
VOID NORETURN EFIAPI bsp_panic(EFI_STATUS status,CONST CHAR8* file,UINTN line,CONST CHAR8* format,...)
{
    if(gST->ConOut!=NULL&&!EFI_ERROR(gST->ConOut->Reset(gST->ConOut,FALSE)))
    {
        UINTN result=UnicodeSPrintAsciiFormat(buffer,2048,
            "Status : %r (0x%016lX)\r\nSource : %a:%lu\r\nDetail : ",status,(UINT64)status,file,line);
        if(result>0)
        {
            gST->ConOut->OutputString(gST->ConOut,buffer);
        }
        VA_LIST list;
        VA_START(list,format);
        result=UnicodeVSPrintAsciiFormat(buffer,2048,format,list);
        VA_END(list);
        if(result>0)
        {
            gST->ConOut->OutputString(gST->ConOut,buffer);
        }
    }
    while(TRUE)
    {
        CpuSleep();
    }
}