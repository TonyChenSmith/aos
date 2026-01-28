/**
 * 内核字符操作库函数测试。
 * @date 2026-01-15
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include <test/utest.h>

#include <library/char.h>

/**
 * 测试ASCII字母判断函数。
 * 
 * @return 无返回值。
 */
UTEST_CASE(ascii_is_alphabetic_basic)
{
    UTEST_ASSERT_TRUE(ascii_is_alphabetic('A'));
    UTEST_ASSERT_TRUE(ascii_is_alphabetic('Z'));
    UTEST_ASSERT_TRUE(ascii_is_alphabetic('a'));
    UTEST_ASSERT_TRUE(ascii_is_alphabetic('z'));
    UTEST_ASSERT_FALSE(ascii_is_alphabetic('0'));
    UTEST_ASSERT_FALSE(ascii_is_alphabetic('9'));
    UTEST_ASSERT_FALSE(ascii_is_alphabetic('@'));
    UTEST_ASSERT_FALSE(ascii_is_alphabetic('['));
    
    UTEST_ASSERT_TRUE(ascii_is_alphabetic('M'));
    UTEST_ASSERT_TRUE(ascii_is_alphabetic('n'));
    UTEST_ASSERT_FALSE(ascii_is_alphabetic('5'));
    UTEST_ASSERT_FALSE(ascii_is_alphabetic('#'));
    
    UTEST_ASSERT_TRUE(ascii_is_alphabetic('G'));
    UTEST_ASSERT_TRUE(ascii_is_alphabetic('h'));
    UTEST_ASSERT_FALSE(ascii_is_alphabetic('7'));
    UTEST_ASSERT_FALSE(ascii_is_alphabetic('&'));
}

/**
 * 测试ASCII数字判断函数。
 * 
 * @return 无返回值。
 */
UTEST_CASE(ascii_is_digit_basic)
{
    for(char32 c='0';c<='9';c++)
    {
        UTEST_ASSERT_TRUE(ascii_is_digit(c));
    }
    
    UTEST_ASSERT_FALSE(ascii_is_digit('/'));
    UTEST_ASSERT_FALSE(ascii_is_digit(':'));
    UTEST_ASSERT_FALSE(ascii_is_digit('A'));
    UTEST_ASSERT_FALSE(ascii_is_digit('a'));
    
    UTEST_ASSERT_TRUE(ascii_is_digit('0'));
    UTEST_ASSERT_TRUE(ascii_is_digit('9'));
    UTEST_ASSERT_FALSE(ascii_is_digit('A'));
    
    UTEST_ASSERT_TRUE(ascii_is_digit('5'));
    UTEST_ASSERT_FALSE(ascii_is_digit('K'));
}

/**
 * 测试ASCII小写字母判断函数。
 * 
 * @return 无返回值。
 */
UTEST_CASE(ascii_is_lowercase_basic)
{
    for(char32 c='a';c<='z';c++)
    {
        UTEST_ASSERT_TRUE(ascii_is_lowercase(c));
    }
    
    for(char32 c='A';c<='Z';c++)
    {
        UTEST_ASSERT_FALSE(ascii_is_lowercase(c));
    }
    
    UTEST_ASSERT_FALSE(ascii_is_lowercase('@'));
    UTEST_ASSERT_FALSE(ascii_is_lowercase('['));
    UTEST_ASSERT_FALSE(ascii_is_lowercase('0'));
    
    UTEST_ASSERT_TRUE(ascii_is_lowercase('m'));
    UTEST_ASSERT_FALSE(ascii_is_lowercase('M'));
    
    UTEST_ASSERT_TRUE(ascii_is_lowercase('x'));
    UTEST_ASSERT_FALSE(ascii_is_lowercase('X'));
}

/**
 * 测试ASCII大写字母判断函数。
 * 
 * @return 无返回值。
 */
