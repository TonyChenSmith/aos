/**
 * 内核格式化输出库函数测试。
 * @date 2026-04-17
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include <test/utest.h>
#include <support/format.h>

/**
 * 测试基本格式化字符串输出（无格式说明符）。
 * 
 * @return 无返回值。
 */
UTEST_CASE(format_string_basic)
{
    char8 buffer[256];
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"Hello,world!"),13);
    UTEST_ASSERT_STRING_EQUAL(buffer,"Hello,world!");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),""),1);
    UTEST_ASSERT_STRING_EQUAL(buffer,"");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,6,"12345"),6);
    UTEST_ASSERT_STRING_EQUAL(buffer,"12345");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,5,"12345"),5);
    UTEST_ASSERT_STRING_EQUAL(buffer,"1234");
    
    UTEST_ASSERT_EQUAL(format_string(null,100,"test"),0);
    UTEST_ASSERT_EQUAL(format_string(buffer,0,"test"),0);
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),null),0);
}

/**
 * 测试字符格式化（%c）。
 * 
 * @return 无返回值。
 */
UTEST_CASE(format_char)
{
    char8 buffer[256];
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%c",(char32)'A'),2);
    UTEST_ASSERT_STRING_EQUAL(buffer,"A");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%c%c",(char32)'1',(char32)'2'),3);
    UTEST_ASSERT_STRING_EQUAL(buffer,"12");
    
    /*字符α*/
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%c",(char32)0x03B1),3);
    UTEST_ASSERT_STRING_EQUAL(buffer,"\xCE\xB1");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%c%c%c",(char32)'X',(char32)'Y',(char32)'Z'),
        4);
    UTEST_ASSERT_STRING_EQUAL(buffer,"XYZ");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"Char: %c!",(char32)'Q'),9);
    UTEST_ASSERT_STRING_EQUAL(buffer,"Char: Q!");
}

/**
 * 测试字符串格式化（%s）。
 * 
 * @return 无返回值。
 */
UTEST_CASE(format_string_specifier)
{
    char8 buffer[256];
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%s","Hello"),6);
    UTEST_ASSERT_STRING_EQUAL(buffer,"Hello");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%s",""),1);
    UTEST_ASSERT_STRING_EQUAL(buffer,"");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%s",null),1);
    UTEST_ASSERT_STRING_EQUAL(buffer,"");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%s %s","Hello","World"),12);
    UTEST_ASSERT_STRING_EQUAL(buffer,"Hello World");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"Text: %s end","middle"),17);
    UTEST_ASSERT_STRING_EQUAL(buffer,"Text: middle end");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%10s","abc"),11);
    UTEST_ASSERT_STRING_EQUAL(buffer,"       abc");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%-10s","abc"),11);
    UTEST_ASSERT_STRING_EQUAL(buffer,"abc       ");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%.3s","abcdef"),4);
    UTEST_ASSERT_STRING_EQUAL(buffer,"abc");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%.0s","abcdef"),7);
    UTEST_ASSERT_STRING_EQUAL(buffer,"abcdef");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%8.3s","abcdef"),9);
    UTEST_ASSERT_STRING_EQUAL(buffer,"     abc");
}

/**
 * 测试32位有符号整数格式化（%i）。
 * 
 * @return 无返回值。
 */
