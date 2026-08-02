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
static inline uint64 x86_read_msr(uint32 index)
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
static inline void x86_write_msr(uint32 index,uint64 value)
{
    __asm__ volatile("wrmsr"::"a"(value&UINT32_MAX),"d"(value>>32),"c"(index));
}

/**
 * 读取标志寄存器。
 * 
 * @return 标志寄存器的值。
 */
static inline uintn x86_read_flags(void)
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
static inline void x86_write_flags(uintn flags)
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
static inline uint8 x86_read_port8(uint16 port)
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
static inline uint16 x86_read_port16(uint16 port)
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
static inline uint32 x86_read_port32(uint16 port)
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
static inline void x86_write_port8(uint16 port, uint8 value)
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
static inline void x86_write_port16(uint16 port,uint16 value)
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
static inline void x86_write_port32(uint16 port,uint32 value)
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
static inline void x86_read_port8_repeat(uint16 port,void* addr,uintn count)
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
static inline void x86_read_port16_repeat(uint16 port,void* addr,uintn count)
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
static inline void x86_read_port32_repeat(uint16 port,void* addr,uintn count)
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
static inline void x86_write_port8_repeat(uint16 port,const void* addr,uintn count)
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
static inline void x86_write_port16_repeat(uint16 port,const void* addr,uintn count)
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
static inline void x86_write_port32_repeat(uint16 port,const void* addr,uintn count)
{
    __asm__ volatile("rep outsl":"+S"(addr),"+c"(count):"d"(port):"memory");
}

/**
 * 查询CPUID。不需要的输出参数传null即可。
 * 
 * @param leaf 主叶号。
 * @param eax  EAX输出。
 * @param ebx  EBX输出。
 * @param ecx  ECX输出。
 * @param edx  EDX输出。
 * 
 * @return 无返回值。
 */
static inline void x86_cpuid(uint32 leaf,uint32* eax,uint32* ebx,uint32* ecx,uint32* edx)
{
    uint32 a,b,c,d;
    __asm__ volatile("cpuid":"=a"(a),"=b"(b),"=c"(c),"=d"(d):"a"(leaf),"c"(0));
    if(eax)
    {
        *eax=a;
    }
    if(ebx)
    {
        *ebx=b;
    }
    if(ecx)
    {
        *ecx=c;
    }
    if(edx)
    {
        *edx=d;
    }
}

/**
 * 按子页号查询CPUID。不需要的输出参数传null即可。
 * 
 * @param leaf    主叶号。
 * @param subleaf 子叶号。
 * @param eax     EAX输出。
 * @param ebx     EBX输出。
 * @param ecx     ECX输出。
 * @param edx     EDX输出。
 * 
 * @return 无返回值。
 */
static inline void x86_cpuid_ex(uint32 leaf,uint32 subleaf,uint32* eax,uint32* ebx,uint32* ecx,uint32* edx)
{
    uint32 a,b,c,d;
    __asm__ volatile("cpuid":"=a"(a),"=b"(b),"=c"(c),"=d"(d):"a"(leaf),"c"(subleaf));
    if(eax)
    {
        *eax=a;
    }
    if(ebx)
    {
        *ebx=b;
    }
    if(ecx)
    {
        *ecx=c;
    }
    if(edx)
    {
        *edx=d;
    }
}

/**
 * 读取CR0寄存器。
 * 
 * @return CR0当前值。
 */
static inline uintn x86_read_cr0(void)
{
    uintn cr0;
    __asm__ volatile("mov %%cr0,%0":"=r"(cr0));
    return cr0;
}

/**
 * 写入CR0寄存器。
 * 
 * @param value 待写入值。
 * 
 * @return 无返回值。
 */
static inline void x86_write_cr0(uintn value)
{
    __asm__ volatile("mov %0,%%cr0"::"r"(value):"memory");
}

/**
 * 读取CR2寄存器。
 * 
 * @return CR2当前值。
 */