UTEST_CASE(ascii_is_uppercase_basic)
{
    for(char32 c='A';c<='Z';c++)
    {
        UTEST_ASSERT_TRUE(ascii_is_uppercase(c));
    }
    
    for(char32 c='a';c<='z';c++)
    {
        UTEST_ASSERT_FALSE(ascii_is_uppercase(c));
    }
    
    UTEST_ASSERT_FALSE(ascii_is_uppercase('@'));
    UTEST_ASSERT_FALSE(ascii_is_uppercase('['));
    UTEST_ASSERT_FALSE(ascii_is_uppercase('0'));
    
    UTEST_ASSERT_TRUE(ascii_is_uppercase('P'));
    UTEST_ASSERT_FALSE(ascii_is_uppercase('p'));
    
    UTEST_ASSERT_TRUE(ascii_is_uppercase('Q'));
    UTEST_ASSERT_FALSE(ascii_is_uppercase('q'));
}

/**
 * 测试ASCII空白字符判断函数。
 * 
 * @return 无返回值。
 */
UTEST_CASE(ascii_is_whitespace_basic)
{
    UTEST_ASSERT_TRUE(ascii_is_whitespace(' '));
    UTEST_ASSERT_TRUE(ascii_is_whitespace('\t'));
    UTEST_ASSERT_TRUE(ascii_is_whitespace('\n'));
    UTEST_ASSERT_TRUE(ascii_is_whitespace('\r'));
    UTEST_ASSERT_TRUE(ascii_is_whitespace('\f'));
    UTEST_ASSERT_TRUE(ascii_is_whitespace('\v'));
    
    UTEST_ASSERT_FALSE(ascii_is_whitespace('A'));
    UTEST_ASSERT_FALSE(ascii_is_whitespace('0'));
    UTEST_ASSERT_FALSE(ascii_is_whitespace('_'));
    
    UTEST_ASSERT_TRUE(ascii_is_whitespace(' '));
    UTEST_ASSERT_TRUE(ascii_is_whitespace('\t'));
    UTEST_ASSERT_FALSE(ascii_is_whitespace('B'));
    
    UTEST_ASSERT_TRUE(ascii_is_whitespace('\n'));
    UTEST_ASSERT_TRUE(ascii_is_whitespace('\r'));
    UTEST_ASSERT_FALSE(ascii_is_whitespace('C'));
}

/**
 * 测试ASCII空白分隔符判断函数。
 * 
 * @return 无返回值。
 */
UTEST_CASE(ascii_is_blank_basic)
{
    UTEST_ASSERT_TRUE(ascii_is_blank(' '));
    UTEST_ASSERT_TRUE(ascii_is_blank('\t'));
    
    UTEST_ASSERT_FALSE(ascii_is_blank('\n'));
    UTEST_ASSERT_FALSE(ascii_is_blank('\r'));
    UTEST_ASSERT_FALSE(ascii_is_blank('A'));
    UTEST_ASSERT_FALSE(ascii_is_blank('0'));
    
    UTEST_ASSERT_TRUE(ascii_is_blank(' '));
    UTEST_ASSERT_FALSE(ascii_is_blank('\n'));
    
    UTEST_ASSERT_TRUE(ascii_is_blank('\t'));
    UTEST_ASSERT_FALSE(ascii_is_blank('\r'));
}

/**
 * 测试ASCII控制字符判断函数。
 * 
 * @return 无返回值。
 */
UTEST_CASE(ascii_is_control_basic)
{
    for(char32 c=0;c<=0x1F;c++)
    {
        UTEST_ASSERT_TRUE(ascii_is_control(c));
    }
    
    UTEST_ASSERT_TRUE(ascii_is_control(0x7F));
    
    UTEST_ASSERT_FALSE(ascii_is_control(' '));
    UTEST_ASSERT_FALSE(ascii_is_control('A'));
    UTEST_ASSERT_FALSE(ascii_is_control('0'));
    UTEST_ASSERT_FALSE(ascii_is_control('~'));
    
    UTEST_ASSERT_TRUE(ascii_is_control('\0'));
    UTEST_ASSERT_TRUE(ascii_is_control(0x1F));
    UTEST_ASSERT_TRUE(ascii_is_control(0x7F));
    UTEST_ASSERT_FALSE(ascii_is_control(' '));
    
    UTEST_ASSERT_TRUE(ascii_is_control(0x08));
    UTEST_ASSERT_TRUE(ascii_is_control(0x7F));
    UTEST_ASSERT_FALSE(ascii_is_control('A'));
}

