/**
 * 内核字符串操作库函数测试。
 * @date 2026-01-19
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include <test/utest.h>

#include <support/string.h>

/**
 * 测试8位字符串长度函数。
 * 
 * @return 无返回值。
 */
UTEST_CASE(string_length8_basic)
{
    UTEST_ASSERT_EQUAL(string_length8(""),0);
    UTEST_ASSERT_EQUAL(string_length8("a"),1);
    UTEST_ASSERT_EQUAL(string_length8("hello"),5);
    UTEST_ASSERT_EQUAL(string_length8("hello world"),11);
    
    const char8* null_term_string="test\0hidden";
    UTEST_ASSERT_EQUAL(string_length8(null_term_string),4);
}

/**
 * 测试16位字符串长度函数。
 * 
 * @return 无返回值。
 */
UTEST_CASE(string_length16_basic)
{
    UTEST_ASSERT_EQUAL(string_length16(u""),0);
    UTEST_ASSERT_EQUAL(string_length16(u"a"),1);
    UTEST_ASSERT_EQUAL(string_length16(u"hello"),5);
    UTEST_ASSERT_EQUAL(string_length16(u"hello world"),11);
    
    const char16* null_term_string=u"test\0hidden";
    UTEST_ASSERT_EQUAL(string_length16(null_term_string),4);
}

/**
 * 测试32位字符串长度函数。
 * 
 * @return 无返回值。
 */
UTEST_CASE(string_length32_basic)
{
    UTEST_ASSERT_EQUAL(string_length32(U""),0);
    UTEST_ASSERT_EQUAL(string_length32(U"a"),1);
    UTEST_ASSERT_EQUAL(string_length32(U"hello"),5);
    UTEST_ASSERT_EQUAL(string_length32(U"hello world"),11);
    
    const char32* null_term_string=U"test\0hidden";
    UTEST_ASSERT_EQUAL(string_length32(null_term_string),4);
}

/**
 * 测试字符串长度泛型宏。
 * 
 * @return 无返回值。
 */
UTEST_CASE(string_length_generic_macros)
{
    const char8* s8="Hello";
    const char16* s16=u"Hello";
    const char32* s32=U"Hello";
    
    UTEST_ASSERT_EQUAL(string_length(s8),5);
    UTEST_ASSERT_EQUAL(string_length(s16),5);
    UTEST_ASSERT_EQUAL(string_length(s32),5);
    
    const char8* s8_empty="";
    const char16* s16_empty=u"";
    const char32* s32_empty=U"";
    
    UTEST_ASSERT_EQUAL(string_length(s8_empty),0);
    UTEST_ASSERT_EQUAL(string_length(s16_empty),0);
    UTEST_ASSERT_EQUAL(string_length(s32_empty),0);
}

/**
 * 测试8位字符串码点计数函数。
 * 
 * @return 无返回值。
 */
UTEST_CASE(string_codepoint_count8_basic)
{
    UTEST_ASSERT_EQUAL(string_codepoint_count8(""),0);
    UTEST_ASSERT_EQUAL(string_codepoint_count8("a"),1);
    UTEST_ASSERT_EQUAL(string_codepoint_count8("hello"),5);
    
    const char8 copyright[]={0xC2,0xA9,0};
    UTEST_ASSERT_EQUAL(string_codepoint_count8(copyright),1);
    
    const char8 euro[]={0xE2,0x82,0xAC,0};
    UTEST_ASSERT_EQUAL(string_codepoint_count8(euro),1);
    
    const char8 old_italic_letter[]={0xF0,0x90,0x8D,0x88,0};
    UTEST_ASSERT_EQUAL(string_codepoint_count8(old_italic_letter),1);
    
    const char8 mixed[]={'H','e','l','l','o',' ',0xC2,0xA9,' ',0xE2,0x82,
        0xAC,0};
    UTEST_ASSERT_EQUAL(string_codepoint_count8(mixed),9);
}