static inline uintn x86_read_cr2(void)
{
    uintn cr2;
    __asm__ volatile("mov %%cr2,%0":"=r"(cr2));
    return cr2;
}

/**
 * 写入CR2寄存器。
 * 
 * @param value 待写入值。
 * 
 * @return 无返回值。
 */
static inline void x86_write_cr2(uintn value)
{
    __asm__ volatile("mov %0,%%cr2"::"r"(value):"memory");
}

/**
 * 读取CR3寄存器。
 * 
 * @return CR3当前值。
 */
static inline uintn x86_read_cr3(void)
{
    uintn cr3;
    __asm__ volatile("mov %%cr3,%0":"=r"(cr3));
    return cr3;
}

/**
 * 写入CR3寄存器。
 * 
 * @param value 待写入值。
 * 
 * @return 无返回值。
 */
static inline void x86_write_cr3(uintn value)
{
    __asm__ volatile("mov %0,%%cr3"::"r"(value):"memory");
}

/**
 * 读取CR4寄存器。
 * 
 * @return CR4当前值。
 */
static inline uintn x86_read_cr4(void)
{
    uintn cr4;
    __asm__ volatile("mov %%cr4, %0" : "=r"(cr4));
    return cr4;
}

/**
 * 写入CR4寄存器。
 * 
 * @param value 待写入值。
 * 
 * @return 无返回值。
 */
static inline void x86_write_cr4(uintn value)
{
    __asm__ volatile("mov %0,%%cr4"::"r"(value):"memory");
}

/**
 * 读取CR8寄存器。
 * 
 * @return CR8当前值。
 */
static inline uintn x86_read_cr8(void)
{
    uintn cr8;
    __asm__ volatile("mov %%cr8,%0":"=r"(cr8));
    return cr8;
}

/**
 * 写入CR8寄存器。
 * 
 * @param value 待写入值。
 * 
 * @return 无返回值。
 */
static inline void x86_write_cr8(uintn value)
{
    __asm__ volatile("mov %0,%%cr8"::"r"(value):"memory");
}

/**
 * 读取MXCSR寄存器。
 * 
 * @return MXCSR的值。
 */
static inline uint32 x86_read_mxcsr(void)
{
    uint32 mxcsr;
    __asm__ volatile("stmxcsr %0":"=m"(mxcsr)::"memory");
    return mxcsr;
}

/**
 * 写入MXCSR寄存器。
 * 
 * @param value 待写入值。
 * 
 * @return 无返回值。
 */
static inline void x86_write_mxcsr(uint32 value)
{
    __asm__ volatile("ldmxcsr %0"::"m"(value):"memory");
}

/**
 * 读取XCR寄存器。
 * 
 * @param index XCR编号。
 * 
 * @return XCR当前值。
 */
static inline uint64 x86_read_xcr(uint32 index)
{
    uint32 low,high;
    __asm__ volatile("xgetbv":"=a"(low),"=d"(high):"c"(index));
    return ((uint64)high<<32)|low;
}

/**
 * 写入XCR寄存器。
 * 
 * @param index XCR编号。
 * @param value 待写入值。
 * 
 * @return 无返回值。
 */
static inline void x86_write_xcr(uint32 index,uint64 value)
{
    uint32 low=(uint32)(value&UINT32_MAX);
    uint32 high=(uint32)(value>>32);
    __asm__ volatile("xsetbv"::"a"(low),"d"(high),"c"(index));
}

/**
 * 读取XCR0寄存器。
 * 
 * @return XCR0当前值。
 */
static inline uint64 x86_read_xcr0(void)
{
    return x86_read_xcr(0);
}

/**
 * 写入XCR0寄存器。
 * 
 * @param value 待写入值。
 * 
 * @return 无返回值。
 */
static inline void x86_write_xcr0(uint64 value)
{
    x86_write_xcr(0,value);
}

#endif /*__AOS_KERNEL_SUPPORT_IO_H__*/