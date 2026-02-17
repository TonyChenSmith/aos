/**
 * 内核数据转换库函数。
 * @date 2026-01-21
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include <support/char.h>
#include <support/kdef.h>
#include <support/ktype.h>
#include <support/string.h>
#include <support/util.h>

/**
 * 将8位十进制字符串转换为有符号整型数值。
 * 
 * @param number 输出数值。
 * @param str    字符串。
 * 
 * @return 正常转换返回真，错误转换返回假。
 */
bool string_to_int8(int64* number,const char8* str)
{
    if(number==null||str==null)
    {
        return false;
    }

    bool neg=false;
    while(ascii_is_blank(*str)){
        str++;
    }

    if(*str=='-')
    {
        neg=true;
        str++;
    }
    else if(*str=='+')
    {
        str++;
    }

    if(!ascii_is_digit(*str))
    {
        return false;
    }
    
    uint64 value=0,temp=0;
    while(ascii_is_digit(*str))
    {
        temp=value*10+(*str-'0');
        if(temp<value)
        {
            return false;
        }
        else
        {
            str++;
            value=temp;
        }
    }

    while(ascii_is_blank(*str)){
        str++;
    }
    if(*str!=0)
    {
        return false;
    }

    if(neg&&value<=BIT63)
    {
        *number=value==BIT63?INT64_MIN:-(int64)value;
        return true;
    }
    else if(value<=INT64_MAX)
    {
        *number=value;
        return true;
    }
    return false;
}

/**
 * 将16位十进制字符串转换为有符号整型数值。
 * 
 * @param number 输出数值。
 * @param str    字符串。
 * 
 * @return 正常转换返回真，错误转换返回假。
 */
bool string_to_int16(int64* number,const char16* str)
{
    if(number==null||str==null)
    {
        return false;
    }

    bool neg=false;
    while(ascii_is_blank(*str)){
        str++;
    }

    if(*str=='-')
    {
        neg=true;
        str++;
    }
    else if(*str=='+')
    {
        str++;
    }

    if(!ascii_is_digit(*str))
    {
        return false;
    }
    
    uint64 value=0,temp=0;
    while(ascii_is_digit(*str))
    {
        temp=value*10+(*str-'0');
        if(temp<value)
        {
            return false;
        }
        else
        {
            str++;
            value=temp;
        }
    }

    while(ascii_is_blank(*str)){
        str++;
    }
    if(*str!=0)
    {
        return false;
    }

    if(neg&&value<=BIT63)
    {
        *number=value==BIT63?INT64_MIN:-(int64)value;
        return true;
    }
    else if(value<=INT64_MAX)
    {
        *number=value;
        return true;
    }
    return false;
}

/**
 * 将32位十进制字符串转换为有符号整型数值。
 * 
 * @param number 输出数值。
 * @param str    字符串。
 * 
 * @return 正常转换返回真，错误转换返回假。
 */
bool string_to_int32(int64* number,const char32* str)
{
    if(number==null||str==null)
    {
        return false;
    }

    bool neg=false;
    while(ascii_is_blank(*str)){
        str++;
    }

    if(*str=='-')
    {
        neg=true;
        str++;
    }
    else if(*str=='+')
    {
        str++;
    }

    if(!ascii_is_digit(*str))
    {
        return false;
    }
    
    uint64 value=0,temp=0;
    while(ascii_is_digit(*str))
    {
        temp=value*10+(*str-'0');
        if(temp<value)
        {
            return false;
        }
        else
        {
            str++;
            value=temp;
        }
    }

    while(ascii_is_blank(*str)){
        str++;
    }
    if(*str!=0)
    {
        return false;
    }

    if(neg&&value<=BIT63)
    {
        *number=value==BIT63?INT64_MIN:-(int64)value;
        return true;
    }
    else if(value<=INT64_MAX)
    {
        *number=value;
        return true;
    }
    return false;
}

/**
 * 将8位数值字符串转换为无符号整型数值。
 * 
 * @param number 输出数值。
 * @param str    字符串。
 * @param scale  进制。合理范围为2到36，非法参数会转回10进制。
 * 
 * @return 正常转换返回真，错误转换返回假。
 */
