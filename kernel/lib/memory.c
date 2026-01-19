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
    if(n==0||src==null||dest==null||src==dest)
    {
        return dest;
    }
    void* result=dest;
    if(n<64)
    {
        uint8* d=(uint8*)dest;
        const uint8* s=(uint8*)src;
        for(uintn index=0;index<n;index++)
        {
            d[index]=s[index];
        }
    }
    else
    {
        __asm__ volatile(
            "rep movsb"
            :"+D"(dest),"+S"(src),"+c"(n)
            :
            :"memory","cc"
        );
    }
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
    if(n==0||src==null||dest==null||src==dest)
    {
        return dest;
    }
    void* result=dest;
    if(dest>=src)
    {
        if(n<64)
        {
            uint8* d=(uint8*)dest;
            const uint8* s=(uint8*)src;
            for(uintn index=n;index>0;index--)
            {
                d[index-1]=s[index-1];
            }
        }
        else
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
        }
        return result;
    }
    else
    {
        if(n<64)
        {
            uint8* d=(uint8*)dest;
            const uint8* s=(uint8*)src;
            for(uintn index=0;index<n;index++)
            {
                d[index]=s[index];
            }
        }
        else
        {
            __asm__ volatile(
                "rep movsb"
                :"+D"(dest),"+S"(src),"+c"(n)
                :
                :"memory","cc"
            );
        }
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
    if(n==0||m==null)
    {
        return m;
    }
    void* result=m;
    if(n<64)
    {
        uint8* d=(uint8*)m;
        for(uintn index=0;index<n;index++)
        {
            d[index]=value;
        }
    }
    else
    {
        __asm__ volatile(
            "rep stosb"
            :"+D"(m),"+c"(n)
            :"a"(value)
            :"memory","cc"
        );
    }
    return result;
}

/**
 * 对内存a与内存b的前n个字节进行比较。
 * 
 * @param a 内存a。
 * @param b 内存b。
 * @param n 比较字节数。
 * 
 * @return 相等返回0，a<b返回负数，a>b返回正数。
 */
int8 memory_compare(const void* a,const void* b,uintn n)
{
    if(n==0||a==b)
    {
        return 0;
    }
    else if(a==null||b==null)
    {
        return a!=null?1:-1;
    }
    else
    {
        for(uintn index=0;index<n;index++)
        {
            if(((uint8*)a)[index]!=((uint8*)b)[index])
            {
                return ((uint8*)a)[index]>((uint8*)b)[index]?1:-1;
            }
        }
        return 0;
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
const void* memory_find(const void* m,uint8 byte,uintn n)
{
    if(n==0||m==null)
    {
        return null;
    }
    else
    {
        for(uintn index=0;index<n;index++)
        {
            if(((uint8*)m)[index]==byte)
            {
                return &((uint8*)m)[index];
            }
        }
        return null;
    }
}