/**
 * 测试16位字符串码点计数函数。
 * 
 * @return 无返回值。
 */
UTEST_CASE(string_codepoint_count16_basic)
{
    const char16 empty[]={0};
    UTEST_ASSERT_EQUAL(string_codepoint_count16(empty),0);
    
    const char16 bmp[]={'H','e','l','l','o',0x00A9,' ',0x20AC,0};
    UTEST_ASSERT_EQUAL(string_codepoint_count16(bmp),8);
    
    const char16 emoji[]={0xD83D,0xDE00,0xD83D,0xDE01,0xD83D,0xDE02,0};
    UTEST_ASSERT_EQUAL(string_codepoint_count16(emoji),3);
}

/**
 * 测试32位字符串码点计数函数。
 * 
 * @return 无返回值。
 */
UTEST_CASE(string_codepoint_count32_basic)
{
    const char32 empty[]={0};
    UTEST_ASSERT_EQUAL(string_codepoint_count32(empty),0);
    
    const char32 simple[]={'H','e','l','l','o',0x00A9,' ',0x20AC,0};
    UTEST_ASSERT_EQUAL(string_codepoint_count32(simple),8);
    
    const char32 emoji[]={0x1F600,0x1F601,0x1F602,0};
    UTEST_ASSERT_EQUAL(string_codepoint_count32(emoji),3);
}

/**
 * 测试字符串编码验证函数。
 * 
 * @return 无返回值。
 */
UTEST_CASE(string_valid_basic)
{
    UTEST_ASSERT_TRUE(string_valid8("Hello"));
    
    const char8 invalid_utf8[]={0xC2,0};
    UTEST_ASSERT_FALSE(string_valid8(invalid_utf8));
    
    const char8 invalid_utf8_2[]={0x80,0};
    UTEST_ASSERT_FALSE(string_valid8(invalid_utf8_2));
    
    const char16 valid_utf16[]=u"Hello";
    UTEST_ASSERT_TRUE(string_valid16(valid_utf16));
    
    const char16 invalid_utf16[]={0xD83D,0};
    UTEST_ASSERT_FALSE(string_valid16(invalid_utf16));
    
    const char32 valid_utf32[]=U"Hello";
    UTEST_ASSERT_TRUE(string_valid32(valid_utf32));
    
    const char32 invalid_utf32[]={0x110000,0};
    UTEST_ASSERT_FALSE(string_valid32(invalid_utf32));
}

/**
 * 测试字符串转换函数。
 * 
 * @return 无返回值。
 */
UTEST_CASE(string_convert_basic)
{
    char8 dest8[100];
    const char8* src8="Hello © €";
    uintn result=string_convert8_char8(dest8,src8,100);
    UTEST_ASSERT_TRUE(result>0);
    UTEST_ASSERT_STRING_EQUAL(dest8,src8);
    
    const char16 src16[]={'H','e','l','l','o',' ',0x00A9,' ',0x20AC,0};
    result=string_convert8_char16(dest8,src16,100);
    UTEST_ASSERT_TRUE(result>0);
    
    const char32 src32[]={'H','e','l','l','o',' ',0x00A9,' ',0x20AC,0};
    result=string_convert8_char32(dest8,src32,100);
    UTEST_ASSERT_TRUE(result>0);
    
    char8 small_buf[5];
    result=string_convert8_char8(small_buf,"Hello World",5);
    UTEST_ASSERT_TRUE(result>5);
}

/**
 * 测试字符串连接函数。
 * 
 * @return 无返回值。
 */
