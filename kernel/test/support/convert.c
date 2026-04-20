/**
 * 内核数据转换库函数测试。
 * @date 2026-01-26
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include <test/utest.h>

#include <support/convert.h>
#include <support/string.h>

/**
 * 测试8位字符串转有符号整型基本功能。
 * 
 * @return 无返回值。
 */
UTEST_CASE(string_to_int8_basic)
{
    int64 number;
    
    UTEST_ASSERT_TRUE(string_to_int8(&number,"0"));
    UTEST_ASSERT_EQUAL(number,0);
    
    UTEST_ASSERT_TRUE(string_to_int8(&number,"123"));
    UTEST_ASSERT_EQUAL(number,123);
    
    UTEST_ASSERT_TRUE(string_to_int8(&number,"-456"));
    UTEST_ASSERT_EQUAL(number,-456);
    
    UTEST_ASSERT_TRUE(string_to_int8(&number," 789 "));
    UTEST_ASSERT_EQUAL(number,789);
    
    UTEST_ASSERT_TRUE(string_to_int8(&number,"+321"));
    UTEST_ASSERT_EQUAL(number,321);
    
    UTEST_ASSERT_FALSE(string_to_int8(&number,"abc"));
    UTEST_ASSERT_FALSE(string_to_int8(&number,"12.3"));
    UTEST_ASSERT_FALSE(string_to_int8(&number,""));
    UTEST_ASSERT_FALSE(string_to_int8(&number,"   "));
    
    UTEST_ASSERT_TRUE(string_to_int8(&number,"42"));
    UTEST_ASSERT_EQUAL(number,42);
    UTEST_ASSERT_TRUE(string_to_int8(&number,"-17"));
    UTEST_ASSERT_EQUAL(number,-17);
    UTEST_ASSERT_FALSE(string_to_int8(&number,"1a2"));
}

/**
 * 测试16位字符串转有符号整型基本功能。
 * 
 * @return 无返回值。
 */
UTEST_CASE(string_to_int16_basic)
{
    int64 number;
    
    UTEST_ASSERT_TRUE(string_to_int16(&number,u"0"));
    UTEST_ASSERT_EQUAL(number,0);
    
    UTEST_ASSERT_TRUE(string_to_int16(&number,u"12345"));
    UTEST_ASSERT_EQUAL(number,12345);
    
    UTEST_ASSERT_TRUE(string_to_int16(&number,u"-67890"));
    UTEST_ASSERT_EQUAL(number,-67890);
    
    UTEST_ASSERT_TRUE(string_to_int16(&number,u" 999 "));
    UTEST_ASSERT_EQUAL(number,999);
    
    UTEST_ASSERT_TRUE(string_to_int16(&number,u"+54321"));
    UTEST_ASSERT_EQUAL(number,54321);
    
    UTEST_ASSERT_FALSE(string_to_int16(&number,u"xyz"));
    UTEST_ASSERT_FALSE(string_to_int16(&number,u"12.34"));
    UTEST_ASSERT_FALSE(string_to_int16(&number,u""));
    
    UTEST_ASSERT_TRUE(string_to_int16(&number,u"32767"));
    UTEST_ASSERT_EQUAL(number,32767);
    UTEST_ASSERT_TRUE(string_to_int16(&number,u"-32768"));
    UTEST_ASSERT_EQUAL(number,-32768);
    UTEST_ASSERT_FALSE(string_to_int16(&number,u"abc123"));
}

/**
 * 测试32位字符串转有符号整型基本功能。
 * 
 * @return 无返回值。
 */
UTEST_CASE(string_to_int32_basic)
{
    int64 number;
    
    UTEST_ASSERT_TRUE(string_to_int32(&number,U"0"));
    UTEST_ASSERT_EQUAL(number,0);
    
    UTEST_ASSERT_TRUE(string_to_int32(&number,U"123456789"));
    UTEST_ASSERT_EQUAL(number,123456789);
    
    UTEST_ASSERT_TRUE(string_to_int32(&number,U"-987654321"));
    UTEST_ASSERT_EQUAL(number,-987654321);
    
    UTEST_ASSERT_TRUE(string_to_int32(&number,U" 2147483647 "));
    UTEST_ASSERT_EQUAL(number,2147483647);
    
    UTEST_ASSERT_TRUE(string_to_int32(&number,U"+1000000000"));
    UTEST_ASSERT_EQUAL(number,1000000000);
    
    UTEST_ASSERT_FALSE(string_to_int32(&number,U"αβγ"));
    UTEST_ASSERT_FALSE(string_to_int32(&number,U"123.456"));
    UTEST_ASSERT_FALSE(string_to_int32(&number,U""));
    
    UTEST_ASSERT_TRUE(string_to_int32(&number,U"65535"));
    UTEST_ASSERT_EQUAL(number,65535);
    UTEST_ASSERT_TRUE(string_to_int32(&number,U"-65535"));
    UTEST_ASSERT_EQUAL(number,-65535);
    UTEST_ASSERT_FALSE(string_to_int32(&number,U"12abc"));
}

/**
 * 测试通用字符串转有符号整型宏。
 * 
 * @return 无返回值。
 */
UTEST_CASE(string_to_int_macro)
{
    int64 number;
    
    UTEST_ASSERT_TRUE(string_to_int(&number,"123"));
    UTEST_ASSERT_EQUAL(number,123);
    
    UTEST_ASSERT_TRUE(string_to_int(&number,u"456"));
    UTEST_ASSERT_EQUAL(number,456);
    
    UTEST_ASSERT_TRUE(string_to_int(&number,U"789"));
    UTEST_ASSERT_EQUAL(number,789);
    
    UTEST_ASSERT_FALSE(string_to_int(&number,"abc"));
    UTEST_ASSERT_FALSE(string_to_int(&number,u"def"));
    UTEST_ASSERT_FALSE(string_to_int(&number,U"ghi"));
    
    UTEST_ASSERT_TRUE(string_to_int(&number,"-100"));
    UTEST_ASSERT_EQUAL(number,-100);
    UTEST_ASSERT_TRUE(string_to_int(&number,u"+200"));
    UTEST_ASSERT_EQUAL(number,200);
}