UTEST_CASE(format_int32)
{
    char8 buffer[256];
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%i",42),3);
    UTEST_ASSERT_STRING_EQUAL(buffer,"42");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%i",-123),5);
    UTEST_ASSERT_STRING_EQUAL(buffer,"-123");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%i",0),2);
    UTEST_ASSERT_STRING_EQUAL(buffer,"0");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%i",2147483647),11);
    UTEST_ASSERT_STRING_EQUAL(buffer,"2147483647");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%i",-2147483648),12);
    UTEST_ASSERT_STRING_EQUAL(buffer,"-2147483648");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%5i",123),6);
    UTEST_ASSERT_STRING_EQUAL(buffer,"  123");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%5i",-123),6);
    UTEST_ASSERT_STRING_EQUAL(buffer," -123");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%-5i",123),6);
    UTEST_ASSERT_STRING_EQUAL(buffer,"123  ");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%.5i",123),6);
    UTEST_ASSERT_STRING_EQUAL(buffer,"00123");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%.5i",-123),7);
    UTEST_ASSERT_STRING_EQUAL(buffer,"-00123");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%.0i",0),2);
    UTEST_ASSERT_STRING_EQUAL(buffer,"0");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%.0i",123),4);
    UTEST_ASSERT_STRING_EQUAL(buffer,"123");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%8.5i",123),9);
    UTEST_ASSERT_STRING_EQUAL(buffer,"   00123");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%i + %i = %i",5,3,8),10);
    UTEST_ASSERT_STRING_EQUAL(buffer,"5 + 3 = 8");
}

/**
 * 测试64位有符号整数格式化（%I）。
 * 
 * @return 无返回值。
 */
UTEST_CASE(format_int64)
{
    char8 buffer[256];
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%I",9223372036854775807LL),20);
    UTEST_ASSERT_STRING_EQUAL(buffer,"9223372036854775807");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%I",-9223372036854775807LL),21);
    UTEST_ASSERT_STRING_EQUAL(buffer,"-9223372036854775807");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%I",1000000000000LL),14);
    UTEST_ASSERT_STRING_EQUAL(buffer,"1000000000000");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%20I",12345LL),21);
    UTEST_ASSERT_STRING_EQUAL(buffer,"               12345");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%-20I",12345LL),21);
    UTEST_ASSERT_STRING_EQUAL(buffer,"12345               ");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%.10I",12345LL),11);
    UTEST_ASSERT_STRING_EQUAL(buffer,"0000012345");
}

/**
 * 测试32位无符号整数格式化（%u）。
 * 
 * @return 无返回值。
 */
UTEST_CASE(format_uint32)
{
    char8 buffer[256];
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%u",4294967295U),11);
    UTEST_ASSERT_STRING_EQUAL(buffer,"4294967295");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%u",0U),2);
    UTEST_ASSERT_STRING_EQUAL(buffer,"0");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%10u",123U),11);
    UTEST_ASSERT_STRING_EQUAL(buffer,"       123");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%-10u",123U),11);
    UTEST_ASSERT_STRING_EQUAL(buffer,"123       ");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%.5u",123U),6);
    UTEST_ASSERT_STRING_EQUAL(buffer,"00123");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%12.8u",12345U),13);
    UTEST_ASSERT_STRING_EQUAL(buffer,"    00012345");
}

/**
 * 测试64位无符号整数格式化（%U）。
 * 
 * @return 无返回值。
 */
UTEST_CASE(format_uint64)
{
    char8 buffer[256];
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%U",18446744073709551615ULL),21);
    UTEST_ASSERT_STRING_EQUAL(buffer,"18446744073709551615");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%U",1000000000000000ULL),17);
    UTEST_ASSERT_STRING_EQUAL(buffer,"1000000000000000");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%25U",123456789ULL),26);
    UTEST_ASSERT_STRING_EQUAL(buffer,"                123456789");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%.15U",123456789ULL),16);
    UTEST_ASSERT_STRING_EQUAL(buffer,"000000123456789");
}

/**
 * 测试十六进制格式化（%h,%H,%x,%X）。
 * 
 * @return 无返回值。
 */
