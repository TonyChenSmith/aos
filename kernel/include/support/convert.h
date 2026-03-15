/**
 * 内核数据转换库函数。
 * @date 2026-01-21
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_KERNEL_SUPPORT_CONVERT_H__
#define __AOS_KERNEL_SUPPORT_CONVERT_H__

#include "const.h"
#include "type.h"

/**
 * 将十进制字符串转换为有符号整型数值。
 */
#define string_to_int(number,str) (generic((str),char8*:string_to_int8,const char8*:string_to_int8,\
    char16*:string_to_int16,const char16*:string_to_int16,char32*:string_to_int32,\
    const char32*:string_to_int32)(number,str))

/**
 * 将数值字符串转换为无符号整型数值。
 */
#define string_to_uint(number,str,scale) (generic((str),char8*:string_to_uint8,const char8*:string_to_uint8,\
    char16*:string_to_uint16,const char16*:string_to_uint16,char32*:string_to_uint32,\
    const char32*:string_to_uint32)(number,str,scale))

/**
 * 将有符号整型数值转换为十进制字符串。
 */
#define int_to_string(str,number,size) (generic((str),char8*:int_to_string8,char16*:int_to_string16,\
    char32*:int_to_string32)(str,number,size))

/**
 * 将无符号整型数值转换为数值字符串。
 */
#define uint_to_string(str,number,size,scale) (generic((str),char8*:uint_to_string8,char16*:uint_to_string16,\
    char32*:uint_to_string32)(str,number,size,scale))

/**
 * 将8位十进制字符串转换为有符号整型数值。
 * 
 * @param number 输出数值。
 * @param str    字符串。
 * 
 * @return 正常转换返回真，错误转换返回假。
 */
bool string_to_int8(int64* number,const char8* str);

/**
 * 将16位十进制字符串转换为有符号整型数值。
 * 
 * @param number 输出数值。
 * @param str    字符串。
 * 
 * @return 正常转换返回真，错误转换返回假。
 */
bool string_to_int16(int64* number,const char16* str);

/**
 * 将32位十进制字符串转换为有符号整型数值。
 * 
 * @param number 输出数值。
 * @param str    字符串。
 * 
 * @return 正常转换返回真，错误转换返回假。
 */
bool string_to_int32(int64* number,const char32* str);

/**
 * 将8位数值字符串转换为无符号整型数值。
 * 
 * @param number 输出数值。
 * @param str    字符串。
 * @param scale  进制。合理范围为2到36，非法参数会转回10进制。
 * 
 * @return 正常转换返回真，错误转换返回假。
 */
bool string_to_uint8(uint64* number,const char8* str,uint8 scale);

/**
 * 将16位数值字符串转换为无符号整型数值。
 * 
 * @param number 输出数值。
 * @param str    字符串。
 * @param scale  进制。合理范围为2到36，非法参数会转回10进制。
 * 
 * @return 正常转换返回真，错误转换返回假。
 */
bool string_to_uint16(uint64* number,const char16* str,uint8 scale);

/**
 * 将32位数值字符串转换为无符号整型数值。
 * 
 * @param number 输出数值。
 * @param str    字符串。
 * @param scale  进制。合理范围为2到36，非法参数会转回10进制。
 * 
 * @return 正常转换返回真，错误转换返回假。
 */
bool string_to_uint32(uint64* number,const char32* str,uint8 scale);

/**
 * 将有符号整型数值转换为8位十进制字符串。
 * 
 * @param str    输出字符串。
 * @param number 输入数值。
 * @param size   字符串可用大小，包含终结符。
 * 
 * @return 正常转换返回真，错误转换返回假。
 */
bool int_to_string8(char8* str,int64 number,uintn size);

/**
 * 将有符号整型数值转换为16位十进制字符串。
 * 
 * @param str    输出字符串。
 * @param number 输入数值。
 * @param size   字符串可用大小，包含终结符。
 * 
 * @return 正常转换返回真，错误转换返回假。
 */
bool int_to_string16(char16* str,int64 number,uintn size);

/**
 * 将有符号整型数值转换为32位十进制字符串。
 * 
 * @param str    输出字符串。
 * @param number 输入数值。
 * @param size   字符串可用大小，包含终结符。
 * 
 * @return 正常转换返回真，错误转换返回假。
 */
bool int_to_string32(char32* str,int64 number,uintn size);

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
bool uint_to_string8(char8* str,uint64 number,uintn size,uint8 scale);

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
bool uint_to_string16(char16* str,uint64 number,uintn size,uint8 scale);

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
bool uint_to_string32(char32* str,uint64 number,uintn size,uint8 scale);

/**
 * 获取有符号数值转换成十进制字符串时所需数组大小。由于使用的的是ASCII字符，三种字符串返回的结果是一致的，无需分开实现。
 * 
 * @param number 输入数值。
 * 
 * @return 包括终结符的所需数组大小。
 */
uintn int_buffer_size(int64 number);

/**
 * 获取无符号数值转换成数值字符串时所需数组大小。由于使用的的是ASCII字符，三种字符串返回的结果是一致的，无需分开实现。
 * 
 * @param number 输入数值。
 * @param scale  进制。合理范围为2到36，非法参数会转回10进制。
 * 
 * @return 包括终结符的所需数组大小。
 */
uintn uint_buffer_size(uint64 number,uint8 scale);

#endif /*__AOS_KERNEL_SUPPORT_CONVERT_H__*/