/**
 * 内核字符串操作库函数。
 * @date 2025-01-05
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include <base.h>

/**
 * 获取8位字符串长度。
 * 
 * @param s 8位字符串。
 * 
 * @return 字符串到终结字符前的长度，不包含终结字符。
 */
uintn string_length8(const char8* s)
{
    uintn result=0;
    __asm__ volatile(
        "repne scasb\n\t"
        "not %%rcx\n\t"
        "dec %%rcx\n\t"
        :"+D"(s),"=c"(result)
        :"a"(0),"c"(UINTN_MAX)
        :"memory","cc"
    );
    return result;
}

/**
 * 获取16位字符串长度。
 * 
 * @param s 16位字符串。
 * 
 * @return 字符串到终结字符前的长度，不包含终结字符。
 */
uintn string_length16(const char16* s)
{
    uintn result=0;
    __asm__ volatile(
        "repne scasw\n\t"
        "not %%rcx\n\t"
        "dec %%rcx\n\t"
        :"+D"(s),"=c"(result)
        :"a"(0),"c"(UINTN_MAX)
        :"memory","cc"
    );
    return result;
}

/**
 * 获取32位字符串长度。
 * 
 * @param s 32位字符串。
 * 
 * @return 字符串到终结字符前的长度，不包含终结字符。
 */
uintn string_length32(const char32* s)
{
    uintn result=0;
    __asm__ volatile(
        "repne scasd\n\t"
        "not %%rcx\n\t"
        "dec %%rcx\n\t"
        :"+D"(s),"=c"(result)
        :"a"(0),"c"(UINTN_MAX)
        :"memory","cc"
    );
    return result;
}

/**
 * 获取8位字符串码点数。
 * 
 * @param s 8位字符串。
 * 
 * @return 字符串到终结字符前码点数目，非法编码返回0。
 */
uintn string_codepoint_count8(const char8* s)
{
    uintn index=0;
    uintn result=0;
    while(s[index]!=0)
    {
        if((s[index]&0x80)==0)
        {
            result++;
            index++;
        }
        else if((s[index]&0xE0)==0xC0)
        {
            if(s[index]>0xC1&&(s[index+1]&0xC0)==0x80)
            {
                result++;
                index+=2;
            }
            else 
            {
                return 0;            
            }
        }
        else if((s[index]&0xF0)==0xE0)
        {
            if((s[index+1]&0xC0)==0x80&&(s[index+2]&0xC0)==0x80&&!(s[index]==0xE0&&s[index+1]<0xA0)&&
                !(s[index]==0xED&&s[index+1]>0x9F))
            {
                result++;
                index+=3;
            }
            else 
            {
                return 0;            
            }
        }
        else if((s[index]&0xF8)==0xF0)
        {
            if(s[index]<0xF5&&(s[index+1]&0xC0)==0x80&&(s[index+2]&0xC0)==0x80&&(s[index+3]&0xC0)==0x80&&
                !(s[index]==0xF0&&s[index+1]<0x90)&&!(s[index]==0xF4&&s[index+1]>0x8F))
            {
                result++;
                index+=4;
            }
            else 
            {
                return 0;            
            }
        }
        else
        {
            return 0;
        }
    }
    return result;
}

/**
 * 获取16位字符串码点数。
 * 
 * @param s 16位字符串。
 * 
 * @return 字符串到终结字符前码点数目，非法编码返回0。
 */
uintn string_codepoint_count16(const char16* s)
{
    uintn index=0;
    uintn result=0;
    while(s[index]!=0)
    {
        if(s[index]<0xD800||s[index]>0xDFFF)
        {
            result++;
            index++;
        }
        else if((s[index]&0xFC00)==0xD800&&(s[index+1]&0xFC00)==0xDC00)
        {
            result++;
            index+=2;
        }
        else
        {
            return 0;
        }
    }
    return result;
}

/**
 * 获取32位字符串码点数。
 * 
 * @param s 32位字符串。
 * 
 * @return 字符串到终结字符前码点数目，非法编码返回0。
 */
uintn string_codepoint_count32(const char32* s)
{
    uintn index=0;
    uintn result=0;
    while(s[index]!=0)
    {
        if(s[index]<0xD800||(0xDFFF<s[index]&&s[index]<=0x10FFFF))
        {
            result++;
            index++;
        }
        else
        {
            return 0;
        }
    }
    return result;
}