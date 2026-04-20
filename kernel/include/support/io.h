/**
 * 内核CPU输入输出函数。
 * @date 2026-04-20
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_KERNEL_SUPPORT_IO_H__
#define __AOS_KERNEL_SUPPORT_IO_H__

#include "const.h"

/**
 * 读取x86平台64位MSR。
 * 
 * @param index 寄存器地址。
 * 
 * @return 读取寄存器的值。
 */
static inline uint64 read_msr(uint32 index)
{
    uint32 low,high;
    __asm__ volatile("rdmsr":"=a"(low),"=d"(high):"c"(index));
    return ((uint64)high<<32)|low;
}

/**
 * 写入x86平台64位MSR。
 * 
 * @param index 寄存器地址。
 * @param value 待写入寄存器的值。
 * 
 * @return 无返回值。
 */
static inline void write_msr(uint32 index,uint64 value)
{
    __asm__ volatile("wrmsr"::"a"(value&UINT32_MAX),"d"(value>>32),"c"(index));
}

/**
 * 读取标志寄存器。
 * 
 * @return 标志寄存器的值。
 */
static inline uintn read_flags(void)
{
    uintn flags;
    __asm__ volatile("pushfq\n\t"
                     "pop %0"
                     :"=r"(flags)
                     :
                     :"memory");
    return flags;
}
/**
 * 写入标志寄存器。
 * 
 * @param flags 待写入标志寄存器的值。
 * 
 * @return 无返回值。
 */
static inline void write_flags(uintn flags)
{
    __asm__ volatile("push %0\n\t"
                     "popfq"
                     :
                     :"r"(flags)
                     :"memory");
}

/**
 * 从端口读取一个8位无符号整数。
 * 
 * @param port 端口地址。
 * 
 * @return 读取的8位无符号整数。
 */
static inline uint8 read_port8(uint16 port)
{
    uint8 value;
    __asm__ volatile("inb %w1,%0":"=a"(value):"Nd"(port));
    return value;
}

/**
 * 从端口读取一个16位无符号整数。
 * 
 * @param port 端口地址。
 * 
 * @return 读取的16位无符号整数。
 */
static inline uint16 read_port16(uint16 port)
{
    uint16 value;
    __asm__ volatile("inw %w1,%0":"=a"(value):"Nd"(port));
    return value;
}

/**
 * 从端口读取一个32位无符号整数。
 * 
 * @param port 端口地址。
 * 
 * @return 读取的32位无符号整数。
 */
static inline uint32 read_port32(uint16 port)
{
    uint32 value;
    __asm__ volatile("inl %w1,%0":"=a"(value):"Nd"(port));
    return value;
}

/**
 * 向端口写入一个8位无符号整数。
 * 
 * @param port  端口地址。
 * @param value 8位无符号整数值。
 * 
 * @return 无返回值。
 */
static inline void write_port8(uint16 port, uint8 value)
{
    __asm__ volatile("outb %0,%w1"::"a"(value),"Nd"(port));
}

/**
 * 向端口写入一个16位无符号整数。
 * 
 * @param port  端口地址。
 * @param value 16位无符号整数。
 * 
 * @return 无返回值。
 */
static inline void write_port16(uint16 port,uint16 value)
{
    __asm__ volatile("outw %0,%w1"::"a"(value),"Nd"(port));
}

/**
 * 向端口写入一个32位无符号整数。
 * 
 * @param port  端口地址。
 * @param value 32位无符号整数。
 * 
 * @return 无返回值。
 */
static inline void write_port32(uint16 port,uint32 value)
{
    __asm__ volatile("outl %0,%w1"::"a"(value),"Nd"(port));
}

/**
 * 从端口连续读取多个8位无符号整数。
 * 
 * @param port  端口地址。
 * @param addr  目标起始地址。
 * @param count 读取个数。
 * 
 * @return 无返回值。
 */
static inline void read_port8_repeat(uint16 port,void* addr,uintn count)
{
    __asm__ volatile("rep insb":"+D"(addr),"+c"(count):"d"(port):"memory");
}

/**
 * 从端口连续读取多个16位无符号整数。
 * 
 * @param port  端口地址。
 * @param addr  目标起始地址。
 * @param count 读取个数。
 * 
 * @return 无返回值。
 */
static inline void read_port16_repeat(uint16 port,void* addr,uintn count)
{
    __asm__ volatile("rep insw":"+D"(addr),"+c"(count):"d"(port):"memory");
}

/**
 * 从端口连续读取多个32位无符号整数。
 * 
 * @param port  端口地址。
 * @param addr  目标起始地址。
 * @param count 读取个数。
 * 
 * @return 无返回值。
 */
static inline void read_port32_repeat(uint16 port,void* addr,uintn count)
{
    __asm__ volatile("rep insl":"+D"(addr),"+c"(count):"d"(port):"memory");
}

/**
 * 向端口连续写入多个8位无符号整数。
 * 
 * @param port  端口地址。
 * @param addr  源起始地址。
 * @param count 写入个数。
 * 
 * @return 无返回值。
 */
static inline void write_port8_repeat(uint16 port,const void* addr,uintn count)
{
    __asm__ volatile("rep outsb":"+S"(addr),"+c"(count):"d"(port):"memory");
}
/**
 * 向端口连续写入多个16位无符号整数。
 * 
 * @param port  端口地址。
 * @param addr  源起始地址。
 * @param count 写入个数。
 * 
 * @return 无返回值。
 */
static inline void write_port16_repeat(uint16 port,const void* addr,uintn count)
{
    __asm__ volatile("rep outsw":"+S"(addr),"+c"(count):"d"(port):"memory");
}
/**
 * 向端口连续写入多个32位无符号整数。
 * 
 * @param port  端口地址。
 * @param addr  源起始地址。
 * @param count 写入个数。
 * 
 * @return 无返回值。
 */
static inline void write_port32_repeat(uint16 port,const void* addr,uintn count)
{
    __asm__ volatile("rep outsl":"+S"(addr),"+c"(count):"d"(port):"memory");
}

#endif /*__AOS_KERNEL_SUPPORT_IO_H__*/