bool string_to_uint8(uint64* number,const char8* str,uint8 scale)
{
    if(number==null||str==null)
    {
        return false;
    }
    else if(scale<2||scale>36)
    {
        scale=10;
    }

    while(ascii_is_blank(*str)){
        str++;
    }

    if(!(ascii_is_digit(*str)||ascii_is_alphabetic(*str)))
    {
        return false;
    }
    
    uint64 value=0,temp=0;
    uint64 digit=0;
    while(ascii_is_digit(*str)||ascii_is_alphabetic(*str))
    {
        if(ascii_is_digit(*str))
        {
            digit=*str-'0';
        }
        else if(ascii_is_lowercase(*str))
        {
            digit=*str-'a'+10;
        }
        else
        {
            digit=*str-'A'+10;
        }

        if(digit>=scale)
        {
            return false;
        }

        temp=value*scale+digit;
        if(temp<value)
        {
            return false;
        }
        else
        {
            str++;
            value=temp;
        }
    }

    while(ascii_is_blank(*str)){
        str++;
    }
    if(*str!=0)
    {
        return false;
    }
    *number=value;
    return true;
}

/**
 * 将16位数值字符串转换为无符号整型数值。
 * 
 * @param number 输出数值。
 * @param str    字符串。
 * @param scale  进制。合理范围为2到36，非法参数会转回10进制。
 * 
 * @return 正常转换返回真，错误转换返回假。
 */
bool string_to_uint16(uint64* number,const char16* str,uint8 scale)
{
    if(number==null||str==null)
    {
        return false;
    }
    else if(scale<2||scale>36)
    {
        scale=10;
    }

    while(ascii_is_blank(*str)){
        str++;
    }

    if(!(ascii_is_digit(*str)||ascii_is_alphabetic(*str)))
    {
        return false;
    }
    
    uint64 value=0,temp=0;
    uint64 digit=0;
    while(ascii_is_digit(*str)||ascii_is_alphabetic(*str))
    {
        if(ascii_is_digit(*str))
        {
            digit=*str-'0';
        }
        else if(ascii_is_lowercase(*str))
        {
            digit=*str-'a'+10;
        }
        else
        {
            digit=*str-'A'+10;
        }

        if(digit>=scale)
        {
            return false;
        }

        temp=value*scale+digit;
        if(temp<value)
        {
            return false;
        }
        else
        {
            str++;
            value=temp;
        }
    }

    while(ascii_is_blank(*str)){
        str++;
    }
    if(*str!=0)
    {
        return false;
    }
    *number=value;
    return true;
}

/**
 * 将32位数值字符串转换为无符号整型数值。
 * 
 * @param number 输出数值。
 * @param str    字符串。
 * @param scale  进制。合理范围为2到36，非法参数会转回10进制。
 * 
 * @return 正常转换返回真，错误转换返回假。
 */
bool string_to_uint32(uint64* number,const char32* str,uint8 scale)
{
    if(number==null||str==null)
    {
        return false;
    }
    else if(scale<2||scale>36)
    {
        scale=10;
    }

    while(ascii_is_blank(*str)){
        str++;
    }

    if(!(ascii_is_digit(*str)||ascii_is_alphabetic(*str)))
    {
        return false;
    }
    
    uint64 value=0,temp=0;
    uint64 digit=0;
    while(ascii_is_digit(*str)||ascii_is_alphabetic(*str))
    {
        if(ascii_is_digit(*str))
        {
            digit=*str-'0';
        }
        else if(ascii_is_lowercase(*str))
        {
            digit=*str-'a'+10;
        }
        else
        {
            digit=*str-'A'+10;
        }

        if(digit>=scale)
        {
            return false;
        }

        temp=value*scale+digit;
        if(temp<value)
        {
            return false;
        }
        else
        {
            str++;
            value=temp;
        }
    }

    while(ascii_is_blank(*str)){
        str++;
    }
    if(*str!=0)
    {
        return false;
    }
    *number=value;
    return true;
}