/**
 * 测试8位字符串转无符号整型基本功能。
 * 
 * @return 无返回值。
 */
UTEST_CASE(string_to_uint8_basic)
{
    uint64 number;
    
    UTEST_ASSERT_TRUE(string_to_uint8(&number,"0",10));
    UTEST_ASSERT_EQUAL(number,0);
    
    UTEST_ASSERT_TRUE(string_to_uint8(&number,"255",10));
    UTEST_ASSERT_EQUAL(number,255);
    
    UTEST_ASSERT_TRUE(string_to_uint8(&number,"FF",16));
    UTEST_ASSERT_EQUAL(number,255);
    
    UTEST_ASSERT_TRUE(string_to_uint8(&number,"11111111",2));
    UTEST_ASSERT_EQUAL(number,255);
    
    UTEST_ASSERT_TRUE(string_to_uint8(&number," 123 ",10));
    UTEST_ASSERT_EQUAL(number,123);
    
    UTEST_ASSERT_FALSE(string_to_uint8(&number,"abc",10));
    UTEST_ASSERT_FALSE(string_to_uint8(&number,"12z",10));
    UTEST_ASSERT_FALSE(string_to_uint8(&number,"",10));
    
    UTEST_ASSERT_TRUE(string_to_uint8(&number,"7F",16));
    UTEST_ASSERT_EQUAL(number,127);
    UTEST_ASSERT_TRUE(string_to_uint8(&number,"1010",2));
    UTEST_ASSERT_EQUAL(number,10);
    UTEST_ASSERT_FALSE(string_to_uint8(&number,"g",16));
}

/**
 * 测试16位字符串转无符号整型基本功能。
 * 
 * @return 无返回值。
 */
UTEST_CASE(string_to_uint16_basic)
{
    uint64 number;
    
    UTEST_ASSERT_TRUE(string_to_uint16(&number,u"0",10));
    UTEST_ASSERT_EQUAL(number,0);
    
    UTEST_ASSERT_TRUE(string_to_uint16(&number,u"65535",10));
    UTEST_ASSERT_EQUAL(number,65535);
    
    UTEST_ASSERT_TRUE(string_to_uint16(&number,u"FFFF",16));
    UTEST_ASSERT_EQUAL(number,65535);
    
    UTEST_ASSERT_TRUE(string_to_uint16(&number,u" 9999 ",10));
    UTEST_ASSERT_EQUAL(number,9999);
    
    UTEST_ASSERT_TRUE(string_to_uint16(&number,u"7FFF",16));
    UTEST_ASSERT_EQUAL(number,32767);
    
    UTEST_ASSERT_FALSE(string_to_uint16(&number,u"xyz",10));
    UTEST_ASSERT_FALSE(string_to_uint16(&number,u"12.34",10));
    UTEST_ASSERT_FALSE(string_to_uint16(&number,u"",10));
    
    UTEST_ASSERT_TRUE(string_to_uint16(&number,u"3FF",16));
    UTEST_ASSERT_EQUAL(number,1023);
    UTEST_ASSERT_TRUE(string_to_uint16(&number,u"1111111111111111",2));
    UTEST_ASSERT_EQUAL(number,65535);
    UTEST_ASSERT_FALSE(string_to_uint16(&number,u"1g2",16));
}

/**
 * 测试32位字符串转无符号整型基本功能。
 * 
 * @return 无返回值。
 */
UTEST_CASE(string_to_uint32_basic)
{
    uint64 number;
    
    UTEST_ASSERT_TRUE(string_to_uint32(&number,U"0",10));
    UTEST_ASSERT_EQUAL(number,0);
    
    UTEST_ASSERT_TRUE(string_to_uint32(&number,U"4294967295",10));
    UTEST_ASSERT_EQUAL(number,4294967295);
    
    UTEST_ASSERT_TRUE(string_to_uint32(&number,U"FFFFFFFF",16));
    UTEST_ASSERT_EQUAL(number,4294967295);
    
    UTEST_ASSERT_TRUE(string_to_uint32(&number,U" 123456789 ",10));
    UTEST_ASSERT_EQUAL(number,123456789);
    
    UTEST_ASSERT_TRUE(string_to_uint32(&number,U"7FFFFFFF",16));
    UTEST_ASSERT_EQUAL(number,2147483647);
    
    UTEST_ASSERT_FALSE(string_to_uint32(&number,U"αβγ",10));
    UTEST_ASSERT_FALSE(string_to_uint32(&number,U"123.456",10));
    UTEST_ASSERT_FALSE(string_to_uint32(&number,U"",10));
    
    UTEST_ASSERT_TRUE(string_to_uint32(&number,U"DEADBEEF",16));
    UTEST_ASSERT_EQUAL(number,0xDEADBEEF);
    UTEST_ASSERT_TRUE(string_to_uint32(&number,U"123456",8));
    UTEST_ASSERT_EQUAL(number,0123456);
    UTEST_ASSERT_FALSE(string_to_uint32(&number,U"1z2",16));
}

/**
 * 测试通用字符串转无符号整型宏。
 * 
 * @return 无返回值。
 */
