/*
 * 基础模块函数和结构定义。定义了所有导出函数和入口函数。
 * @date 2024-10-27
 */
#ifndef __AOS_BOOT_BASE_H__
#define __AOS_BOOT_BASE_H__

#include "basic_type.h"

/*
 * 对MS ABI两参数函数的调用。
 *
 * @param func 被调用函数地址。
 * @param arg1 被调用函数的第一个参数。
 * @param arg2 被调用函数的第二个参数。
 * 
 * @return 被调用函数的返回值。
 */
typedef uintn (*boot_ms_call_2)(void* restrict func,uintn arg1,uintn arg2);

/*
 * 对MS ABI四参数函数的调用。
 *
 * @param func 被调用函数地址。
 * @param arg1 被调用函数的第一个参数。
 * @param arg2 被调用函数的第二个参数。
 * @param arg3 被调用函数的第三个参数。
 * @param arg4 被调用函数的第四个参数。
 * 
 * @return 被调用函数的返回值。
 */
typedef uintn (*boot_ms_call_4)(void* restrict func,uintn arg1,uintn arg2,uintn arg3,uintn arg4);

/*
 * 内存复制，行为类似于string.h的memcpy函数。
 *
 * @param s1 输出内存。
 * @param s2 输入内存。
 * @param n  复制字节数。
 * 
 * @return 输出内存。
 */
typedef void* (*boot_memcpy)(void* restrict s1,const void* restrict s2,uintn n);

/*
 * 内存设置，行为类似于string.h的memset函数。
 *
 * @param s	写入内存。
 * @param c	写入值。
 * @param n	写入字节数。
 * 
 * @return 写入内存。
 */
typedef void* (*boot_memset)(void* restrict s,uint32 c,uintn n);

/*端口位宽*/
#define PORT_WIDTH_8 0
#define PORT_WIDTH_16 1
#define PORT_WIDTH_32 2

/*
 * 端口读取。
 *
 * @param dest	输出内存。
 * @param width	读入位宽。
 * @param port	读入端口。
 * @param n		读取长度。
 *
 * @return 输出内存。
 */
typedef void* (*boot_readport)(void* restrict dest,uint8 width,uint16 port,uintn n);

/*
 * 端口写入。
 *
 * @param src	写入内存。
 * @param width	写入位宽。
 * @param port	写入端口。
 * @param n		写入长度。
 *
 * @return 写入内存。
 */
typedef void* (*boot_writeport)(const void* restrict src,uint8 width,uint16 port,uintn n);

/*
 * 基础模块函数表。
 */
typedef struct _boot_base_functions
{
	boot_ms_call_2 boot_ms_call_2; /*MS ABI两参数调用*/
	boot_ms_call_4 boot_ms_call_4; /*MS ABI四参数调用*/
	boot_memcpy boot_memcpy;	   /*内存复制*/
	boot_memset boot_memset;	   /*内存设置*/
	boot_readport boot_readport;   /*端口读取*/
	boot_writeport boot_writeport; /*端口写入*/
} boot_base_functions;

#endif /*__AOS_BOOT_BASE_H__*/