UTEST_CASE(format_hex)
{
    char8 buffer[256];
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%h",0xDEADBEEF),9);
    UTEST_ASSERT_STRING_EQUAL(buffer,"deadbeef");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%h",0x123ABC),7);
    UTEST_ASSERT_STRING_EQUAL(buffer,"123abc");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%H",0xDEADBEEF),9);
    UTEST_ASSERT_STRING_EQUAL(buffer,"DEADBEEF");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%x",0x123456789ABCDEFULL),16);
    UTEST_ASSERT_STRING_EQUAL(buffer,"123456789abcdef");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%X",0x123456789ABCDEFULL),16);
    UTEST_ASSERT_STRING_EQUAL(buffer,"123456789ABCDEF");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%10h",0xABC),11);
    UTEST_ASSERT_STRING_EQUAL(buffer,"       abc");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%-10h",0xABC),11);
    UTEST_ASSERT_STRING_EQUAL(buffer,"abc       ");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%.8h",0xABC),9);
    UTEST_ASSERT_STRING_EQUAL(buffer,"00000abc");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%h",0),2);
    UTEST_ASSERT_STRING_EQUAL(buffer,"0");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%.4h",0),5);
    UTEST_ASSERT_STRING_EQUAL(buffer,"0000");
}

/**
 * 测试指针格式化（%p,%P,%n,%N）。
 * 
 * @return 无返回值。
 */
UTEST_CASE(format_pointer)
{
    char8 buffer[256];
    uintn ptr_val=0x12345678;
    void* ptr=(void*)ptr_val;
    
    uintn hex_digits=sizeof(uintn)*2;
    uintn expected_len=hex_digits + 1;
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%p",ptr),expected_len);
    for (uintn i=0;i<hex_digits-8;i++) {
        UTEST_ASSERT_EQUAL(buffer[i],'0');
    }
    UTEST_ASSERT_STRING_EQUAL(buffer+hex_digits-8,"12345678");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%P",ptr),expected_len);
    for (uintn i=0;i<hex_digits-8;i++) {
        UTEST_ASSERT_EQUAL(buffer[i],'0');
    }
    UTEST_ASSERT_STRING_EQUAL(buffer+hex_digits-8,"12345678");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%n",(intn)ptr_val),10);
    UTEST_ASSERT_STRING_EQUAL(buffer,"305419896");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%N",ptr_val),10);
    UTEST_ASSERT_STRING_EQUAL(buffer,"305419896");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%p",null),expected_len);
    for (uintn i=0;i<hex_digits;i++) {
        UTEST_ASSERT_EQUAL(buffer[i],'0');
    }
}

/**
 * 测试二进制格式化（%b,%B）。
 * 
 * @return 无返回值。
 */
UTEST_CASE(format_binary)
{
    char8 buffer[256];
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%b",0b1101),5);
    UTEST_ASSERT_STRING_EQUAL(buffer,"1101");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%b",0b101010),7);
    UTEST_ASSERT_STRING_EQUAL(buffer,"101010");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%B",0b1101ULL),5);
    UTEST_ASSERT_STRING_EQUAL(buffer,"1101");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%10b",0b101),11);
    UTEST_ASSERT_STRING_EQUAL(buffer,"       101");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%.8b",0b101),9);
    UTEST_ASSERT_STRING_EQUAL(buffer,"00000101");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%b",0),2);
    UTEST_ASSERT_STRING_EQUAL(buffer,"0");
}

/**
 * 测试百分号转义（%%）。
 * 
 * @return 无返回值。
 */
UTEST_CASE(format_percent)
{
    char8 buffer[256];
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%%"),2);
    UTEST_ASSERT_STRING_EQUAL(buffer,"%");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"100%% pure"),10);
    UTEST_ASSERT_STRING_EQUAL(buffer,"100% pure");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%%%"),2);
    UTEST_ASSERT_STRING_EQUAL(buffer,"%");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%%i%%",42),4);
    UTEST_ASSERT_STRING_EQUAL(buffer,"%i%");
}

/**
 * 测试未识别的格式说明符（应直接输出字符）。
 * 
 * @return 无返回值。
 */
UTEST_CASE(format_unknown_specifier)
{
    char8 buffer[256];
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%z"),2);
    UTEST_ASSERT_STRING_EQUAL(buffer,"z");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%a%b%c",2,(char32)'C'),5);
    UTEST_ASSERT_STRING_EQUAL(buffer,"a10C");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"Test %q End",123),11);
    UTEST_ASSERT_STRING_EQUAL(buffer,"Test q End");
}