UTEST_CASE(string_concat_basic)
{
    char8 str8[50]="Hello";
    const char8* append8=" World!";
    uintn result=string_concat8(str8,append8,50);
    UTEST_ASSERT_EQUAL(result,13);
    UTEST_ASSERT_STRING_EQUAL(str8,"Hello World!");
    
    char16 str16[50]=u"Hello";
    const char16* append16=u" World!";
    result=string_concat16(str16,append16,50);
    UTEST_ASSERT_EQUAL(result,13);
    
    char32 str32[50]=U"Hello";
    const char32* append32=U" World!";
    result=string_concat32(str32,append32,50);
    UTEST_ASSERT_EQUAL(result,13);
    
    char8 small_str[10]="Hello";
    result=string_concat8(small_str," World!",10);
    UTEST_ASSERT_TRUE(result>10);
}

/**
 * 测试字符串复制函数。
 * 
 * @return 无返回值。
 */
UTEST_CASE(string_copy_basic)
{
    char8 dest8[50];
    const char8* src8="Hello World!";
    uintn result=string_copy8(dest8,src8,50);
    UTEST_ASSERT_EQUAL(result,13);
    UTEST_ASSERT_STRING_EQUAL(dest8,src8);
    
    char16 dest16[50];
    const char16* src16=u"Hello World!";
    result=string_copy16(dest16,src16,50);
    UTEST_ASSERT_EQUAL(result,13);
    
    char32 dest32[50];
    const char32* src32=U"Hello World!";
    result=string_copy32(dest32,src32,50);
    UTEST_ASSERT_EQUAL(result,13);
    
    result=string_copy8(dest8,"",50);
    UTEST_ASSERT_EQUAL(result,1);
    UTEST_ASSERT_EQUAL(dest8[0],0);
    
    char8 small_dest[5];
    result=string_copy8(small_dest,"Hello World",5);
    UTEST_ASSERT_TRUE(result>5);
}

/**
 * 测试字符串查找函数。
 * 
 * @return 无返回值。
 */
UTEST_CASE(string_find_basic)
{
    const char8* str8="Hello World! Hello Universe!";
    const char8* target8="World";
    const char8* result8=string_find8(str8,target8);
    UTEST_ASSERT_NOT_NULL(result8);
    UTEST_ASSERT_EQUAL(result8-str8,6);
    
    const char16* str16=u"Hello World! Hello Universe!";
    const char16* target16=u"World";
    const char16* result16=string_find16(str16,target16);
    UTEST_ASSERT_NOT_NULL(result16);
    UTEST_ASSERT_EQUAL(result16-str16,6);
    
    const char32* str32=U"Hello World! Hello Universe!";
    const char32* target32=U"World";
    const char32* result32=string_find32(str32,target32);
    UTEST_ASSERT_NOT_NULL(result32);
    UTEST_ASSERT_EQUAL(result32-str32,6);
    
    UTEST_ASSERT_NULL(string_find8("Hello","XYZ"));
    UTEST_ASSERT_NULL(string_find8("","Hello"));
    UTEST_ASSERT_NULL(string_find8("Hello",""));
}

/**
 * 测试反向字符串查找函数。
 * 
 * @return 无返回值。
 */
UTEST_CASE(string_reverse_find_basic)
{
    const char8* str8="Hello World! Hello World!";
    const char8* target8="World";
    const char8* result8=string_reverse_find8(str8,target8);
    UTEST_ASSERT_NOT_NULL(result8);
    UTEST_ASSERT_EQUAL(result8-str8,19);
    
    const char16* str16=u"Hello World! Hello World!";
    const char16* target16=u"World";
    const char16* result16 = string_reverse_find16(str16,target16);
    UTEST_ASSERT_NOT_NULL(result16);
    UTEST_ASSERT_EQUAL(result16-str16,19);

    const char32* str32=U"Hello World! Hello World!";
    const char32* target32=U"World";
    const char32* result32 = string_reverse_find32(str32,target32);
    UTEST_ASSERT_NOT_NULL(result32);
    UTEST_ASSERT_EQUAL(result32-str32,19);
    
    UTEST_ASSERT_NULL(string_reverse_find8("Hello","XYZ"));
    UTEST_ASSERT_NULL(string_reverse_find8("","Hello"));
}