UTEST_CASE(string_to_uint_macro)
{
    uint64 number;
    
    UTEST_ASSERT_TRUE(string_to_uint(&number,"123",10));
    UTEST_ASSERT_EQUAL(number,123);
    
    UTEST_ASSERT_TRUE(string_to_uint(&number,u"456",10));
    UTEST_ASSERT_EQUAL(number,456);
    
    UTEST_ASSERT_TRUE(string_to_uint(&number,U"789",10));
    UTEST_ASSERT_EQUAL(number,789);
    
    UTEST_ASSERT_TRUE(string_to_uint(&number,"FF",16));
    UTEST_ASSERT_EQUAL(number,255);
    
    UTEST_ASSERT_TRUE(string_to_uint(&number,u"1010",2));
    UTEST_ASSERT_EQUAL(number,10);
    
    UTEST_ASSERT_FALSE(string_to_uint(&number,"abc",10));
    UTEST_ASSERT_FALSE(string_to_uint(&number,u"xyz",16));
}

/**
 * 测试不同进制转换功能。
 * 
 * @return 无返回值。
 */
UTEST_CASE(string_to_uint_multiscale)
{
    uint64 number;
    
    UTEST_ASSERT_TRUE(string_to_uint8(&number,"1010",2));
    UTEST_ASSERT_EQUAL(number,10);
    
    UTEST_ASSERT_TRUE(string_to_uint8(&number,"755",8));
    UTEST_ASSERT_EQUAL(number,0755);
    
    UTEST_ASSERT_TRUE(string_to_uint8(&number,"123",10));
    UTEST_ASSERT_EQUAL(number,123);
    
    UTEST_ASSERT_TRUE(string_to_uint8(&number,"FF",16));
    UTEST_ASSERT_EQUAL(number,255);
    
    UTEST_ASSERT_TRUE(string_to_uint8(&number,"Z",36));
    UTEST_ASSERT_EQUAL(number,35);
    
    UTEST_ASSERT_TRUE(string_to_uint8(&number,"123",1));
    UTEST_ASSERT_EQUAL(number,123);
    
    UTEST_ASSERT_TRUE(string_to_uint8(&number,"456",37));
    UTEST_ASSERT_EQUAL(number,456);
    
    UTEST_ASSERT_TRUE(string_to_uint8(&number,"10",2));
    UTEST_ASSERT_EQUAL(number,2);
    UTEST_ASSERT_TRUE(string_to_uint8(&number,"10",8));
    UTEST_ASSERT_EQUAL(number,8);
    UTEST_ASSERT_TRUE(string_to_uint8(&number,"10",16));
    UTEST_ASSERT_EQUAL(number,16);
    UTEST_ASSERT_TRUE(string_to_uint8(&number,"10",36));
    UTEST_ASSERT_EQUAL(number,36);
}

/**
 * 测试有符号整型转8位字符串基本功能。
 * 
 * @return 无返回值。
 */
UTEST_CASE(int_to_string8_basic)
{
    char8 buffer[32];
    
    UTEST_ASSERT_TRUE(int_to_string8(buffer,0,sizeof(buffer)));
    UTEST_ASSERT_STRING_EQUAL(buffer,"0");
    
    UTEST_ASSERT_TRUE(int_to_string8(buffer,123,sizeof(buffer)));
    UTEST_ASSERT_STRING_EQUAL(buffer,"123");
    
    UTEST_ASSERT_TRUE(int_to_string8(buffer,-456,sizeof(buffer)));
    UTEST_ASSERT_STRING_EQUAL(buffer,"-456");
    
    UTEST_ASSERT_TRUE(int_to_string8(buffer,2147483647,sizeof(buffer)));
    UTEST_ASSERT_STRING_EQUAL(buffer,"2147483647");
    
    UTEST_ASSERT_TRUE(int_to_string8(buffer,-2147483648,sizeof(buffer)));
    UTEST_ASSERT_STRING_EQUAL(buffer,"-2147483648");
    
    UTEST_ASSERT_FALSE(int_to_string8(buffer,123456,3));
    
    UTEST_ASSERT_TRUE(int_to_string8(buffer,999,5));
    UTEST_ASSERT_STRING_EQUAL(buffer,"999");
    
    UTEST_ASSERT_TRUE(int_to_string8(buffer,-100,5));
    UTEST_ASSERT_STRING_EQUAL(buffer,"-100");
    UTEST_ASSERT_FALSE(int_to_string8(buffer,1000,4));
}

/**
 * 测试有符号整型转16位字符串基本功能。
 * 
 * @return 无返回值。
 */
UTEST_CASE(int_to_string16_basic)
{
    char16 buffer[32];
    
    UTEST_ASSERT_TRUE(int_to_string16(buffer,0,sizeof(buffer)/sizeof(char16)));
    UTEST_ASSERT_EQUAL(string_compare(buffer,u"0"),0);
    
    UTEST_ASSERT_TRUE(int_to_string16(buffer,12345,sizeof(buffer)/sizeof(char16)));
    UTEST_ASSERT_EQUAL(string_compare(buffer,u"12345"),0);
    
    UTEST_ASSERT_TRUE(int_to_string16(buffer,-67890,sizeof(buffer)/sizeof(char16)));
    UTEST_ASSERT_EQUAL(string_compare(buffer,u"-67890"),0);
    
    UTEST_ASSERT_TRUE(int_to_string16(buffer,32767,sizeof(buffer)/sizeof(char16)));
    UTEST_ASSERT_EQUAL(string_compare(buffer,u"32767"),0);
    
    UTEST_ASSERT_TRUE(int_to_string16(buffer,-32768,sizeof(buffer)/sizeof(char16)));
    UTEST_ASSERT_EQUAL(string_compare(buffer,u"-32768"),0);
    
    UTEST_ASSERT_FALSE(int_to_string16(buffer,123456,3));
    
    UTEST_ASSERT_TRUE(int_to_string16(buffer,777,5));
    UTEST_ASSERT_EQUAL(string_compare(buffer,u"777"),0);
    UTEST_ASSERT_TRUE(int_to_string16(buffer,-888,5));
    UTEST_ASSERT_EQUAL(string_compare(buffer,u"-888"),0);
    UTEST_ASSERT_FALSE(int_to_string16(buffer,12345,5));
}

