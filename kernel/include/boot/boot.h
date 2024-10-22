/*
 * 引导器总头文件。
 * @date 2024-10-17
 */
#ifndef __AOS_BOOT_H__
#define __AOS_BOOT_H__

typedef int aos_status;

typedef unsigned long long uint64;

extern int boot_code;

extern void print_bytes(const char* buf,uint64 size);

#endif