/**
 * 测试字符查找函数。
 * 
 * @return 无返回值。
 */
UTEST_CASE(string_find_char_basic)
{
    const char8* str8="Hello World!";
    const char8* result8=string_find_char8(str8,'o');
    UTEST_ASSERT_NOT_NULL(result8);
    UTEST_ASSERT_EQUAL(result8-str8,4);
    
    const char16* str16=u"Hello World!";
    const char16* result16=string_find_char16(str16,'o');
    UTEST_ASSERT_NOT_NULL(result16);
    UTEST_ASSERT_EQUAL(result16-str16,4);
    
    const char32* str32=U"Hello World!";
    const char32* result32=string_find_char32(str32,'o');
    UTEST_ASSERT_NOT_NULL(result32);
    UTEST_ASSERT_EQUAL(result32-str32,4);
    
    UTEST_ASSERT_NULL(string_find_char8("Hello",'x'));
    UTEST_ASSERT_NULL(string_find_char8("",'x'));
}

/**
 * 测试反向字符查找函数。
 * 
 * @return 无返回值。
 */
UTEST_CASE(string_reverse_find_char_basic)
{
    const char8* str8="Hello World!";
    const char8* result8=string_reverse_find_char8(str8,'o');
    UTEST_ASSERT_NOT_NULL(result8);
    UTEST_ASSERT_EQUAL(result8-str8,7);
    
    const char16* str16=u"Hello World!";
    const char16* result16=string_reverse_find_char16(str16,'o');
    UTEST_ASSERT_NOT_NULL(result16);
    UTEST_ASSERT_EQUAL(result16-str16,7);

    const char32* str32=U"Hello World!";
    const char32* result32=string_reverse_find_char32(str32,'o');
    UTEST_ASSERT_NOT_NULL(result32);
    UTEST_ASSERT_EQUAL(result32-str32,7);
    
    UTEST_ASSERT_NULL(string_reverse_find_char8("Hello",'x'));
    UTEST_ASSERT_NULL(string_reverse_find_char8("",'x'));
}

/**
 * 测试字符串比较函数。
 * 
 * @return 无返回值。
 */
UTEST_CASE(string_compare_basic)
{
    UTEST_ASSERT_EQUAL(string_compare8("",""),0);
    UTEST_ASSERT_EQUAL(string_compare8("Hello","Hello"),0);
    UTEST_ASSERT_TRUE(string_compare8("Apple","Banana")<0);
    UTEST_ASSERT_TRUE(string_compare8("Banana","Apple")>0);
    UTEST_ASSERT_TRUE(string_compare8("hello","Hello")>0);
    UTEST_ASSERT_TRUE(string_compare8("Hello","hello")<0);
    
    const char16* str1_16=u"Hello";
    const char16* str2_16=u"Hello";
    const char16* str3_16=u"Apple";
    const char16* str4_16=u"Banana";
    
    UTEST_ASSERT_EQUAL(string_compare16(str1_16,str2_16),0);
    UTEST_ASSERT_TRUE(string_compare16(str3_16,str4_16)<0);
    UTEST_ASSERT_TRUE(string_compare16(str4_16,str3_16)>0);
    
    const char32* str1_32=U"Hello";
    const char32* str2_32=U"Hello";
    const char32* str3_32=U"Apple";
    const char32* str4_32=U"Banana";
    
    UTEST_ASSERT_EQUAL(string_compare32(str1_32,str2_32),0);
    UTEST_ASSERT_TRUE(string_compare32(str3_32,str4_32)<0);
    UTEST_ASSERT_TRUE(string_compare32(str4_32,str3_32)>0);
    
    UTEST_ASSERT_TRUE(string_compare8("Hello","Hello World")<0);
    UTEST_ASSERT_TRUE(string_compare8("Hello World","Hello")>0);
}