/**
 * 测试有符号整型转32位字符串基本功能。
 * 
 * @return 无返回值。
 */
UTEST_CASE(int_to_string32_basic)
{
    char32 buffer[32];
    
    UTEST_ASSERT_TRUE(int_to_string32(buffer,0,sizeof(buffer)/sizeof(char32)));
    UTEST_ASSERT_EQUAL(string_compare(buffer,U"0"),0);
    
    UTEST_ASSERT_TRUE(int_to_string32(buffer,123456789,sizeof(buffer)/sizeof(char32)));
    UTEST_ASSERT_EQUAL(string_compare(buffer,U"123456789"),0);
    
    UTEST_ASSERT_TRUE(int_to_string32(buffer,-987654321,sizeof(buffer)/sizeof(char32)));
    UTEST_ASSERT_EQUAL(string_compare(buffer,U"-987654321"),0);
    
    UTEST_ASSERT_TRUE(int_to_string32(buffer,2147483647,sizeof(buffer)/sizeof(char32)));
    UTEST_ASSERT_EQUAL(string_compare(buffer,U"2147483647"),0);
    
    UTEST_ASSERT_TRUE(int_to_string32(buffer,-2147483648,sizeof(buffer)/sizeof(char32)));
    UTEST_ASSERT_EQUAL(string_compare(buffer,U"-2147483648"),0);
    
    UTEST_ASSERT_FALSE(int_to_string32(buffer,1234567890,5));
    
    UTEST_ASSERT_TRUE(int_to_string32(buffer,9999,6));
    UTEST_ASSERT_EQUAL(string_compare(buffer,U"9999"),0);
    UTEST_ASSERT_TRUE(int_to_string32(buffer,-1111,6));
    UTEST_ASSERT_EQUAL(string_compare(buffer,U"-1111"),0);
    UTEST_ASSERT_FALSE(int_to_string32(buffer,123456,6));
}

/**
 * 测试通用有符号整型转字符串宏。
 * 
 * @return 无返回值。
 */
UTEST_CASE(int_to_string_macro)
{
    char8 buffer8[32];
    char16 buffer16[32];
    char32 buffer32[32];
    
    UTEST_ASSERT_TRUE(int_to_string(buffer8,123,sizeof(buffer8)));
    UTEST_ASSERT_STRING_EQUAL(buffer8,"123");
    
    UTEST_ASSERT_TRUE(int_to_string(buffer16,-456,sizeof(buffer16)/sizeof(char16)));
    UTEST_ASSERT_EQUAL(string_compare(buffer16,u"-456"),0);
    
    UTEST_ASSERT_TRUE(int_to_string(buffer32,789,sizeof(buffer32)/sizeof(char32)));
    UTEST_ASSERT_EQUAL(string_compare(buffer32,U"789"),0);
    
    UTEST_ASSERT_FALSE(int_to_string(buffer8,1234567890,5));
    UTEST_ASSERT_FALSE(int_to_string(buffer16,-987654321,5));
}

/**
 * 测试无符号整型转8位字符串基本功能。
 * 
 * @return 无返回值。
 */
UTEST_CASE(uint_to_string8_basic)
{
    char8 buffer[32];
    
    UTEST_ASSERT_TRUE(uint_to_string8(buffer,0,sizeof(buffer),10));
    UTEST_ASSERT_STRING_EQUAL(buffer,"0");
    
    UTEST_ASSERT_TRUE(uint_to_string8(buffer,255,sizeof(buffer),10));
    UTEST_ASSERT_STRING_EQUAL(buffer,"255");
    
    UTEST_ASSERT_TRUE(uint_to_string8(buffer,255,sizeof(buffer),16));
    UTEST_ASSERT_STRING_EQUAL(buffer,"FF");
    
    UTEST_ASSERT_TRUE(uint_to_string8(buffer,10,sizeof(buffer),2));
    UTEST_ASSERT_STRING_EQUAL(buffer,"1010");
    
    UTEST_ASSERT_TRUE(uint_to_string8(buffer,0755,sizeof(buffer),8));
    UTEST_ASSERT_STRING_EQUAL(buffer,"755");
    
    UTEST_ASSERT_FALSE(uint_to_string8(buffer,123456,3,10));
    
    UTEST_ASSERT_TRUE(uint_to_string8(buffer,0xABCD,sizeof(buffer),16));
    UTEST_ASSERT_STRING_EQUAL(buffer,"ABCD");
    
    UTEST_ASSERT_TRUE(uint_to_string8(buffer,35,sizeof(buffer),36));
    UTEST_ASSERT_STRING_EQUAL(buffer,"Z");
    
    UTEST_ASSERT_TRUE(uint_to_string8(buffer,123,sizeof(buffer),10));
    UTEST_ASSERT_STRING_EQUAL(buffer,"123");
    UTEST_ASSERT_TRUE(uint_to_string8(buffer,0xFF,sizeof(buffer),16));
    UTEST_ASSERT_STRING_EQUAL(buffer,"FF");
    UTEST_ASSERT_FALSE(uint_to_string8(buffer,1000,2,10));
}

/**
 * 测试无符号整型转16位字符串基本功能。
 * 
 * @return 无返回值。
 */
