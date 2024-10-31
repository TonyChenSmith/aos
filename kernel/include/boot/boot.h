/*
 * aos.boot总头文件。
 * @date 2024-10-17
 */
#ifndef __AOS_BOOT_BOOT_H__
#define __AOS_BOOT_BOOT_H__

#include "type.h"
#include "param.h"

#define offset_of(type,field) (uintn)__builtin_offsetof(type,field)

extern void print_bytes(const char* buf,uint64 size);

extern void aos_cpu_hlt(void);

#endif /*__AOS_BOOT_BOOT_H__*/