/**
 * 数字对应字符数组。
 */
static const char8 DIGIT_CHARS[]={'0','1','2','3','4','5','6','7','8','9',
    'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R',
    'S','T','U','V','W','X','Y','Z'};

/**
 * 将有符号整型数值转换为8位十进制字符串。
 * 
 * @param str    输出字符串。
 * @param number 输入数值。
 * @param size   字符串可用大小，包含终结符。
 * 
 * @return 正常转换返回真，错误转换返回假。
 */
bool int_to_string8(char8* str,int64 number,uintn size)
{
    if(str==null||size<2)
    {
        return false;
    }
    
    if(number==INT64_MIN)
    {
        if(size<21)
        {
            return false;
        }
        else
        {
            string_copy(str,"-9223372036854775808",size);
            return true;
        }
    }
    else if(number==0)
    {
        str[0]='0';
        str[1]=0;
        return true;
    }
    else if(number<0)
    {
        *str='-';
        str++;
        size--;
        number=abs(number);
    }

    char8* end=&str[size-1];
    *end--=0;
    while(end>=str&&number>0)
    {
        *end--=DIGIT_CHARS[number%10];
        number/=10;
    }
    end++;
    if(number>0)
    {
        return false;
    }
    else if(end>str)
    {
        while(*end!=0)
        {
            *str++=*end++;
        }
        *str=0;
    }
    return true;
}

/**
 * 将有符号整型数值转换为16位十进制字符串。
 * 
 * @param str    输出字符串。
 * @param number 输入数值。
 * @param size   字符串可用大小，包含终结符。
 * 
 * @return 正常转换返回真，错误转换返回假。
 */
bool int_to_string16(char16* str,int64 number,uintn size)
{
    if(str==null||size<2)
    {
        return false;
    }
    
    if(number==INT64_MIN)
    {
        if(size<21)
        {
            return false;
        }
        else
        {
            string_copy(str,u"-9223372036854775808",size);
            return true;
        }
    }
    else if(number==0)
    {
        str[0]='0';
        str[1]=0;
        return true;
    }
    else if(number<0)
    {
        *str='-';
        str++;
        size--;
        number=abs(number);
    }

    char16* end=&str[size-1];
    *end--=0;
    while(end>=str&&number>0)
    {
        *end--=DIGIT_CHARS[number%10];
        number/=10;
    }
    end++;
    if(number>0)
    {
        return false;
    }
    else if(end>str)
    {
        while(*end!=0)
        {
            *str++=*end++;
        }
        *str=0;
    }
    return true;
}

/**
 * 将有符号整型数值转换为32位十进制字符串。
 * 
 * @param str    输出字符串。
 * @param number 输入数值。
 * @param size   字符串可用大小，包含终结符。
 * 
 * @return 正常转换返回真，错误转换返回假。
 */
bool int_to_string32(char32* str,int64 number,uintn size)
{
    if(str==null||size<2)
    {
        return false;
    }
    
    if(number==INT64_MIN)
    {
        if(size<21)
        {
            return false;
        }
        else
        {
            string_copy(str,U"-9223372036854775808",size);
            return true;
        }
    }
    else if(number==0)
    {
        str[0]='0';
        str[1]=0;
        return true;
    }
    else if(number<0)
    {
        *str='-';
        str++;
        size--;
        number=abs(number);
    }

    char32* end=&str[size-1];
    *end--=0;
    while(end>=str&&number>0)
    {
        *end--=DIGIT_CHARS[number%10];
        number/=10;
    }
    end++;
    if(number>0)
    {
        return false;
    }
    else if(end>str)
    {
        while(*end!=0)
        {
            *str++=*end++;
        }
        *str=0;
    }
    return true;
}

/**
 * 将无符号整型数值转换为8位数值字符串。
 * 
 * @param str    输出字符串。
 * @param number 输入数值。
 * @param size   字符串可用大小，包含终结符。
 * @param scale  进制。合理范围为2到36，非法参数会转回10进制。
 * 
 * @return 正常转换返回真，错误转换返回假。
 */