UTEST_CASE(uint_to_string16_basic)
{
    char16 buffer[32];
    
    UTEST_ASSERT_TRUE(uint_to_string16(buffer,0,sizeof(buffer)/sizeof(char16),10));
    UTEST_ASSERT_EQUAL(string_compare(buffer,u"0"),0);
    
    UTEST_ASSERT_TRUE(uint_to_string16(buffer,65535,sizeof(buffer)/sizeof(char16),10));
    UTEST_ASSERT_EQUAL(string_compare(buffer,u"65535"),0);
    
    UTEST_ASSERT_TRUE(uint_to_string16(buffer,0xFFFF,sizeof(buffer)/sizeof(char16),16));
    UTEST_ASSERT_EQUAL(string_compare(buffer,u"FFFF"),0);
    
    UTEST_ASSERT_TRUE(uint_to_string16(buffer,0b1010,sizeof(buffer)/sizeof(char16),2));
    UTEST_ASSERT_EQUAL(string_compare(buffer,u"1010"),0);
    
    UTEST_ASSERT_TRUE(uint_to_string16(buffer,07777,sizeof(buffer)/sizeof(char16),8));
    UTEST_ASSERT_EQUAL(string_compare(buffer,u"7777"),0);
    
    UTEST_ASSERT_FALSE(uint_to_string16(buffer,123456,3,10));
    
    UTEST_ASSERT_TRUE(uint_to_string16(buffer,0xDEAD,sizeof(buffer)/sizeof(char16),16));
    UTEST_ASSERT_EQUAL(string_compare(buffer,u"DEAD"),0);
    UTEST_ASSERT_TRUE(uint_to_string16(buffer,1234,sizeof(buffer)/sizeof(char16),10));
    UTEST_ASSERT_EQUAL(string_compare(buffer,u"1234"),0);
    UTEST_ASSERT_FALSE(uint_to_string16(buffer,12345,5,10));
}

/**
 * 测试无符号整型转32位字符串基本功能。
 * 
 * @return 无返回值。
 */
UTEST_CASE(uint_to_string32_basic)
{
    char32 buffer[32];
    
    UTEST_ASSERT_TRUE(uint_to_string32(buffer,0,sizeof(buffer)/sizeof(char32),10));
    UTEST_ASSERT_EQUAL(string_compare(buffer,U"0"),0);
    
    UTEST_ASSERT_TRUE(uint_to_string32(buffer,4294967295,sizeof(buffer)/sizeof(char32),10));
    UTEST_ASSERT_EQUAL(string_compare(buffer,U"4294967295"),0);
    
    UTEST_ASSERT_TRUE(uint_to_string32(buffer,0xFFFFFFFF,sizeof(buffer)/sizeof(char32),16));
    UTEST_ASSERT_EQUAL(string_compare(buffer,U"FFFFFFFF"),0);
    
    UTEST_ASSERT_TRUE(uint_to_string32(buffer,0b1111,sizeof(buffer)/sizeof(char32),2));
    UTEST_ASSERT_EQUAL(string_compare(buffer,U"1111"),0);
    
    UTEST_ASSERT_TRUE(uint_to_string32(buffer,077777777,sizeof(buffer)/sizeof(char32),8));
    UTEST_ASSERT_EQUAL(string_compare(buffer,U"77777777"),0);
    
    UTEST_ASSERT_FALSE(uint_to_string32(buffer,1234567890,5,10));
    
    UTEST_ASSERT_TRUE(uint_to_string32(buffer,0xDEADBEEF,sizeof(buffer)/sizeof(char32),16));
    UTEST_ASSERT_EQUAL(string_compare(buffer,U"DEADBEEF"),0);
    UTEST_ASSERT_TRUE(uint_to_string32(buffer,1234567,sizeof(buffer)/sizeof(char32),10));
    UTEST_ASSERT_EQUAL(string_compare(buffer,U"1234567"),0);
    UTEST_ASSERT_FALSE(uint_to_string32(buffer,12345678,8,10));
}

/**
 * 测试通用无符号整型转字符串宏。
 * 
 * @return 无返回值。
 */
UTEST_CASE(uint_to_string_macro)
{
    char8 buffer8[32];
    char16 buffer16[32];
    char32 buffer32[32];
    
    UTEST_ASSERT_TRUE(uint_to_string(buffer8,123,sizeof(buffer8),10));
    UTEST_ASSERT_STRING_EQUAL(buffer8,"123");
    
    UTEST_ASSERT_TRUE(uint_to_string(buffer16,0xFF,sizeof(buffer16)/sizeof(char16),16));
    UTEST_ASSERT_EQUAL(string_compare(buffer16,u"FF"),0);
    
    UTEST_ASSERT_TRUE(uint_to_string(buffer32,0b1010,sizeof(buffer32)/sizeof(char32),2));
    UTEST_ASSERT_EQUAL(string_compare(buffer32,U"1010"),0);
    
    UTEST_ASSERT_FALSE(uint_to_string(buffer8,1234567890,5,10));
    UTEST_ASSERT_FALSE(uint_to_string(buffer16,0xFFFFF,3,16));
}

/**
 * 测试不同进制的无符号整型转字符串。
 * 
 * @return 无返回值。
 */