/**
 * 测试边界情况和特殊字符。
 * 
 * @return 无返回值。
 */
UTEST_CASE(ascii_edge_cases)
{
    UTEST_ASSERT_FALSE(ascii_is_alphabetic(0));
    UTEST_ASSERT_FALSE(ascii_is_alphabetic(127));
    UTEST_ASSERT_FALSE(ascii_is_alphabetic(128));
    
    UTEST_ASSERT_FALSE(ascii_is_digit(0xFF));
    UTEST_ASSERT_FALSE(ascii_is_lowercase(0xFF));
    UTEST_ASSERT_FALSE(ascii_is_uppercase(0xFF));
    
    UTEST_ASSERT_FALSE(ascii_is_whitespace(0xFFFF));
    
    UTEST_ASSERT_FALSE(ascii_is_control(0xFFFFFFFF));
}

/**
 * 测试字符分类组合。
 * 
 * @return 无返回值。
 */
UTEST_CASE(ascii_character_categories)
{
    char32 lower='g';
    UTEST_ASSERT_TRUE(ascii_is_alphabetic(lower));
    UTEST_ASSERT_TRUE(ascii_is_lowercase(lower));
    UTEST_ASSERT_FALSE(ascii_is_uppercase(lower));
    UTEST_ASSERT_FALSE(ascii_is_digit(lower));
    UTEST_ASSERT_FALSE(ascii_is_whitespace(lower));
    UTEST_ASSERT_FALSE(ascii_is_blank(lower));
    UTEST_ASSERT_FALSE(ascii_is_control(lower));
    
    char32 upper='G';
    UTEST_ASSERT_TRUE(ascii_is_alphabetic(upper));
    UTEST_ASSERT_FALSE(ascii_is_lowercase(upper));
    UTEST_ASSERT_TRUE(ascii_is_uppercase(upper));
    UTEST_ASSERT_FALSE(ascii_is_digit(upper));
    UTEST_ASSERT_FALSE(ascii_is_whitespace(upper));
    UTEST_ASSERT_FALSE(ascii_is_blank(upper));
    UTEST_ASSERT_FALSE(ascii_is_control(upper));
    
    char32 digit='9';
    UTEST_ASSERT_FALSE(ascii_is_alphabetic(digit));
    UTEST_ASSERT_FALSE(ascii_is_lowercase(digit));
    UTEST_ASSERT_FALSE(ascii_is_uppercase(digit));
    UTEST_ASSERT_TRUE(ascii_is_digit(digit));
    UTEST_ASSERT_FALSE(ascii_is_whitespace(digit));
    UTEST_ASSERT_FALSE(ascii_is_blank(digit));
    UTEST_ASSERT_FALSE(ascii_is_control(digit));
    
    char32 space=' ';
    UTEST_ASSERT_FALSE(ascii_is_alphabetic(space));
    UTEST_ASSERT_FALSE(ascii_is_lowercase(space));
    UTEST_ASSERT_FALSE(ascii_is_uppercase(space));
    UTEST_ASSERT_FALSE(ascii_is_digit(space));
    UTEST_ASSERT_TRUE(ascii_is_whitespace(space));
    UTEST_ASSERT_TRUE(ascii_is_blank(space));
    UTEST_ASSERT_FALSE(ascii_is_control(space));
    
    char32 tab='\t';
    UTEST_ASSERT_FALSE(ascii_is_alphabetic(tab));
    UTEST_ASSERT_FALSE(ascii_is_lowercase(tab));
    UTEST_ASSERT_FALSE(ascii_is_uppercase(tab));
    UTEST_ASSERT_FALSE(ascii_is_digit(tab));
    UTEST_ASSERT_TRUE(ascii_is_whitespace(tab));
    UTEST_ASSERT_TRUE(ascii_is_blank(tab));
    UTEST_ASSERT_TRUE(ascii_is_control(tab));
    
    char32 newline='\n';
    UTEST_ASSERT_FALSE(ascii_is_alphabetic(newline));
    UTEST_ASSERT_FALSE(ascii_is_lowercase(newline));
    UTEST_ASSERT_FALSE(ascii_is_uppercase(newline));
    UTEST_ASSERT_FALSE(ascii_is_digit(newline));
    UTEST_ASSERT_TRUE(ascii_is_whitespace(newline));
    UTEST_ASSERT_FALSE(ascii_is_blank(newline));
    UTEST_ASSERT_TRUE(ascii_is_control(newline));
}