/**
 * 测试子字符串函数。
 * 
 * @return 无返回值。
 */
UTEST_CASE(string_substring_basic)
{
    char8 dest8[50];
    const char8* src8="Hello World!";
    
    uintn result=string_substring8(dest8,src8,0,5,50);
    UTEST_ASSERT_EQUAL(result,5);
    UTEST_ASSERT_STRING_EQUAL(dest8,"Hello");
    
    result=string_substring8(dest8,src8,6,5,50);
    UTEST_ASSERT_EQUAL(result,5);
    UTEST_ASSERT_STRING_EQUAL(dest8,"World");
    
    result=string_substring8(dest8,src8,6,100,50);
    UTEST_ASSERT_EQUAL(result,6);
    UTEST_ASSERT_STRING_EQUAL(dest8,"World!");
    
    result=string_substring8(dest8,src8,100,5,50);
    UTEST_ASSERT_EQUAL(result,0);
    
    char16 dest16[50];
    const char16* src16=u"Hello World!";
    
    result=string_substring16(dest16,src16,0,5,50);
    UTEST_ASSERT_EQUAL(result,5);
    
    result=string_substring16(dest16,src16,6,5,50);
    UTEST_ASSERT_EQUAL(result,5);
    
    char8 small_dest[3];
    result=string_substring8(small_dest,"Hello",0,5,3);
    UTEST_ASSERT_EQUAL(result,2);
}

/**
 * 测试泛型宏的基本功能。
 * 
 * @return 无返回值。
 */
UTEST_CASE(string_generic_macros_basic)
{
    const char8* s8="Hello";
    const char16* s16=u"Hello";
    const char32* s32=U"Hello";
    
    UTEST_ASSERT_EQUAL(string_length(s8),5);
    UTEST_ASSERT_EQUAL(string_length(s16),5);
    UTEST_ASSERT_EQUAL(string_length(s32),5);
    
    const char8* a8="Apple";
    const char8* b8="Banana";
    const char16* a16=u"Apple";
    const char16* b16=u"Banana";
    const char32* a32=U"Apple";
    const char32* b32=U"Banana";
    
    UTEST_ASSERT_TRUE(string_compare(a8,b8)<0);
    UTEST_ASSERT_TRUE(string_compare(a16,b16)<0);
    UTEST_ASSERT_TRUE(string_compare(a32,b32)<0);
    
    UTEST_ASSERT_NOT_NULL(string_find_char(s8,'e'));
    UTEST_ASSERT_NOT_NULL(string_find_char(s16,'e'));
    UTEST_ASSERT_NOT_NULL(string_find_char(s32,'e'));
    
    UTEST_ASSERT_TRUE(string_valid(s8));
    UTEST_ASSERT_TRUE(string_valid(s16));
    UTEST_ASSERT_TRUE(string_valid(s32));
}

/**
 * 测试边缘情况和边界条件。
 * 
 * @return 无返回值。
 */
UTEST_CASE(string_edge_cases)
{
    UTEST_ASSERT_EQUAL(string_length8(""),0);
    UTEST_ASSERT_EQUAL(string_codepoint_count8(""),0);
    UTEST_ASSERT_TRUE(string_valid8(""));
    
    UTEST_ASSERT_EQUAL(string_length8("a"),1);
    UTEST_ASSERT_EQUAL(string_codepoint_count8("a"),1);
    
    const char8 null_str[]={0};
    UTEST_ASSERT_EQUAL(string_length8(null_str),0);
    
    const char8 invalid_utf8[]={0xC2,0};
    UTEST_ASSERT_FALSE(string_valid8(invalid_utf8));
    UTEST_ASSERT_EQUAL(string_codepoint_count8(invalid_utf8),0);
    
    const char8* very_long="This is a very long string that should still work correctly with our string functions.";
    UTEST_ASSERT_TRUE(string_length8(very_long)>0);
    UTEST_ASSERT_TRUE(string_valid8(very_long));
    
    const char8 mixed[]="ASCII: Hello, Unicode: © € 𐍈";
    UTEST_ASSERT_TRUE(string_length8(mixed)>0);
    UTEST_ASSERT_TRUE(string_valid8(mixed));
}