UTEST_CASE(uint_to_string_multiscale)
{
    char8 buffer[32];
    
    UTEST_ASSERT_TRUE(uint_to_string8(buffer,10,sizeof(buffer),2));
    UTEST_ASSERT_STRING_EQUAL(buffer,"1010");
    
    UTEST_ASSERT_TRUE(uint_to_string8(buffer,0755,sizeof(buffer),8));
    UTEST_ASSERT_STRING_EQUAL(buffer,"755");
    
    UTEST_ASSERT_TRUE(uint_to_string8(buffer,123,sizeof(buffer),10));
    UTEST_ASSERT_STRING_EQUAL(buffer,"123");
    
    UTEST_ASSERT_TRUE(uint_to_string8(buffer,0xFF,sizeof(buffer),16));
    UTEST_ASSERT_STRING_EQUAL(buffer,"FF");
    
    UTEST_ASSERT_TRUE(uint_to_string8(buffer,35,sizeof(buffer),36));
    UTEST_ASSERT_STRING_EQUAL(buffer,"Z");
    
    UTEST_ASSERT_TRUE(uint_to_string8(buffer,123,sizeof(buffer),1));
    UTEST_ASSERT_STRING_EQUAL(buffer,"123");
    
    UTEST_ASSERT_TRUE(uint_to_string8(buffer,456,sizeof(buffer),37));
    UTEST_ASSERT_STRING_EQUAL(buffer,"456");
    
    UTEST_ASSERT_TRUE(uint_to_string8(buffer,2,sizeof(buffer),2));
    UTEST_ASSERT_STRING_EQUAL(buffer,"10");
    UTEST_ASSERT_TRUE(uint_to_string8(buffer,8,sizeof(buffer),8));
    UTEST_ASSERT_STRING_EQUAL(buffer,"10");
    UTEST_ASSERT_TRUE(uint_to_string8(buffer,16,sizeof(buffer),16));
    UTEST_ASSERT_STRING_EQUAL(buffer,"10");
    UTEST_ASSERT_TRUE(uint_to_string8(buffer,36,sizeof(buffer),36));
    UTEST_ASSERT_STRING_EQUAL(buffer,"10");
}

/**
 * 测试有符号整型缓冲区大小计算。
 * 
 * @return 无返回值。
 */
UTEST_CASE(int_buffer_size_test)
{
    UTEST_ASSERT_EQUAL(int_buffer_size(0),2);
    UTEST_ASSERT_EQUAL(int_buffer_size(1),2);
    UTEST_ASSERT_EQUAL(int_buffer_size(9),2);
    UTEST_ASSERT_EQUAL(int_buffer_size(10),3);
    UTEST_ASSERT_EQUAL(int_buffer_size(99),3);
    UTEST_ASSERT_EQUAL(int_buffer_size(100),4);
    UTEST_ASSERT_EQUAL(int_buffer_size(999),4);
    UTEST_ASSERT_EQUAL(int_buffer_size(1000),5);
    UTEST_ASSERT_EQUAL(int_buffer_size(12345),6);
    UTEST_ASSERT_EQUAL(int_buffer_size(-1),3);
    UTEST_ASSERT_EQUAL(int_buffer_size(-10),4);
    UTEST_ASSERT_EQUAL(int_buffer_size(-100),5);
    UTEST_ASSERT_EQUAL(int_buffer_size(-999),5);
    UTEST_ASSERT_EQUAL(int_buffer_size(-1000),6);
    UTEST_ASSERT_EQUAL(int_buffer_size(2147483647),11);
    UTEST_ASSERT_EQUAL(int_buffer_size(-2147483648),12);
    
    UTEST_ASSERT_EQUAL(int_buffer_size(42),3);
    UTEST_ASSERT_EQUAL(int_buffer_size(-777),5);
    UTEST_ASSERT_EQUAL(int_buffer_size(999999),7);
}

/**
 * 测试无符号整型缓冲区大小计算。
 * 
 * @return 无返回值。
 */
UTEST_CASE(uint_buffer_size_test)
{
    UTEST_ASSERT_EQUAL(uint_buffer_size(0,10),2);
    UTEST_ASSERT_EQUAL(uint_buffer_size(1,10),2);
    UTEST_ASSERT_EQUAL(uint_buffer_size(9,10),2);
    UTEST_ASSERT_EQUAL(uint_buffer_size(10,10),3);
    UTEST_ASSERT_EQUAL(uint_buffer_size(99,10),3);
    UTEST_ASSERT_EQUAL(uint_buffer_size(100,10),4);
    UTEST_ASSERT_EQUAL(uint_buffer_size(255,10),4);
    UTEST_ASSERT_EQUAL(uint_buffer_size(255,16),3);
    UTEST_ASSERT_EQUAL(uint_buffer_size(255,2),9);
    UTEST_ASSERT_EQUAL(uint_buffer_size(255,8),4);
    UTEST_ASSERT_EQUAL(uint_buffer_size(65535,10),6);
    UTEST_ASSERT_EQUAL(uint_buffer_size(65535,16),5);
    UTEST_ASSERT_EQUAL(uint_buffer_size(4294967295,10),11);
    UTEST_ASSERT_EQUAL(uint_buffer_size(4294967295,16),9);
    UTEST_ASSERT_EQUAL(uint_buffer_size(4294967295,2),33);
    
    UTEST_ASSERT_EQUAL(uint_buffer_size(35,36),2);
    UTEST_ASSERT_EQUAL(uint_buffer_size(36,36),3);
    
    UTEST_ASSERT_EQUAL(uint_buffer_size(123,1),4);
    UTEST_ASSERT_EQUAL(uint_buffer_size(456,37),4);
    
    UTEST_ASSERT_EQUAL(uint_buffer_size(0xFF,16),3);
    UTEST_ASSERT_EQUAL(uint_buffer_size(0b1010,2),5);
    UTEST_ASSERT_EQUAL(uint_buffer_size(1234,10),5);
}

/**
 * 测试边界值和特殊情况。
 * 
 * @return 无返回值。
 */
