/**
 * 内核内存操作库函数。
 * @date 2025-01-04
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include <base.h>

/**
 * 将源内存复制前n个字节到目的内存。源内存与目的内在前n字节内有重叠时复制结果不保证正确。
 * 
 * @param dest 目的内存。
 * @param src  源内存。
 * @param n    复制字节数。
 * 
 * @return 返回目的内存的起始地址，便于进行链式处理。
 */
void* memory_copy(void* dest,const void* src,uintn n)
{
    void* result=dest;
    __asm__ volatile(
        "rep movsb"
        :"+D"(dest),"+S"(src),"+c"(n)
        :
        :"memory","cc"
    );
    return result;   
}

/**
 * 将源内存复制前n个字节到目的内存。这一函数允许复制时存在重叠情况。
 * 
 * @param dest 目的内存。
 * @param src  源内存。
 * @param n    复制字节数。
 * 
 * @return 返回目的内存的起始地址，便于进行链式处理。
 */
void* memory_move(void* dest,const void* src,uintn n)
{
    void* result=dest;
    if(n==0||src==dest)
    {
        return result;
    }
    else if(dest>=src)
    {
        dest=(void*)((uintn)dest+n-1);
        src=(void*)((uintn)src+n-1);

        __asm__ volatile(
            "std\n\t"
            "rep movsb\n\t"
            "cld"
            :"+D"(dest),"+S"(src),"+c"(n)
            :
            :"memory","cc"
        );
        return result;
    }
    else
    {
        __asm__ volatile(
            "rep movsb"
            :"+D"(dest),"+S"(src),"+c"(n)
            :
            :"memory","cc"
        );
        return result;
    }
}

/**
 * 将内存m前n个字节设置成固定值。
 * 
 * @param m     内存m。
 * @param value 设置值。
 * @param n     操作字节数。
 * 
 * @return 返回内存m的起始地址，便于进行链式处理。
 */
void* memory_set(void* m,uint8 value,uintn n)
{
    void* result=m;
    __asm__ volatile(
        "rep stosb"
        :"+D"(m),"+c"(n)
        :"a"(value)
        :"memory","cc"
    );
    return result;
}

/**
 * 对内存a与内存b的前n个字节进行比较。
 * 
 * @param a 内存a。
 * @param b 内存b。
 * @param n 比较字节数。
 * 
 * @return 相等返回真，不相等返回假。
 */
bool memory_compare(const void* a,const void* b,uintn n)
{
    if(n==0||a==b)
    {
        return true;
    }
    else
    {
        bool result=false;
        __asm__ volatile(
            "repe cmpsb\n\t"
            "sete %%al"
            :"=a"(result),"+D"(a),"+S"(b),"+c"(n)
            :
            :"memory","cc"
        );
        return result;
    }
}

/**
 * 在内存m的前n个字节寻找输入字节第一次出现的位置。
 * 
 * @param m    内存m。
 * @param byte 被比较字节。
 * @param n    操作字节数。
 * 
 * @return 找到返回所在地址，未找到返回空指针。
 */
void* memory_find(const void* m,uint8 byte,uintn n)
{
    if(n==0)
    {
        return null;
    }
    else
    {
        void* result=null;
        __asm__ volatile(
            "repne scasb\n\t"
            "jne 1f\n\t"
            "dec %%rdi\n\t"
            "mov %%rdi,%%rax\n\t"
            "jmp 2f\n\t"
            "1:xor %%rax,%%rax\n\t"
            "2:"
            :"=A"(result),"+D"(m),"+c"(n)
            :"a"(byte)
            :"memory","cc"
        );
        return result;
    }
}