bool uint_to_string8(char8* str,uint64 number,uintn size,uint8 scale)
{
    if(str==null||size<2)
    {
        return false;
    }
    else if(scale<2||scale>36)
    {
        scale=10;
    }
    
    if(number==0)
    {
        str[0]='0';
        str[1]=0;
        return true;
    }

    char8* end=&str[size-1];
    *end--=0;
    while(end>=str&&number>0)
    {
        *end--=DIGIT_CHARS[number%scale];
        number/=scale;
    }
    end++;
    if(number>0)
    {
        return false;
    }
    else if(end>str)
    {
        while(*end!=0)
        {
            *str++=*end++;
        }
        *str=0;
    }
    return true;
}

/**
 * 将无符号整型数值转换为16位数值字符串。
 * 
 * @param str    输出字符串。
 * @param number 输入数值。
 * @param size   字符串可用大小，包含终结符。
 * @param scale  进制。合理范围为2到36，非法参数会转回10进制。
 * 
 * @return 正常转换返回真，错误转换返回假。
 */
bool uint_to_string16(char16* str,uint64 number,uintn size,uint8 scale)
{
    if(str==null||size<2)
    {
        return false;
    }
    else if(scale<2||scale>36)
    {
        scale=10;
    }
    
    if(number==0)
    {
        str[0]='0';
        str[1]=0;
        return true;
    }

    char16* end=&str[size-1];
    *end--=0;
    while(end>=str&&number>0)
    {
        *end--=DIGIT_CHARS[number%scale];
        number/=scale;
    }
    end++;
    if(number>0)
    {
        return false;
    }
    else if(end>str)
    {
        while(*end!=0)
        {
            *str++=*end++;
        }
        *str=0;
    }
    return true;
}

/**
 * 将无符号整型数值转换为32位数值字符串。
 * 
 * @param str    输出字符串。
 * @param number 输入数值。
 * @param size   字符串可用大小，包含终结符。
 * @param scale  进制。合理范围为2到36，非法参数会转回10进制。
 * 
 * @return 正常转换返回真，错误转换返回假。
 */
bool uint_to_string32(char32* str,uint64 number,uintn size,uint8 scale)
{
    if(str==null||size<2)
    {
        return false;
    }
    else if(scale<2||scale>36)
    {
        scale=10;
    }
    
    if(number==0)
    {
        str[0]='0';
        str[1]=0;
        return true;
    }

    char32* end=&str[size-1];
    *end--=0;
    while(end>=str&&number>0)
    {
        *end--=DIGIT_CHARS[number%scale];
        number/=scale;
    }
    end++;
    if(number>0)
    {
        return false;
    }
    else if(end>str)
    {
        while(*end!=0)
        {
            *str++=*end++;
        }
        *str=0;
    }
    return true;
}

/**
 * 获取有符号数值转换成十进制字符串时所需数组大小。由于使用的的是ASCII字符，三种字符串返回的结果是一致的，无需分开实现。
 * 
 * @param number 输入数值。
 * 
 * @return 包括终结符的所需数组大小。
 */
uintn int_buffer_size(int64 number)
{
    uintn result=1;
    if(number==INT64_MIN)
    {
        return 21;
    }
    else if(number==0)
    {
        return 2;
    }
    else if(number<0)
    {
        result++;
        number=abs(number);
    }

    while(number>0)
    {
        result++;
        number/=10;
    }
    return result;
}

/**
 * 获取无符号数值转换成数值字符串时所需数组大小。由于使用的的是ASCII字符，三种字符串返回的结果是一致的，无需分开实现。
 * 
 * @param number 输入数值。
 * @param scale  进制。合理范围为2到36，非法参数会转回10进制。
 * 
 * @return 包括终结符的所需数组大小。
 */
uintn uint_buffer_size(uint64 number,uint8 scale)
{
    if(scale<2||scale>36)
    {
        scale=10;
    }
    uintn result=1;
    if(number==0)
    {
        return 2;
    }

    while(number>0)
    {
        result++;
        number/=scale;
    }
    return result;
}