UTEST_CASE(convert_edge_cases)
{
    int64 signed_number;
    uint64 unsigned_number;
    char8 buffer8[64];
    char16 buffer16[64];
    char32 buffer32[64];
    
    UTEST_ASSERT_TRUE(string_to_int8(&signed_number,"9223372036854775807"));
    UTEST_ASSERT_EQUAL(signed_number,INT64_MAX);
    
    UTEST_ASSERT_TRUE(string_to_int8(&signed_number,"-9223372036854775808"));
    UTEST_ASSERT_EQUAL(signed_number,INT64_MIN);
    
    UTEST_ASSERT_TRUE(string_to_uint8(&unsigned_number,"18446744073709551615",10));
    UTEST_ASSERT_EQUAL(unsigned_number,UINT64_MAX);
    
    UTEST_ASSERT_FALSE(string_to_int8(&signed_number,"9223372036854775808"));
    UTEST_ASSERT_FALSE(string_to_int8(&signed_number,"-9223372036854775809"));
    UTEST_ASSERT_FALSE(string_to_uint8(&unsigned_number,"18446744073709551616",10));
    
    UTEST_ASSERT_TRUE(int_to_string8(buffer8,INT64_MAX,int_buffer_size(INT64_MAX)));
    UTEST_ASSERT_STRING_EQUAL(buffer8,"9223372036854775807");
    
    UTEST_ASSERT_TRUE(int_to_string8(buffer8,INT64_MIN,int_buffer_size(INT64_MIN)));
    UTEST_ASSERT_STRING_EQUAL(buffer8,"-9223372036854775808");
    
    UTEST_ASSERT_TRUE(uint_to_string8(buffer8,UINT64_MAX,uint_buffer_size(UINT64_MAX,10),10));
    UTEST_ASSERT_STRING_EQUAL(buffer8,"18446744073709551615");
    
    UTEST_ASSERT_TRUE(int_to_string8(buffer8,123,int_buffer_size(123)));
    UTEST_ASSERT_STRING_EQUAL(buffer8,"123");
    
    UTEST_ASSERT_TRUE(uint_to_string8(buffer8,0xFF,uint_buffer_size(0xFF,16),16));
    UTEST_ASSERT_STRING_EQUAL(buffer8,"FF");
    
    UTEST_ASSERT_TRUE(string_to_uint8(&unsigned_number,"0",10));
    UTEST_ASSERT_EQUAL(unsigned_number,0);
    
    UTEST_ASSERT_TRUE(string_to_int8(&signed_number,"0"));
    UTEST_ASSERT_EQUAL(signed_number,0);
    
    UTEST_ASSERT_TRUE(string_to_int8(&signed_number,"-0"));
    UTEST_ASSERT_EQUAL(signed_number,0);
    
    UTEST_ASSERT_TRUE(string_to_int8(&signed_number,"+0"));
    UTEST_ASSERT_EQUAL(signed_number,0);
    
    UTEST_ASSERT_TRUE(string_to_int8(&signed_number,"   123"));
    UTEST_ASSERT_EQUAL(signed_number,123);
    
    UTEST_ASSERT_TRUE(string_to_int8(&signed_number,"   -456   "));
    UTEST_ASSERT_EQUAL(signed_number,-456);
    
    UTEST_ASSERT_FALSE(string_to_int8(&signed_number,"   "));
    UTEST_ASSERT_FALSE(string_to_int8(&signed_number,""));
    
    UTEST_ASSERT_TRUE(string_to_uint8(&unsigned_number,"Z",36));
    UTEST_ASSERT_EQUAL(unsigned_number,35);
    
    UTEST_ASSERT_TRUE(string_to_uint8(&unsigned_number,"10",36));
    UTEST_ASSERT_EQUAL(unsigned_number,36);
    
    UTEST_ASSERT_FALSE(string_to_uint8(&unsigned_number,"Z",35));
    
    UTEST_ASSERT_TRUE(int_to_string16(buffer16,123,int_buffer_size(123)));
    UTEST_ASSERT_EQUAL(string_compare(buffer16,u"123"),0);
    
    UTEST_ASSERT_TRUE(int_to_string32(buffer32,-456,int_buffer_size(-456)));
    UTEST_ASSERT_EQUAL(string_compare(buffer32,U"-456"),0);
}

/**
 * 测试综合场景。
 * 
 * @return 无返回值。
 */
UTEST_CASE(convert_integration_test)
{
    char8 buffer8[32];
    int64 signed_number;
    uint64 unsigned_number;
    
    UTEST_ASSERT_TRUE(string_to_int8(&signed_number,"-123456789"));
    UTEST_ASSERT_EQUAL(signed_number,-123456789);
    UTEST_ASSERT_TRUE(int_to_string8(buffer8,signed_number,sizeof(buffer8)));
    UTEST_ASSERT_STRING_EQUAL(buffer8,"-123456789");
    
    UTEST_ASSERT_TRUE(string_to_uint8(&unsigned_number,"FF",16));
    UTEST_ASSERT_EQUAL(unsigned_number,255);
    UTEST_ASSERT_TRUE(uint_to_string8(buffer8,unsigned_number,sizeof(buffer8),10));
    UTEST_ASSERT_STRING_EQUAL(buffer8,"255");
    UTEST_ASSERT_TRUE(uint_to_string8(buffer8,unsigned_number,sizeof(buffer8),2));
    UTEST_ASSERT_STRING_EQUAL(buffer8,"11111111");
    UTEST_ASSERT_TRUE(uint_to_string8(buffer8,unsigned_number,sizeof(buffer8),8));
    UTEST_ASSERT_STRING_EQUAL(buffer8,"377");
    
    uintn size_needed = uint_buffer_size(unsigned_number,2);
    UTEST_ASSERT_EQUAL(size_needed,9);
    UTEST_ASSERT_TRUE(uint_to_string8(buffer8,unsigned_number,size_needed,2));
    
    UTEST_ASSERT_TRUE(string_to_uint8(&unsigned_number,"FFFFFFFF",16));
    UTEST_ASSERT_EQUAL(unsigned_number,0xFFFFFFFF);
    UTEST_ASSERT_TRUE(uint_to_string8(buffer8,unsigned_number,sizeof(buffer8),16));
    UTEST_ASSERT_STRING_EQUAL(buffer8,"FFFFFFFF");

    UTEST_ASSERT_TRUE(string_to_int(&signed_number,"999"));
    UTEST_ASSERT_EQUAL(signed_number,999);
    
    UTEST_ASSERT_TRUE(string_to_uint(&unsigned_number,"ABC",16));
    UTEST_ASSERT_EQUAL(unsigned_number,0xABC);
    
    UTEST_ASSERT_TRUE(int_to_string(buffer8,signed_number,sizeof(buffer8)));
    UTEST_ASSERT_STRING_EQUAL(buffer8,"999");
    
    UTEST_ASSERT_TRUE(uint_to_string(buffer8,unsigned_number,sizeof(buffer8),16));
    UTEST_ASSERT_STRING_EQUAL(buffer8,"ABC");
}

