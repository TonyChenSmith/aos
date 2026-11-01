/**
 * 控制台调试端口输出。
 * @date 2026-11-01
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include <support/format.h>
#include <support/io.h>
#include <support/handle.h>

/**
 * 调试端口UTF8输出句柄魔数。
 */
#define DEBUGCON_UTF8_HANDLE_MAGIC 0x2026110115130102ULL

/**
 * 调试端口地址。
 */
#define DEBUGCON_PORT 0x402

/**
 * 调试端口UTF8输出句柄状态。
 */
static uint64 debugcon_utf8_status=IO_STATUS_SUCCESS;

/**
 * 调试端口UTF8输出句柄。
 */
static io_handle debugcon_utf8_handle;

/**
 * 调试端口UTF8句柄读取。
 * 
 * @param handle 句柄。
 * @param dest   目标数组。
 * @param n      请求读取长度。
 * 
 * @return 恒为0。
 */
static uintn debugcon_utf8_read(io_handle* handle,uint8* dest,uintn n)
{
    (void)handle;
    (void)dest;
    (void)n;
    debugcon_utf8_status=IO_STATUS_NOT_SUPPORTED;
    return 0;
}

/**
 * 调试端口UTF8句柄写入。
 * 
 * @param handle 句柄。
 * @param src    源数组。
 * @param n      请求写入长度。
 * 
 * @return 实际写入长度，恒等于请求长度。
 */
static uintn debugcon_utf8_write(io_handle* handle,const uint8* src,uintn n)
{
    (void)handle;
    if(src==null)
    {
        debugcon_utf8_status=IO_STATUS_DEVICE_ERROR;
        return 0;
    }

    uintn result=n;
    uintn end=(uintn)&src[n];

    while((uintn)src<end)
    {
        uintn len;
        const uint8* next=null;
        uintn next_n=0;
        for(len=0;len<n;len++)
        {
            if((src[len]!='\t'&&src[len]!='\n'&&src[len]!='\r'&&src[len]<0x20)||src[len]==0x7F)
            {
                next=&src[len+1];
                next_n=n-len-1;
                break;
            }
        }

        if(len>0)
        {
            x86_write_port8_repeat(DEBUGCON_PORT,src,len);
        }

        if(next_n==0)
        {
            break;
        }
        src=next;
        n=next_n;
    }

    debugcon_utf8_status=IO_STATUS_SUCCESS;
    return result;
}

/**
 * 调试端口UTF8句柄刷新。
 * 
 * @param handle 句柄。
 * 
 * @return 状态码。
 */
static uint64 debugcon_utf8_flush(io_handle* handle)
{
    (void)handle;
    debugcon_utf8_status=IO_STATUS_SUCCESS;
    return debugcon_utf8_status;
}

/**
 * 调试端口UTF8句柄获取位置。
 * 
 * @param handle 句柄。
 * 
 * @return 恒为0。
 */
static uint64 debugcon_utf8_get_position(io_handle* handle)
{
    (void)handle;
    debugcon_utf8_status=IO_STATUS_SUCCESS;
    return 0;
}

/**
 * 调试端口UTF8句柄设置位置。句柄不支持寻址。
 * 
 * @param handle   句柄。
 * @param position 目标位置。
 * 
 * @return 操作不支持。
 */
static uint64 debugcon_utf8_set_position(io_handle* handle,uint64 position)
{
    (void)handle;
    (void)position;
    debugcon_utf8_status=IO_STATUS_NOT_SUPPORTED;
    return debugcon_utf8_status;
}

/**
 * 调试端口UTF8句柄获取大小。
 * 
 * @param handle 句柄。
 * 
 * @return 恒为0。
 */
static uint64 debugcon_utf8_get_size(io_handle* handle)
{
    (void)handle;
    debugcon_utf8_status=IO_STATUS_SUCCESS;
    return 0;
}

/**
 * 调试端口UTF8句柄获取能力。
 * 
 * @param handle 句柄。
 * 
 * @return 只写能力。
 */
static uint64 debugcon_utf8_get_capabilities(io_handle* handle)
{
    (void)handle;
    debugcon_utf8_status=IO_STATUS_SUCCESS;
    return IO_CAPABILITY_WRITABLE;
}

/**
 * 调试端口UTF8句柄获取状态。
 * 
 * @param handle 句柄。
 * 
 * @return 最近一次状态码，读取后重置。
 */
static uint64 debugcon_utf8_get_io_status(io_handle* handle)
{
    uint64 status=debugcon_utf8_status;

    (void)handle;
    debugcon_utf8_status=IO_STATUS_SUCCESS;
    return status;
}

/**
 * 调试端口UTF8句柄关闭。句柄不支持关闭。
 * 
 * @param handle 句柄。
 * 
 * @return 操作不支持。
 */
static uint64 debugcon_utf8_close(io_handle* handle)
{
    (void)handle;
    debugcon_utf8_status=IO_STATUS_NOT_SUPPORTED;
    return debugcon_utf8_status;
}

/**
 * 初始化控制台调试端口输出。
 * 
 * @return 无返回值。
 */
void console_debugcon_init(void)
{
    debugcon_utf8_handle.magic=DEBUGCON_UTF8_HANDLE_MAGIC;
    debugcon_utf8_handle.read=debugcon_utf8_read;
    debugcon_utf8_handle.write=debugcon_utf8_write;
    debugcon_utf8_handle.flush=debugcon_utf8_flush;
    debugcon_utf8_handle.get_position=debugcon_utf8_get_position;
    debugcon_utf8_handle.set_position=debugcon_utf8_set_position;
    debugcon_utf8_handle.get_size=debugcon_utf8_get_size;
    debugcon_utf8_handle.get_capabilities=debugcon_utf8_get_capabilities;
    debugcon_utf8_handle.get_io_status=debugcon_utf8_get_io_status;
    debugcon_utf8_handle.close=debugcon_utf8_close;
}

/**
 * 在控制台调试端口格式化输出字符串。
 * 
 * @param format 格式化字符串。
 * 
 * @return 输出状态码。
 */
uintn console_debugcon_print(const char8* format,...)
{
    va_list args;

    va_start(args,format);
    uintn result=format_print_valist(&debugcon_utf8_handle,format,args);
    va_end(args);

    return result;
}

/**
 * 在控制台调试端口通过可变参数列表格式化输出字符串。
 * 
 * @param format 格式化字符串。
 * @param args   可变参数列表。
 * 
 * @return 输出状态码。
 */
uintn console_debugcon_print_valist(const char8* format,va_list args)
{
    va_list list;

    va_copy(list,args);
    uintn result=format_print_valist(&debugcon_utf8_handle,format,list);
    va_end(list);

    return result;
}