/*
 * 基础内建函数。
 * @date 2024-12-12
 */
#ifndef __AOS_BASIC_BUILTIN_H__
#define __AOS_BASIC_BUILTIN_H__

#include <x86intrin.h>
#include "basic_type.h"

/*
 * 读取64位MSR内容，作为内联函数实现。
 * 
 * @param reg 寄存器地址。
 * 
 * @return 寄存器的值。
 */
static inline uint64 __attribute__((__always_inline__,__nodebug__)) builtin_rdmsr(const uint32 reg)
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
static inline void __attribute__((__always_inline__,__nodebug__)) builtin_wrmsr(const uint32 reg,const uint64 value)
{
	uint32 edx=(uint32)(value>>32),eax=(uint32)value;
	__asm__("wrmsr"::"d"(edx),"a"(eax),"c"(reg));
}

/*
 * 读取64位CR4内容，作为内联函数实现。
 * 
 * @return 寄存器的值。
 */
static inline uint64 __attribute__((__always_inline__,__nodebug__)) builtin_rdcr4(void)
{
	uint64 cr4;
	__asm__  __volatile__("mov {%%cr4,%0|%0,cr4}":"=r"(cr4)::"memory");
	return cr4;
}

#endif /*__AOS_BASIC__BUILTIN_H__*/