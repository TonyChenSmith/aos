/**
 * 内核字符串操作库函数。
 * @date 2026-01-05
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include <support/kdef.h>
#include <support/ktype.h>
#include <support/util.h>

/**
 * 获取8位字符串长度。
 * 
 * @param s 8位字符串。
 * 
 * @return 字符串到终结字符前的长度，不包含终结字符。
 */
uintn string_length8(const char8* s)
{
    if(s==null)
    {
        return 0;
    }
    const char8* p=s;
    while(*p!=0)
    {
        p++;
    }
    return p-s;
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
    if(s==null)
    {
        return 0;
    }
    const char16* p=s;
    while(*p!=0)
    {
        p++;
    }
    return p-s;
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
    if(s==null)
    {
        return 0;
    }
    const char32* p=s;
    while(*p!=0)
    {
        p++;
    }
    return p-s;
}

/**
 * 获取8位字符串码点数。
 * 
 * @param s 8位字符串。
 * 
 * @return 字符串到终结字符前码点数目，无效编码返回0。
 */
uintn string_codepoint_count8(const char8* s)
{
    if(s==null)
    {
        return 0;
    }
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
 * @return 字符串到终结字符前码点数目，无效编码返回0。
 */
uintn string_codepoint_count16(const char16* s)
{
    if(s==null)
    {
        return 0;
    }
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
 * @return 字符串到终结字符前码点数目，无效编码返回0。
 */
uintn string_codepoint_count32(const char32* s)
{
    if(s==null)
    {
        return 0;
    }
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

/**
 * 获取8位字符串所需大小。
 * 
 * @param s 8位字符串。
 * 
 * @return 8位字符串所需空间大小，无效编码返回0。
 */
static uintn string_char8_required8(const char8* s)
{
    if(s==null)
    {
        return 0;
    }
    uintn index=0;
    uintn result=1;
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
                result+=2;
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
                result+=3;
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
                result+=4;
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
 * 获取16位字符串转码为8位字符串所需大小。
 * 
 * @param s 16位字符串。
 * 
 * @return 8位字符串所需空间大小，无效编码返回0。
 */
static uintn string_char8_required16(const char16* s)
{
    if(s==null)
    {
        return 0;
    }
    uintn index=0;
    uintn result=1;
    while(s[index]!=0)
    {
        if(s[index]<0xD800||s[index]>0xDFFF)
        {
            if(s[index]<0x80)
            {
                result++;
                index++;
            }
            else if(s[index]<0x800)
            {
                result+=2;
                index++;
            }
            else
            {
                result+=3;
                index++;
            }
        }
        else if((s[index]&0xFC00)==0xD800&&(s[index+1]&0xFC00)==0xDC00)
        {
            result+=4;
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
 * 获取32位字符串转码为8位字符串所需大小。
 * 
 * @param s 32位字符串。
 * 
 * @return 8位字符串所需空间大小，无效编码返回0。
 */
static uintn string_char8_required32(const char32* s)
{
    if(s==null)
    {
        return 0;
    }
    uintn index=0;
    uintn result=1;
    while(s[index]!=0)
    {
        if(s[index]<0xD800||(0xDFFF<s[index]&&s[index]<=0x10FFFF))
        {
            if(s[index]<0x80)
            {
                result++;
                index++;
            }
            else if(s[index]<0x800)
            {
                result+=2;
                index++;
            }
            else if(s[index]<0x10000)
            {
                result+=3;
                index++;
            }
            else
            {
                result+=4;
                index++;
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
 * 获取8位字符串转码为16位字符串所需大小。
 * 
 * @param s 8位字符串。
 * 
 * @return 16位字符串所需空间大小，无效编码返回0。
 */
static uintn string_char16_required8(const char8* s)
{
    if(s==null)
    {
        return 0;
    }
    uintn index=0;
    uintn result=2;
    while(s[index]!=0)
    {
        if((s[index]&0x80)==0)
        {
            result+=2;
            index++;
        }
        else if((s[index]&0xE0)==0xC0)
        {
            if(s[index]>0xC1&&(s[index+1]&0xC0)==0x80)
            {
                result+=2;
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
                result+=2;
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
                result+=4;
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
 * 获取16位字符串所需大小。
 * 
 * @param s 16位字符串。
 * 
 * @return 16位字符串所需空间大小，无效编码返回0。
 */
static uintn string_char16_required16(const char16* s)
{
    if(s==null)
    {
        return 0;
    }
    uintn index=0;
    uintn result=2;
    while(s[index]!=0)
    {
        if(s[index]<0xD800||s[index]>0xDFFF)
        {
            result+=2;
            index++;
        }
        else if((s[index]&0xFC00)==0xD800&&(s[index+1]&0xFC00)==0xDC00)
        {
            result+=4;
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
 * 获取32位字符串转码为16位字符串所需大小。
 * 
 * @param s 32位字符串。
 * 
 * @return 16位字符串所需空间大小，无效编码返回0。
 */
static uintn string_char16_required32(const char32* s)
{
    if(s==null)
    {
        return 0;
    }
    uintn index=0;
    uintn result=2;
    while(s[index]!=0)
    {
        if(s[index]<0xD800||(0xDFFF<s[index]&&s[index]<=0x10FFFF))
        {
            if(s[index]<0x10000)
            {
                result+=2;
                index++;
            }
            else
            {
                result+=4;
                index++;
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
 * 获取8位字符串转码为32位字符串所需大小。
 * 
 * @param s 8位字符串。
 * 
 * @return 32位字符串所需空间大小，无效编码返回0。
 */
static uintn string_char32_required8(const char8* s)
{
    if(s==null)
    {
        return 0;
    }
    uintn index=0;
    uintn result=4;
    while(s[index]!=0)
    {
        if((s[index]&0x80)==0)
        {
            result+=4;
            index++;
        }
        else if((s[index]&0xE0)==0xC0)
        {
            if(s[index]>0xC1&&(s[index+1]&0xC0)==0x80)
            {
                result+=4;
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
                result+=4;
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
                result+=4;
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
 * 获取16位字符串转码为32位字符串所需大小。
 * 
 * @param s 16位字符串。
 * 
 * @return 32位字符串所需空间大小，无效编码返回0。
 */
static uintn string_char32_required16(const char16* s)
{
    if(s==null)
    {
        return 0;
    }
    uintn index=0;
    uintn result=4;
    while(s[index]!=0)
    {
        if(s[index]<0xD800||s[index]>0xDFFF)
        {
            result+=4;
            index++;
        }
        else if((s[index]&0xFC00)==0xD800&&(s[index+1]&0xFC00)==0xDC00)
        {
            result+=4;
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
 * 获取32位字符串所需大小。
 * 
 * @param s 32位字符串。
 * 
 * @return 32位字符串所需空间大小，无效编码返回0。
 */
static uintn string_char32_required32(const char32* s)
{
    if(s==null)
    {
        return 0;
    }
    uintn index=0;
    uintn result=4;
    while(s[index]!=0)
    {
        if(s[index]<0xD800||(0xDFFF<s[index]&&s[index]<=0x10FFFF))
        {
            result+=4;
            index++;
        }
        else
        {
            return 0;
        }
    }
    return result;
}

/**
 * 将8位字符串转换成等价有效8位字符串。
 * 
 * @param dest 目标内存空间。
 * @param src  8位字符串。
 * @param size 目标字符串预留空间大小。
 * 
 * @return 已使用空间大小，当预留大小不足时返回期望大小，无效编码返回0。
 */
uintn string_convert8_char8(char8* dest,const char8* src,uintn size)
{
    if(src==null||dest==null)
    {
        return 0;
    }
    uintn expect=string_char8_required8(src);
    if(expect==0||expect>size)
    {
        return expect;
    }
    for(uintn index=0;index<expect;index++)
    {
        dest[index]=src[index];
    }
    return expect;
}

/**
 * 将16位字符串转换成8位字符串。
 * 
 * @param dest 目标内存空间。
 * @param src  16位字符串。
 * @param size 目标字符串预留空间大小。
 * 
 * @return 已使用空间大小，当预留大小不足时返回期望大小，无效编码返回0。
 */
uintn string_convert8_char16(char8* dest,const char16* src,uintn size)
{
    if(src==null||dest==null)
    {
        return 0;
    }
    uintn expect=string_char8_required16(src);
    if(expect==0||expect>size)
    {
        return expect;
    }
    uintn index=0;
    uintn result=0;
    while(src[index]!=0)
    {
        if(src[index]<0xD800||src[index]>0xDFFF)
        {
            if(src[index]<0x80)
            {
                dest[result++]=(char8)src[index++];
            }
            else if(src[index]<0x800)
            {
                dest[result++]=(char8)(0xC0|(src[index]>>6));
                dest[result++]=(char8)(0x80|(src[index++]&0x3F));
            }
            else
            {
                dest[result++]=(char8)(0xE0|(src[index]>>12));
                dest[result++]=(char8)(0x80|((src[index]>>6)&0x3F));
                dest[result++]=(char8)(0x80|(src[index++]&0x3F));
            }
        }
        else
        {
            char32 cp=(((char32)(src[index++]&0x3FF))<<10);
            cp|=src[index++]&0x3FF;
            cp+=0x10000;
            dest[result++]=(char8)(0xF0|(cp>>18));
            dest[result++]=(char8)(0x80|((cp>>12)&0x3F));
            dest[result++]=(char8)(0x80|((cp>>6)&0x3F));
            dest[result++]=(char8)(0x80|(cp&0x3F));
        }
    }
    dest[result++]=0;
    return result;
}

/**
 * 将32位字符串转换成8位字符串。
 * 
 * @param dest 目标内存空间。
 * @param src  32位字符串。
 * @param size 目标字符串预留空间大小。
 * 
 * @return 已使用空间大小，当预留大小不足时返回期望大小，无效编码返回0。
 */
uintn string_convert8_char32(char8* dest,const char32* src,uintn size)
{
    if(src==null||dest==null)
    {
        return 0;
    }
    uintn expect=string_char8_required32(src);
    if(expect==0||expect>size)
    {
        return expect;
    }
    uintn index=0;
    uintn result=0;
    while(src[index]!=0)
    {
        if(src[index]<0x80)
        {
            dest[result++]=(char8)src[index++];
        }
        else if(src[index]<0x800)
        {
            dest[result++]=(char8)(0xC0|(src[index]>>6));
            dest[result++]=(char8)(0x80|(src[index++]&0x3F));
        }
        else if(src[index]<0x10000)
        {
            dest[result++]=(char8)(0xE0|(src[index]>>12));
            dest[result++]=(char8)(0x80|((src[index]>>6)&0x3F));
            dest[result++]=(char8)(0x80|(src[index++]&0x3F));
        }
        else
        {
            dest[result++]=(char8)(0xF0|(src[index]>>18));
            dest[result++]=(char8)(0x80|((src[index]>>12)&0x3F));
            dest[result++]=(char8)(0x80|((src[index]>>6)&0x3F));
            dest[result++]=(char8)(0x80|(src[index++]&0x3F));
        }
    }
    dest[result++]=0;
    return result;
}

/**
 * 将8位字符串转换成16位字符串。
 * 
 * @param dest 目标内存空间。
 * @param src  8位字符串。
 * @param size 目标字符串预留空间大小。
 * 
 * @return 已使用空间大小，当预留大小不足时返回期望大小，无效编码返回0。
 */
uintn string_convert16_char8(char16* dest,const char8* src,uintn size)
{
    if(src==null||dest==null)
    {
        return 0;
    }
    uintn expect=string_char16_required8(src);
    if(expect==0||expect>size)
    {
        return expect;
    }
    uintn index=0;
    uintn result=0;
    while(src[index]!=0)
    {
        if((src[index]&0x80)==0)
        {
            dest[result++]=src[index++];
        }
        else if((src[index]&0xE0)==0xC0)
        {
            dest[result]=((char16)(src[index++]&0x1F))<<6;
            dest[result++]|=src[index++]&0x3F;
        }
        else if((src[index]&0xF0)==0xE0)
        {
            dest[result]=((char16)(src[index++]&0xF))<<12;
            dest[result]|=((char16)(src[index++]&0x3F))<<6;
            dest[result++]|=src[index++]&0x3F;
        }
        else
        {
            char32 cp=((char32)(src[index++]&0x7))<<18;
            cp|=((char32)(src[index++]&0x3F))<<12;
            cp|=((char32)(src[index++]&0x3F))<<6;
            cp|=src[index++]&0x3F;
            cp-=0x10000;
            dest[result++]=(char16)(0xDB00|(cp>>10));
            dest[result++]=(char16)(0xDC00|(cp&0x3FF));
        }
    }
    dest[result++]=0;
    return result<<1;
}

/**
 * 将16位字符串转换成等价有效16位字符串。
 * 
 * @param dest 目标内存空间。
 * @param src  16位字符串。
 * @param size 目标字符串预留空间大小。
 * 
 * @return 已使用空间大小，当预留大小不足时返回期望大小，无效编码返回0。
 */
uintn string_convert16_char16(char16* dest,const char16* src,uintn size)
{
    if(src==null||dest==null)
    {
        return 0;
    }
    uintn expect=string_char16_required16(src);
    if(expect==0||expect>size)
    {
        return expect;
    }
    for(uintn index=0;index<expect;index++)
    {
        dest[index]=src[index];
    }
    return expect;
}

/**
 * 将32位字符串转换成16位字符串。
 * 
 * @param dest 目标内存空间。
 * @param src  32位字符串。
 * @param size 目标字符串预留空间大小。
 * 
 * @return 已使用空间大小，当预留大小不足时返回期望大小，无效编码返回0。
 */
uintn string_convert16_char32(char16* dest,const char32* src,uintn size)
{
    if(src==null||dest==null)
    {
        return 0;
    }
    uintn expect=string_char16_required32(src);
    if(expect==0||expect>size)
    {
        return expect;
    }
    uintn index=0;
    uintn result=0;
    while(src[index]!=0)
    {
        if(src[index]<0x10000)
        {
            dest[result++]=(char16)src[index++];
        }
        else
        {
            dest[result++]=(char16)(0xDB00|(src[index]>>10));
            dest[result++]=(char16)(0xDC00|(src[index++]&0x3FF));
        }
    }
    dest[result++]=0;
    return result<<1;
}

/**
 * 将8位字符串转换成32位字符串。
 * 
 * @param dest 目标内存空间。
 * @param src  8位字符串。
 * @param size 目标字符串预留空间大小。
 * 
 * @return 已使用空间大小，当预留大小不足时返回期望大小，无效编码返回0。
 */
uintn string_convert32_char8(char32* dest,const char8* src,uintn size)
{
    if(src==null||dest==null)
    {
        return 0;
    }
    uintn expect=string_char32_required8(src);
    if(expect==0||expect>size)
    {
        return expect;
    }
    uintn index=0;
    uintn result=0;
    while(src[index]!=0)
    {
        if((src[index]&0x80)==0)
        {
            dest[result++]=src[index++];
        }
        else if((src[index]&0xE0)==0xC0)
        {
            dest[result]=((char32)(src[index++]&0x1F))<<6;
            dest[result++]|=src[index++]&0x3F;
        }
        else if((src[index]&0xF0)==0xE0)
        {
            dest[result]=((char32)(src[index++]&0xF))<<12;
            dest[result]|=((char32)(src[index++]&0x3F))<<6;
            dest[result++]|=src[index++]&0x3F;
        }
        else
        {
            dest[result]=((char32)(src[index++]&0x7))<<18;
            dest[result]|=((char32)(src[index++]&0x3F))<<12;
            dest[result]|=((char32)(src[index++]&0x3F))<<6;
            dest[result++]|=src[index++]&0x3F;
        }
    }
    dest[result]=0;
    return (result+1)<<2;
}

/**
 * 将16位字符串转换成32位字符串。
 * 
 * @param dest 目标内存空间。
 * @param src  16位字符串。
 * @param size 目标字符串预留空间大小。
 * 
 * @return 已使用空间大小，当预留大小不足时返回期望大小，无效编码返回0。
 */
uintn string_convert32_char16(char32* dest,const char16* src,uintn size)
{
    if(src==null||dest==null)
    {
        return 0;
    }
    uintn expect=string_char32_required16(src);
    if(expect==0||expect>size)
    {
        return expect;
    }
    uintn index=0;
    uintn result=0;
    while(src[index]!=0)
    {
        if(src[index]<0xD800||src[index]>0xDFFF)
        {
            dest[result++]=src[index++];
        }
        else
        {
            dest[result]=((char32)(src[index++]&0x3FF))<<10;
            dest[result]|=(src[index++]&0x3FF);
            dest[result++]+=0x10000;
        }
    }
    dest[result]=0;
    return (result+1)<<2;
}

/**
 * 将32位字符串转换成等价有效32位字符串。
 * 
 * @param dest 目标内存空间。
 * @param src  32位字符串。
 * @param size 目标字符串预留空间大小。
 * 
 * @return 已使用空间大小，当预留大小不足时返回期望大小，无效编码返回0。
 */
uintn string_convert32_char32(char32* dest,const char32* src,uintn size)
{
    if(src==null||dest==null)
    {
        return 0;
    }
    uintn expect=string_char32_required32(src);
    if(expect==0||expect>size)
    {
        return expect;
    }
    for(uintn index=0;index<expect;index++)
    {
        dest[index]=src[index];
    }
    return expect;
}

/**
 * 连接8位字符串b到字符串a后面，需要字符串a的可用空间大小。
 * 
 * @param a    字符串a。
 * @param b    字符串b。
 * @param size 包含字符串a预留空间大小。
 * 
 * @return 总使用空间大小，当预留大小不足时返回期望大小。
 */
uintn string_concat8(char8* a,const char8* b,uintn size)
{
    if(a==null||b==null)
    {
        return 0;
    }
    uintn len_a=string_length8(a);
    uintn len_b=string_length8(b);
    uintn expect=len_a+len_b+1;
    if(expect>size)
    {
        return expect;
    }
    a=&a[len_a];
    for(uintn index=0;index<len_b+1;index++)
    {
        a[index]=b[index];
    }
    return expect;
}

/**
 * 连接16位字符串b到字符串a后面，需要字符串a的可用空间大小。
 * 
 * @param a    字符串a。
 * @param b    字符串b。
 * @param size 包含字符串a预留空间大小。
 * 
 * @return 总使用空间大小，当预留大小不足时返回期望大小。
 */
uintn string_concat16(char16* a,const char16* b,uintn size)
{
    if(a==null||b==null)
    {
        return 0;
    }
    uintn len_a=string_length16(a);
    uintn len_b=string_length16(b);
    uintn expect=len_a+len_b+1;
    if(expect>size)
    {
        return expect;
    }
    a=&a[len_a];
    for(uintn index=0;index<len_b+1;index++)
    {
        a[index]=b[index];
    }
    return expect;
}

/**
 * 连接32位字符串b到字符串a后面，需要字符串a的可用空间大小。
 * 
 * @param a    字符串a。
 * @param b    字符串b。
 * @param size 包含字符串a预留空间大小。
 * 
 * @return 总使用空间大小，当预留大小不足时返回期望大小。
 */
uintn string_concat32(char32* a,const char32* b,uintn size)
{
    if(a==null||b==null)
    {
        return 0;
    }
    uintn len_a=string_length32(a);
    uintn len_b=string_length32(b);
    uintn expect=len_a+len_b+1;
    if(expect>size)
    {
        return expect;
    }
    a=&a[len_a];
    for(uintn index=0;index<len_b+1;index++)
    {
        a[index]=b[index];
    }
    return expect;
}

/**
 * 复制8位的源字符串到目标字符串，需要源字符串的可用空间大小。
 * 
 * @param dest 目标字符串。
 * @param src  源字符串。
 * @param size 目标字符串可用空间大小。
 * 
 * @return 总使用空间大小，当预留大小不足时返回期望大小。
 */
uintn string_copy8(char8* dest,const char8* src,uintn size)
{
    if(dest==null||src==null)
    {
        return 0;
    }
    uintn expect=string_length8(src)+1;
    if(expect>size)
    {
        return expect;
    }
    for(uintn index=0;index<expect;index++)
    {
        dest[index]=src[index];
    }
    return expect;
}

/**
 * 复制16位的源字符串到目标字符串，需要源字符串的可用空间大小。
 * 
 * @param dest 目标字符串。
 * @param src  源字符串。
 * @param size 目标字符串可用空间大小。
 * 
 * @return 总使用空间大小，当预留大小不足时返回期望大小。
 */
uintn string_copy16(char16* dest,const char16* src,uintn size)
{
    if(dest==null||src==null)
    {
        return 0;
    }
    uintn expect=string_length16(src)+1;
    if(expect>size)
    {
        return expect;
    }
    for(uintn index=0;index<expect;index++)
    {
        dest[index]=src[index];
    }
    return expect;
}

/**
 * 复制32位的源字符串到目标字符串，需要源字符串的可用空间大小。
 * 
 * @param dest 目标字符串。
 * @param src  源字符串。
 * @param size 目标字符串可用空间大小。
 * 
 * @return 总使用空间大小，当预留大小不足时返回期望大小。
 */
uintn string_copy32(char32* dest,const char32* src,uintn size)
{
    if(dest==null||src==null)
    {
        return 0;
    }
    uintn expect=string_length32(src)+1;
    if(expect>size)
    {
        return expect;
    }
    for(uintn index=0;index<expect;index++)
    {
        dest[index]=src[index];
    }
    return expect;
}

/**
 * 搜索8位源字符串中从前往后第一次出现目标字符串的位置。
 * 
 * @param src    源字符串。
 * @param target 目标字符串。
 * 
 * @return 对应位置指针，或返回空值。
 */
const char8* string_find8(const char8* src,const char8* target)
{
    if(src==null||target==null)
    {
        return null;
    }
    uintn len_s=string_length8(src);
    uintn len_t=string_length8(target);
    if(len_s<len_t||len_t==0)
    {
        return null;
    }
    uintn scan=len_s-len_t;
    for(uintn index=0;index<=scan;index++)
    {
        const char8* result=&src[index];
        for(uintn i=0;i<len_t;i++)
        {
            if(result[i]!=target[i])
            {
                result=null;
                break;
            }
        }
        if(result!=null)
        {
            return result;
        }
    }
    return null;
}

/**
 * 搜索16位源字符串中从前往后第一次出现目标字符串的位置。
 * 
 * @param src    源字符串。
 * @param target 目标字符串。
 * 
 * @return 对应位置指针，或返回空值。
 */
const char16* string_find16(const char16* src,const char16* target)
{
    if(src==null||target==null)
    {
        return null;
    }
    uintn len_s=string_length16(src);
    uintn len_t=string_length16(target);
    if(len_s<len_t||len_t==0)
    {
        return null;
    }
    uintn scan=len_s-len_t;
    for(uintn index=0;index<=scan;index++)
    {
        const char16* result=&src[index];
        for(uintn i=0;i<len_t;i++)
        {
            if(result[i]!=target[i])
            {
                result=null;
                break;
            }
        }
        if(result!=null)
        {
            return result;
        }
    }
    return null;
}

/**
 * 搜索32位源字符串中从前往后第一次出现目标字符串的位置。
 * 
 * @param src    源字符串。
 * @param target 目标字符串。
 * 
 * @return 对应位置指针，或返回空值。
 */
const char32* string_find32(const char32* src,const char32* target)
{
    if(src==null||target==null)
    {
        return null;
    }
    uintn len_s=string_length32(src);
    uintn len_t=string_length32(target);
    if(len_s<len_t||len_t==0)
    {
        return null;
    }
    uintn scan=len_s-len_t;
    for(uintn index=0;index<=scan;index++)
    {
        const char32* result=&src[index];
        for(uintn i=0;i<len_t;i++)
        {
            if(result[i]!=target[i])
            {
                result=null;
                break;
            }
        }
        if(result!=null)
        {
            return result;
        }
    }
    return null;
}

/**
 * 搜索8位源字符串中从后往前第一次出现目标字符串的位置。
 * 
 * @param src    源字符串。
 * @param target 目标字符串。
 * 
 * @return 对应位置指针，或返回空值。
 */
const char8* string_reverse_find8(const char8* src,const char8* target)
{
    if(src==null||target==null)
    {
        return null;
    }
    uintn len_s=string_length8(src);
    uintn len_t=string_length8(target);
    if(len_s<len_t||len_t==0)
    {
        return null;
    }
    uintn scan=len_s-len_t;
    for(uintn index=scan+1;index>0;index--)
    {
        const char8* result=&src[index-1];
        for(uintn i=0;i<len_t;i++)
        {
            if(result[i]!=target[i])
            {
                result=null;
                break;
            }
        }
        if(result!=null)
        {
            return result;
        }
    }
    return null;
}

/**
 * 搜索16位源字符串中从后往前第一次出现目标字符串的位置。
 * 
 * @param src    源字符串。
 * @param target 目标字符串。
 * 
 * @return 对应位置指针，或返回空值。
 */
const char16* string_reverse_find16(const char16* src,const char16* target)
{
    if(src==null||target==null)
    {
        return null;
    }
    uintn len_s=string_length16(src);
    uintn len_t=string_length16(target);
    if(len_s<len_t||len_t==0)
    {
        return null;
    }
    uintn scan=len_s-len_t;
    for(uintn index=scan+1;index>0;index--)
    {
        const char16* result=&src[index-1];
        for(uintn i=0;i<len_t;i++)
        {
            if(result[i]!=target[i])
            {
                result=null;
                break;
            }
        }
        if(result!=null)
        {
            return result;
        }
    }
    return null;
}

/**
 * 搜索32位源字符串中从后往前第一次出现目标字符串的位置。
 * 
 * @param src    源字符串。
 * @param target 目标字符串。
 * 
 * @return 对应位置指针，或返回空值。
 */
const char32* string_reverse_find32(const char32* src,const char32* target)
{
    if(src==null||target==null)
    {
        return null;
    }
    uintn len_s=string_length32(src);
    uintn len_t=string_length32(target);
    if(len_s<len_t||len_t==0)
    {
        return null;
    }
    uintn scan=len_s-len_t;
    for(uintn index=scan+1;index>0;index--)
    {
        const char32* result=&src[index-1];
        for(uintn i=0;i<len_t;i++)
        {
            if(result[i]!=target[i])
            {
                result=null;
                break;
            }
        }
        if(result!=null)
        {
            return result;
        }
    }
    return null;
}

/**
 * 搜索8位字符串中从前往后第一次出现目标字符的位置。
 * 
 * @param s 字符串。
 * @param c 目标字符。
 * 
 * @return 对应位置指针，或返回空值。
 */
const char8* string_find_char8(const char8* s,char8 c)
{
    if(s==null)
    {
        return null;
    }
    while(*s!=0)
    {
        if(*s==c)
        {
            return s;
        }
        else
        {
            s++;
        }
    }
    return null;
}

/**
 * 搜索16位字符串中从前往后第一次出现目标字符的位置。
 * 
 * @param s 字符串。
 * @param c 目标字符。
 * 
 * @return 对应位置指针，或返回空值。
 */
const char16* string_find_char16(const char16* s,char16 c)
{
    if(s==null)
    {
        return null;
    }
    while(*s!=0)
    {
        if(*s==c)
        {
            return s;
        }
        else
        {
            s++;
        }
    }
    return null;
}

/**
 * 搜索32位字符串中从前往后第一次出现目标字符的位置。
 * 
 * @param s 字符串。
 * @param c 目标字符。
 * 
 * @return 对应位置指针，或返回空值。
 */
const char32* string_find_char32(const char32* s,char32 c)
{
    if(s==null)
    {
        return null;
    }
    while(*s!=0)
    {
        if(*s==c)
        {
            return s;
        }
        else
        {
            s++;
        }
    }
    return null;
}

/**
 * 搜索8位字符串中从后往前第一次出现目标字符的位置。
 * 
 * @param s 字符串。
 * @param c 目标字符。
 * 
 * @return 对应位置指针，或返回空值。
 */
const char8* string_reverse_find_char8(const char8* s,char8 c)
{
    if(s==null)
    {
        return null;
    }
    const char8* result=null;
    while(*s!=0)
    {
        if(*s==c)
        {
            result=s;
        }
        s++;
    }
    return result;
}

/**
 * 搜索16位字符串中从后往前第一次出现目标字符的位置。
 * 
 * @param s 字符串。
 * @param c 目标字符。
 * 
 * @return 对应位置指针，或返回空值。
 */
const char16* string_reverse_find_char16(const char16* s,char16 c)
{
    if(s==null)
    {
        return null;
    }
    const char16* result=null;
    while(*s!=0)
    {
        if(*s==c)
        {
            result=s;
        }
        s++;
    }
    return result;
}

/**
 * 搜索32位字符串中从后往前第一次出现目标字符的位置。
 * 
 * @param s 字符串。
 * @param c 目标字符。
 * 
 * @return 对应位置指针，或返回空值。
 */
const char32* string_reverse_find_char32(const char32* s,char32 c)
{
    if(s==null)
    {
        return null;
    }
    const char32* result=null;
    while(*s!=0)
    {
        if(*s==c)
        {
            result=s;
        }
        s++;
    }
    return result;
}

/**
 * 对8位字符串a与字符串b进行比较。
 * 
 * @param a 字符串a。
 * @param b 字符串b。
 * 
 * @return 相等返回0，a<b返回负数，a>b返回正数。
 */
int8 string_compare8(const char8* a,const char8* b)
{
    if(a==b)
    {
        return 0;
    }
    else if(a==null||b==null)
    {
        return a!=null?1:-1;
    }
    else
    {
        while(*a!=0&&*b!=0&&*a==*b)
        {
            a++;
            b++;
        }
        if(*a==*b)
        {
            return 0;
        }
        else
        {
            return *a>*b?1:-1;
        }
    }
}

/**
 * 对16位字符串a与字符串b进行比较。
 * 
 * @param a 字符串a。
 * @param b 字符串b。
 * 
 * @return 相等返回0，a<b返回负数，a>b返回正数。
 */
int8 string_compare16(const char16* a,const char16* b)
{
    if(a==b)
    {
        return 0;
    }
    else if(a==null||b==null)
    {
        return a!=null?1:-1;
    }
    else
    {
        while(*a!=0&&*b!=0&&*a==*b)
        {
            a++;
            b++;
        }
        if(*a==*b)
        {
            return 0;
        }
        else
        {
            return *a>*b?1:-1;
        }
    }
}

/**
 * 对32位字符串a与字符串b进行比较。
 * 
 * @param a 字符串a。
 * @param b 字符串b。
 * 
 * @return 相等返回0，a<b返回负数，a>b返回正数。
 */
int8 string_compare32(const char32* a,const char32* b)
{
    if(a==b)
    {
        return 0;
    }
    else if(a==null||b==null)
    {
        return a!=null?1:-1;
    }
    else
    {
        while(*a!=0&&*b!=0&&*a==*b)
        {
            a++;
            b++;
        }
        if(*a==*b)
        {
            return 0;
        }
        else
        {
            return *a>*b?1:-1;
        }
    }
}

/**
 * 从8位字符串获取子字符串。如果获取长度超过了源字符串长度则到源字符串结尾。
 * 
 * @param dest   目标字符串。
 * @param src    源字符串。
 * @param start  起始索引。
 * @param length 获取长度。
 * @param size   目标字符串可用大小，包含终结符。
 * 
 * @return 能获取返回一个正数长度，不能获取返回0。
 */
uintn string_substring8(char8* dest,const char8* src,uintn start,uintn length,uintn size)
{
    if(dest==null||src==null||size<1)
    {
        return 0;
    }
    uintn len_src=string_length8(src);
    if(start>=len_src)
    {
        dest[0]=0;
        return 0;
    }
    uintn result;
    uintn max=len_src-start;
    if(length>=max)
    {
        result=min(max,size-1);
    }
    else
    {
        result=min(length,size-1);
    }
    src=&src[start];
    for(uintn index=0;index<result;index++)
    {
        dest[index]=src[index];
    }
    dest[result]=0;
    return result;
}

/**
 * 从16位字符串获取子字符串。如果获取长度超过了源字符串长度则到源字符串结尾。
 * 
 * @param dest   目标字符串。
 * @param src    源字符串。
 * @param start  起始索引。
 * @param length 获取长度。
 * @param size   目标字符串可用大小，包含终结符。
 * 
 * @return 能获取返回一个正数长度，不能获取返回0。
 */
uintn string_substring16(char16* dest,const char16* src,uintn start,uintn length,uintn size)
{
    if(dest==null||src==null||size<1)
    {
        return 0;
    }
    uintn len_src=string_length16(src);
    if(start>=len_src)
    {
        dest[0]=0;
        return 0;
    }
    uintn result;
    uintn max=len_src-start;
    if(length>=max)
    {
        result=min(max,size-1);
    }
    else
    {
        result=min(length,size-1);
    }
    src=&src[start];
    for(uintn index=0;index<result;index++)
    {
        dest[index]=src[index];
    }
    dest[result]=0;
    return result;
}

/**
 * 从32位字符串获取子字符串。如果获取长度超过了源字符串长度则到源字符串结尾。
 * 
 * @param dest   目标字符串。
 * @param src    源字符串。
 * @param start  起始索引。
 * @param length 获取长度。
 * @param size   目标字符串可用大小，包含终结符。
 * 
 * @return 能获取返回一个正数长度，不能获取返回0。
 */
uintn string_substring32(char32* dest,const char32* src,uintn start,uintn length,uintn size)
{
    if(dest==null||src==null||size<1)
    {
        return 0;
    }
    uintn len_src=string_length32(src);
    if(start>=len_src)
    {
        dest[0]=0;
        return 0;
    }
    uintn result;
    uintn max=len_src-start;
    if(length>=max)
    {
        result=min(max,size-1);
    }
    else
    {
        result=min(length,size-1);
    }
    src=&src[start];
    for(uintn index=0;index<result;index++)
    {
        dest[index]=src[index];
    }
    dest[result]=0;
    return result;
}