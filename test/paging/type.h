/*
 * aos内核基础类型定义。
 * @date 2024-10-23
 */
#ifndef __AOS_BASE_TYPE_H__
#define __AOS_BASE_TYPE_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <x86intrin.h>

/*缩减名称*/
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

/*地址位宽类型*/
typedef size_t uintn;

/*
 * 读取64位MSR内容，作为内联函数实现。
 * 
 * @param reg 寄存器地址。
 * 
 * @return 寄存器的值。
 */
static inline uint64 __attribute__((__always_inline__,__nodebug__)) _boot_rdmsr(const uint32 reg)
{
	uint32 edx,eax;
	__asm__("rdmsr":"=d"(edx),"=a"(eax):"c"(reg));
	return (((uint64)edx)<<32)|eax;
}

/*
 * 写入64位MSR内容，作为内联函数实现。
 * 
 * @param reg 寄存器地址。
 * @param val 写入值。
 */
static inline void __attribute__((__always_inline__,__nodebug__)) _boot_wrmsr(const uint32 reg,const uint64 value)
{
	uint32 edx=(uint32)(value>>32),eax=(uint32)value;
	__asm__("wrmsr"::"d"(edx),"a"(eax),"c"(reg));
}

/*
 * 读取64位CR4内容，作为内联函数实现。
 * 
 * @return 寄存器的值。
 */
static inline uint64 __attribute__((__always_inline__,__nodebug__)) _boot_rdcr4(void)
{
	uint32 cr4;
	__asm__  __volatile__("mov {%%cr4,%0|%0,cr3}":"=r"(cr4)::"memory");
	return cr4;
}

#endif /*__AOS_BASE_TYPE_H__*/