/**
 * 测试性能相关的基本操作。
 * 
 * @return 无返回值。
 */
UTEST_CASE(string_performance_basic)
{
    const char8* long_str="Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
        "Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
    uintn len=string_length8(long_str);
    UTEST_ASSERT_EQUAL(len,123);
    
    const char8* found=string_find8(long_str,"adipiscing");
    UTEST_ASSERT_NOT_NULL(found);
    
    const char8* similar_str="Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
        "Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
    UTEST_ASSERT_EQUAL(string_compare8(long_str,similar_str),0);
    
    for(uintn i=0;i<100;i++) {
        char8 small_str[20];
        string_copy8(small_str,"test",20);
        UTEST_ASSERT_STRING_EQUAL(small_str,"test");
    }
}

/**
 * 测试字符串操作的综合使用。
 * 
 * @return 无返回值。
 */
UTEST_CASE(string_integration_test)
{
    char8 buffer[100];
    const char8* original="Hello World!";
    
    uintn copied=string_copy8(buffer,original,100);
    UTEST_ASSERT_EQUAL(copied,13);
    UTEST_ASSERT_STRING_EQUAL(buffer,original);
    
    copied=string_concat8(buffer," Welcome to the test!",100);
    UTEST_ASSERT_TRUE(copied>12);
    UTEST_ASSERT_STRING_EQUAL(buffer,"Hello World! Welcome to the test!");
    
    const char8* found=string_find8(buffer,"Welcome");
    UTEST_ASSERT_NOT_NULL(found);
    
    char8 substring[50];
    uintn sub_len=string_substring8(substring,buffer,13,7,50);
    UTEST_ASSERT_EQUAL(sub_len,7);
    UTEST_ASSERT_STRING_EQUAL(substring,"Welcome");
    
    UTEST_ASSERT_EQUAL(string_compare8(buffer,"Hello World! Welcome to the test!"),0);
    UTEST_ASSERT_TRUE(string_compare8(buffer,"Hello")>0);
    
    UTEST_ASSERT_TRUE(string_valid8(buffer));
    
    uintn codepoints=string_codepoint_count8(buffer);
    UTEST_ASSERT_TRUE(codepoints>0);
    
    uintn length=string_length8(buffer);
    UTEST_ASSERT_EQUAL(length,33);
}

/**
 * 字符串库函数测试。
 * 
 * @return 失败测试数。
 */
int32 string_test()
{
    UTEST_SUITE("aos.kernel.test.support.string");
    
    UTEST_RUN(string_length8_basic);
    UTEST_RUN(string_length16_basic);
    UTEST_RUN(string_length32_basic);
    UTEST_RUN(string_length_generic_macros);
    UTEST_RUN(string_codepoint_count8_basic);
    UTEST_RUN(string_codepoint_count16_basic);
    UTEST_RUN(string_codepoint_count32_basic);
    UTEST_RUN(string_valid_basic);
    UTEST_RUN(string_convert_basic);
    UTEST_RUN(string_concat_basic);
    UTEST_RUN(string_copy_basic);
    UTEST_RUN(string_find_basic);
    UTEST_RUN(string_reverse_find_basic);
    UTEST_RUN(string_find_char_basic);
    UTEST_RUN(string_reverse_find_char_basic);
    UTEST_RUN(string_compare_basic);
    UTEST_RUN(string_substring_basic);
    UTEST_RUN(string_generic_macros_basic);

    UTEST_RUN(string_edge_cases);
    UTEST_RUN(string_performance_basic);
    UTEST_RUN(string_integration_test);
    
    UTEST_SUMMARY("aos.kernel.test.support.string");
}