/**
 * 测试所有ASCII字符的全面分类。
 * 
 * @return 无返回值。
 */
UTEST_CASE(ascii_comprehensive_test)
{
    int32 alphabetic_count=0;
    int32 digit_count=0;
    int32 lowercase_count=0;
    int32 uppercase_count=0;
    int32 whitespace_count=0;
    int32 blank_count=0;
    int32 control_count=0;
    
    for(uintn i=0;i<128;i++)
    {
        char32 c=(char32)i;
        
        if(ascii_is_alphabetic(c))
        {
            alphabetic_count++;
            if(c>='a'&&c<='z')
            {
                UTEST_ASSERT_TRUE(ascii_is_lowercase(c));
                UTEST_ASSERT_FALSE(ascii_is_uppercase(c));
            }
            else
            {
                UTEST_ASSERT_TRUE(ascii_is_uppercase(c));
                UTEST_ASSERT_FALSE(ascii_is_lowercase(c));
            }
        }
        
        if(ascii_is_digit(c))
        {
            digit_count++;
            UTEST_ASSERT_TRUE(c>='0'&&c<='9');
        }
        
        if(ascii_is_lowercase(c))
        {
            lowercase_count++;
            UTEST_ASSERT_TRUE(c>='a'&&c<='z');
        }
        
        if(ascii_is_uppercase(c))
        {
            uppercase_count++;
            UTEST_ASSERT_TRUE(c>='A'&&c<='Z');
        }
        
        if(ascii_is_whitespace(c))
        {
            whitespace_count++;
            UTEST_ASSERT_TRUE(c==' '||c=='\t'||c=='\n'||c=='\r'||c=='\f'||c=='\v');
        }
        
        if(ascii_is_blank(c))
        {
            blank_count++;
            UTEST_ASSERT_TRUE(c==' '||c=='\t');
        }
        
        if(ascii_is_control(c))
        {
            control_count++;
            UTEST_ASSERT_TRUE(i<=0x1F||i==0x7F);
        }
    }
    
    UTEST_ASSERT_EQUAL(alphabetic_count,52);
    UTEST_ASSERT_EQUAL(digit_count,10);
    UTEST_ASSERT_EQUAL(lowercase_count,26);
    UTEST_ASSERT_EQUAL(uppercase_count,26);
    UTEST_ASSERT_EQUAL(whitespace_count,6);
    UTEST_ASSERT_EQUAL(blank_count,2);
    UTEST_ASSERT_EQUAL(control_count,33);
}

/**
 * 测试字符大小写转换函数。
 * 
 * @return 无返回值。
 */
UTEST_CASE(ascii_case_conversion_basic)
{
    for(char32 c='A';c<='Z';c++)
    {
        char32 lower=to_lowercase(c);
        UTEST_ASSERT_EQUAL(lower,c+'a'-'A');
        UTEST_ASSERT_TRUE(ascii_is_lowercase(lower));
    }
    
    for(char32 c='a';c<='z';c++)
    {
        char32 upper=to_uppercase(c);
        UTEST_ASSERT_EQUAL(upper,c-'a'+'A');
        UTEST_ASSERT_TRUE(ascii_is_uppercase(upper));
    }
    
    UTEST_ASSERT_EQUAL(to_lowercase('0'),'0');
    UTEST_ASSERT_EQUAL(to_lowercase('@'),'@');
    UTEST_ASSERT_EQUAL(to_lowercase('['),'[');
    UTEST_ASSERT_EQUAL(to_uppercase('0'),'0');
    UTEST_ASSERT_EQUAL(to_uppercase('@'),'@');
    UTEST_ASSERT_EQUAL(to_uppercase('['),'[');
    
    UTEST_ASSERT_EQUAL(to_lowercase('A'),'a');
    UTEST_ASSERT_EQUAL(to_lowercase('Z'),'z');
    UTEST_ASSERT_EQUAL(to_uppercase('a'),'A');
    UTEST_ASSERT_EQUAL(to_uppercase('z'),'Z');
    UTEST_ASSERT_EQUAL(to_lowercase('5'),'5');
    UTEST_ASSERT_EQUAL(to_uppercase('#'),'#');
    
    UTEST_ASSERT_EQUAL(to_lowercase('B'),'b');
    UTEST_ASSERT_EQUAL(to_lowercase('Y'),'y');
    UTEST_ASSERT_EQUAL(to_uppercase('c'),'C');
    UTEST_ASSERT_EQUAL(to_uppercase('x'),'X');
    UTEST_ASSERT_EQUAL(to_lowercase('!'),'!');
    UTEST_ASSERT_EQUAL(to_uppercase('?'),'?');
}