/**
 * 测试各种错误情况。
 * 
 * @return 无返回值。
 */
UTEST_CASE(convert_error_cases)
{
    int64 signed_number;
    uint64 unsigned_number;
    char8 buffer8[16];
    
    UTEST_ASSERT_FALSE(string_to_int8(null,"123"));
    UTEST_ASSERT_FALSE(string_to_int8(&signed_number,null));
    
    UTEST_ASSERT_FALSE(string_to_int8(&signed_number,"abc"));
    UTEST_ASSERT_FALSE(string_to_int8(&signed_number,"123abc"));
    UTEST_ASSERT_FALSE(string_to_int8(&signed_number,"12.3"));
    UTEST_ASSERT_FALSE(string_to_int8(&signed_number,"12-3"));
    UTEST_ASSERT_FALSE(string_to_int8(&signed_number,"--123"));
    UTEST_ASSERT_FALSE(string_to_int8(&signed_number,"++123"));
    UTEST_ASSERT_FALSE(string_to_int8(&signed_number,"-+123"));
    UTEST_ASSERT_FALSE(string_to_int8(&signed_number,"+-123"));
    
    UTEST_ASSERT_FALSE(string_to_uint8(&unsigned_number,"G",16));
    UTEST_ASSERT_FALSE(string_to_uint8(&unsigned_number,"1G2",16));
    UTEST_ASSERT_FALSE(string_to_uint8(&unsigned_number,"2",2));
    UTEST_ASSERT_FALSE(string_to_uint8(&unsigned_number,"8",8));
    
    UTEST_ASSERT_FALSE(int_to_string8(buffer8,1234567890,5));
    UTEST_ASSERT_FALSE(uint_to_string8(buffer8,0xFFFFFFFF,5,16));
    
    UTEST_ASSERT_FALSE(int_to_string8(buffer8,123,0));
    UTEST_ASSERT_FALSE(uint_to_string8(buffer8,123,0,10));
    
    UTEST_ASSERT_FALSE(int_to_string8(buffer8,1000,4));
    UTEST_ASSERT_FALSE(uint_to_string8(buffer8,255,3,10));
    
    UTEST_ASSERT_FALSE(string_to_uint8(&unsigned_number,"-123",10));
    
    UTEST_ASSERT_FALSE(string_to_int8(&signed_number,"0123a"));
    UTEST_ASSERT_FALSE(string_to_uint8(&unsigned_number,"0xFF",10));
    
    UTEST_ASSERT_FALSE(string_to_int8(&signed_number,""));
    UTEST_ASSERT_FALSE(string_to_int8(&signed_number,"   "));
    UTEST_ASSERT_FALSE(string_to_int8(&signed_number,"\t\n\r "));
    
    UTEST_ASSERT_FALSE(string_to_int8(&signed_number,"123-"));
    UTEST_ASSERT_FALSE(string_to_int8(&signed_number,"12+3"));
    UTEST_ASSERT_FALSE(string_to_int8(&signed_number,"1-23"));
}

/**
 * 转换库函数测试。
 * 
 * @return 失败测试数。
 */
int32 convert_test(void)
{
    UTEST_SUITE("aos.kernel.test.support.convert");
    
    UTEST_RUN(string_to_int8_basic);
    UTEST_RUN(string_to_int16_basic);
    UTEST_RUN(string_to_int32_basic);
    UTEST_RUN(string_to_int_macro);
    
    UTEST_RUN(string_to_uint8_basic);
    UTEST_RUN(string_to_uint16_basic);
    UTEST_RUN(string_to_uint32_basic);
    UTEST_RUN(string_to_uint_macro);
    UTEST_RUN(string_to_uint_multiscale);
    
    UTEST_RUN(int_to_string8_basic);
    UTEST_RUN(int_to_string16_basic);
    UTEST_RUN(int_to_string32_basic);
    UTEST_RUN(int_to_string_macro);
    
    UTEST_RUN(uint_to_string8_basic);
    UTEST_RUN(uint_to_string16_basic);
    UTEST_RUN(uint_to_string32_basic);
    UTEST_RUN(uint_to_string_macro);
    UTEST_RUN(uint_to_string_multiscale);
    
    UTEST_RUN(int_buffer_size_test);
    UTEST_RUN(uint_buffer_size_test);
    
    UTEST_RUN(convert_edge_cases);
    UTEST_RUN(convert_integration_test);
    UTEST_RUN(convert_error_cases);
    
    UTEST_SUMMARY("aos.kernel.test.support.convert");
}