/**
 * 测试复杂格式化组合。
 * 
 * @return 无返回值。
 */
UTEST_CASE(format_complex)
{
    char8 buffer[256];
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"Int: %i,Str: %s,Char: %c",42,
        "test",(char32)'X'),26);
    UTEST_ASSERT_STRING_EQUAL(buffer,"Int: 42,Str: test,Char: X");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%8s: %-5i","Value",123),16);
    UTEST_ASSERT_STRING_EQUAL(buffer,"   Value: 123  ");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"Hex: 0x%08h,Dec: %010i",0xABCD,-123),32);
    UTEST_ASSERT_STRING_EQUAL(buffer,"Hex: 0x    abcd,Dec:       -123");
    
    const char8* name="User";
    int32 score=95;
    uint32 id=1001;
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),
        "Name: %-10s | Score: %3i | ID: %05u | Hex: 0x%04h",name,score,id,id),56);
    UTEST_ASSERT_STRING_EQUAL(buffer,"Name: User       | Score:  95 | ID:  1001 | Hex: 0x 3e9");
}

/**
 * 测试边界和错误情况。
 * 
 * @return 无返回值。
 */
UTEST_CASE(format_edge_cases)
{
    char8 buffer[256];
    
    char8 small[5];
    UTEST_ASSERT_EQUAL(format_string(small,sizeof(small),"Hello"),5);
    UTEST_ASSERT_STRING_EQUAL(small,"Hell");
    
    UTEST_ASSERT_EQUAL(format_string(small,sizeof(small),"%i",12345),5);
    UTEST_ASSERT_STRING_EQUAL(small,"1234");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%100i",1),101);

    uintn spaces=0;
    while(buffer[spaces]==' ')
    {
        spaces++;
    }
    UTEST_ASSERT_EQUAL(spaces,99);
    UTEST_ASSERT_STRING_EQUAL(buffer+spaces,"1");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%.0i",0),2);
    UTEST_ASSERT_STRING_EQUAL(buffer,"0");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%.0i",123),4);
    UTEST_ASSERT_STRING_EQUAL(buffer,"123");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%0i",456),4);
    UTEST_ASSERT_STRING_EQUAL(buffer,"456");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),""),1);
    UTEST_ASSERT_STRING_EQUAL(buffer,"");
}

/**
 * 测试可变参数版本（format_string_valist）。
 * 注：由于直接测试va_list较复杂，这里通过包装函数测试。
 * 
 * @return 无返回值。
 */
UTEST_CASE(format_valist_wrapper)
{
    char8 buffer[256];
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%s %i %c","Test",42,(char32)'A'),10);
    UTEST_ASSERT_STRING_EQUAL(buffer,"Test 42 A");
    
    UTEST_ASSERT_EQUAL(format_string(buffer,sizeof(buffer),"%i %u %h %s",-123,456U,0xABC,"end"),17);
    UTEST_ASSERT_STRING_EQUAL(buffer,"-123 456 abc end");
}

/**
 * 格式化输出库函数测试。
 * 
 * @return 失败测试数。
 */
int32 format_test(void)
{
    UTEST_SUITE("aos.kernel.test.support.format");
    
    UTEST_RUN(format_string_basic);
    UTEST_RUN(format_char);
    UTEST_RUN(format_string_specifier);
    UTEST_RUN(format_int32);
    UTEST_RUN(format_int64);
    UTEST_RUN(format_uint32);
    UTEST_RUN(format_uint64);
    UTEST_RUN(format_hex);
    UTEST_RUN(format_pointer);
    UTEST_RUN(format_binary);
    UTEST_RUN(format_percent);
    UTEST_RUN(format_unknown_specifier);
    UTEST_RUN(format_complex);
    UTEST_RUN(format_edge_cases);
    UTEST_RUN(format_valist_wrapper);
    
    UTEST_SUMMARY("aos.kernel.test.support.format");
}