/**
 * 测试边界情况和特殊字符转换。
 * 
 * @return 无返回值。
 */
UTEST_CASE(ascii_case_conversion_edge_cases)
{
    UTEST_ASSERT_EQUAL(to_lowercase('\0'),'\0');
    UTEST_ASSERT_EQUAL(to_uppercase('\n'),'\n');
    UTEST_ASSERT_EQUAL(to_lowercase(0x7F),0x7F);
    UTEST_ASSERT_EQUAL(to_uppercase(0x7F),0x7F);
    
    UTEST_ASSERT_EQUAL(to_lowercase(128),128);
    UTEST_ASSERT_EQUAL(to_uppercase(255),255);
    
    UTEST_ASSERT_EQUAL(to_lowercase(' '),' ');
    UTEST_ASSERT_EQUAL(to_uppercase('\t'),'\t');
    UTEST_ASSERT_EQUAL(to_lowercase('\n'),'\n');
    UTEST_ASSERT_EQUAL(to_uppercase('\r'),'\r');
    
    for(char32 c='0';c<='9';c++)
    {
        UTEST_ASSERT_EQUAL(to_lowercase(c),c);
        UTEST_ASSERT_EQUAL(to_uppercase(c),c);
    }
    
    UTEST_ASSERT_EQUAL(to_lowercase('.'),'.');
    UTEST_ASSERT_EQUAL(to_uppercase(','),',');
    UTEST_ASSERT_EQUAL(to_lowercase(';'),';');
    UTEST_ASSERT_EQUAL(to_uppercase(':'),':');
}

/**
 * 测试大小写转换的幂等性。
 * 
 * @return 无返回值。
 */
UTEST_CASE(ascii_case_conversion_idempotent)
{
    for(char32 c='a';c<='z';c++)
    {
        UTEST_ASSERT_EQUAL(to_lowercase(c),c);
        UTEST_ASSERT_EQUAL(to_lowercase(to_lowercase(c)),c);
    }
    
    for(char32 c='A';c<='Z';c++)
    {
        UTEST_ASSERT_EQUAL(to_uppercase(c),c);
        UTEST_ASSERT_EQUAL(to_uppercase(to_uppercase(c)),c);
    }
    
    for(char32 c='A';c<='Z';c++)
    {
        UTEST_ASSERT_EQUAL(to_uppercase(to_lowercase(c)),c);
    }
    
    for(char32 c='a';c<='z';c++)
    {
        UTEST_ASSERT_EQUAL(to_lowercase(to_uppercase(c)),c);
    }
    
    UTEST_ASSERT_EQUAL(to_lowercase(to_lowercase('a')),'a');
    UTEST_ASSERT_EQUAL(to_uppercase(to_uppercase('A')),'A');
    UTEST_ASSERT_EQUAL(to_lowercase(to_lowercase('z')),'z');
    UTEST_ASSERT_EQUAL(to_uppercase(to_uppercase('Z')),'Z');
}

/**
 * 测试大小写转换与分类函数的一致性。
 * 
 * @return 无返回值。
 */
UTEST_CASE(ascii_case_conversion_consistency)
{
    for(uintn i=0;i<256;i++)
    {
        char32 c=(char32)i;
        char32 lower=to_lowercase(c);
        char32 upper=to_uppercase(c);
        
        if(ascii_is_uppercase(c))
        {
            UTEST_ASSERT_TRUE(ascii_is_lowercase(lower));
            UTEST_ASSERT_TRUE(ascii_is_uppercase(upper));
            UTEST_ASSERT_EQUAL(lower,c+'a'-'A');
        }
        else if(ascii_is_lowercase(c))
        {
            UTEST_ASSERT_TRUE(ascii_is_uppercase(upper));
            UTEST_ASSERT_TRUE(ascii_is_lowercase(lower));
            UTEST_ASSERT_EQUAL(upper,c-'a'+'A');
        }
        else
        {
            UTEST_ASSERT_EQUAL(lower,c);
            UTEST_ASSERT_EQUAL(upper,c);
        }
        
        UTEST_ASSERT_EQUAL(to_lowercase(to_uppercase(c)),to_lowercase(c));
        UTEST_ASSERT_EQUAL(to_uppercase(to_lowercase(c)),to_uppercase(c));
    }
}

/**
 * 测试完整的ASCII字符表大小写转换。
 * 
 * @return 无返回值。
 */
UTEST_CASE(ascii_case_conversion_comprehensive)
{
    int32 uppercase_converted=0;
    int32 lowercase_converted=0;
    int32 unchanged=0;
    
    for(uintn i=0;i<128;i++)
    {
        char32 c=(char32)i;
        char32 lower=to_lowercase(c);
        char32 upper=to_uppercase(c);
        
        if(ascii_is_uppercase(c))
        {
            uppercase_converted++;
            UTEST_ASSERT_EQUAL(lower,c+'a'-'A');
            UTEST_ASSERT_EQUAL(upper,c);
            UTEST_ASSERT_TRUE(ascii_is_lowercase(lower));
        }
        else if(ascii_is_lowercase(c))
        {
            lowercase_converted++;
            UTEST_ASSERT_EQUAL(upper,c-'a'+'A');
            UTEST_ASSERT_EQUAL(lower,c);
            UTEST_ASSERT_TRUE(ascii_is_uppercase(upper));
        }
        else
        {
            unchanged++;
            UTEST_ASSERT_EQUAL(lower,c);
            UTEST_ASSERT_EQUAL(upper,c);
        }
    }
    
    UTEST_ASSERT_EQUAL(uppercase_converted,26);
    UTEST_ASSERT_EQUAL(lowercase_converted,26);
    UTEST_ASSERT_EQUAL(unchanged,76);
    
    UTEST_ASSERT_EQUAL(to_lowercase('A'),'a');
    UTEST_ASSERT_EQUAL(to_lowercase('Z'),'z');
    UTEST_ASSERT_EQUAL(to_uppercase('a'),'A');
    UTEST_ASSERT_EQUAL(to_uppercase('z'),'Z');
    
    for(char32 upper='A',lower='a';upper<='Z';upper++,lower++)
    {
        UTEST_ASSERT_EQUAL(to_lowercase(upper),lower);
        UTEST_ASSERT_EQUAL(to_uppercase(lower),upper);
    }
}

/**
 * 字符库函数测试。
 * 
 * @return 失败测试数。
 */
int32 char_test()
{
    UTEST_SUITE("aos.kernel.test.library.char");
    
    UTEST_RUN(ascii_is_alphabetic_basic);
    UTEST_RUN(ascii_is_digit_basic);
    UTEST_RUN(ascii_is_lowercase_basic);
    UTEST_RUN(ascii_is_uppercase_basic);
    UTEST_RUN(ascii_is_whitespace_basic);
    UTEST_RUN(ascii_is_blank_basic);
    UTEST_RUN(ascii_is_control_basic);
    UTEST_RUN(ascii_edge_cases);
    UTEST_RUN(ascii_character_categories);
    UTEST_RUN(ascii_comprehensive_test);

    UTEST_RUN(ascii_case_conversion_basic);
    UTEST_RUN(ascii_case_conversion_edge_cases);
    UTEST_RUN(ascii_case_conversion_idempotent);
    UTEST_RUN(ascii_case_conversion_consistency);
    UTEST_RUN(ascii_case_conversion_comprehensive);
    
    UTEST_SUMMARY("aos.kernel.test.library.char");
}