/**
 * 内核内存库函数测试。
 * @date 2026-01-04
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include <lib/memory.h>
#include <test/utest.h>

#include <stdlib.h>

/**
 * 生成测试数据。
 * 
 * @param buffer 缓冲区。
 * @param size   生成字节数。
 * 
 * @return 无返回值。
 */
static void memory_generate_test_data(void* buffer,uintn size)
{
    uint8* p=(uint8*)buffer;
    for(uintn index=0;index<size;index++)
    {
        p[index]=(uint8)(index%256);
    }
}

/**
 * 测试基础内存复制。
 * 
 * @return 无返回值。
 */
UTEST_CASE(memory_copy_basic)
{
    uint8 src[10]={0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A};
    uint8 dest[10]={0};
    
    void* result=memory_copy(dest, src, 10);
    
    UTEST_ASSERT_EQUAL(result,dest);
    UTEST_ASSERT_FALSE(memcmp(dest,src,10));
    UTEST_ASSERT_NOT_EQUAL(dest[0],0);
}

/**
 * 测试0字节内存复制。
 * 
 * @return 无返回值。
 */
UTEST_CASE(memory_copy_zero)
{
    uint8 src[5]={1,2,3,4,5};
    uint8 dest[5]={6,7,8,9,10};
    
    void* result=memory_copy(dest,src,0);
    
    UTEST_ASSERT_EQUAL(result,dest);
    UTEST_ASSERT_EQUAL(dest[0],6);
    UTEST_ASSERT_EQUAL(dest[4],10);
}

/**
 * 测试部分内存复制。
 * 
 * @return 无返回值。
 */
UTEST_CASE(memory_copy_partial)
{
    uint8 src[8]={1,2,3,4,5,6,7,8};
    uint8 dest[8]={0};
    
    void* result=memory_copy(dest,src,5);
    
    UTEST_ASSERT_EQUAL(result,dest);
    UTEST_ASSERT_EQUAL(dest[0],1);
    UTEST_ASSERT_EQUAL(dest[4],5);
    UTEST_ASSERT_EQUAL(dest[5],0);
}

/**
 * 测试不重叠内存移动。
 * 
 * @return 无返回值。
 */
UTEST_CASE(memory_move_non_overlap)
{
    uint8 src[8]={1,2,3,4,5,6,7,8};
    uint8 dest[8];
    
    void* result=memory_move(dest,src,8);
    
    UTEST_ASSERT_EQUAL(result,dest);
    UTEST_ASSERT_FALSE(memcmp(dest,src,8));
}

/**
 * 测试目标在源后重叠内存移动。
 * 
 * @return 无返回值。
 */
UTEST_CASE(memory_move_dest_after_src)
{
    uint8 buffer[10]={1,2,3,4,5,6,7,8,9,10};
    
    void* result=memory_move(buffer+4,buffer+2,6);
    
    UTEST_ASSERT_EQUAL(result,buffer+4);
    
    uint8 expected[10]={1,2,3,4,3,4,5,6,7,8};
    UTEST_ASSERT_FALSE(memcmp(buffer,expected,10));
}

/**
 * 测试目标在源前重叠内存移动。
 * 
 * @return 无返回值。
 */
UTEST_CASE(memory_move_src_after_dest)
{
    uint8 buffer[10]={1,2,3,4,5,6,7,8,9,10};
    
    void* result=memory_move(buffer+2,buffer+4,6);

    UTEST_ASSERT_EQUAL(result,buffer+2);

    uint8 expected[10]={1,2,5,6,7,8,9,10,9,10};
    UTEST_ASSERT_FALSE(memcmp(buffer,expected,10));
}

/**
 * 测试同指针内存移动。
 * 
 * @return 无返回值。
 */
UTEST_CASE(memory_move_same)
{
    uint8 buffer[5]={1,2,3,4,5};
    
    void* result=memory_move(buffer,buffer,5);
    
    UTEST_ASSERT_EQUAL(result,buffer);
    UTEST_ASSERT_EQUAL(buffer[0],1);
}

/**
 * 测试0字节内存移动。
 * 
 * @return 无返回值。
 */
UTEST_CASE(memory_move_zero)
{
    uint8 buffer[5]={1,2,3,4,5};
    uint8 orig[5]={1,2,3,4,5};
    
    void* result=memory_move(buffer,buffer,0);
    
    UTEST_ASSERT_EQUAL(result,buffer);
    UTEST_ASSERT_FALSE(memcmp(buffer,orig,5));
}

/**
 * 测试基础内存设置。
 * 
 * @return 无返回值。
 */
UTEST_CASE(memory_set_basic)
{
    uint8 buffer[10];
    memory_generate_test_data(buffer,10);
    
    void* result=memory_set(buffer,0xFF,10);
    
    UTEST_ASSERT_EQUAL(result,buffer);
    for(uintn index=0;index<10;index++)
    {
        UTEST_ASSERT_EQUAL(buffer[index],0xFF);
    }
}

/**
 * 测试0字节内存设置。
 * 
 * @return 无返回值。
 */
UTEST_CASE(memory_set_zero)
{
    uint8 buffer[5]={1,2,3,4,5};
    
    void* result=memory_set(buffer,0xAA,0);
    
    UTEST_ASSERT_EQUAL(result,buffer);
    UTEST_ASSERT_EQUAL(buffer[0],1);
}

/**
 * 测试部分内存设置。
 * 
 * @return 无返回值。
 */
UTEST_CASE(memory_set_partial)
{
    uint8 buffer[8]={1,2,3,4,5,6,7,8};
    
    void* result=memory_set(buffer,0xCC,4);
    
    UTEST_ASSERT_EQUAL(result,buffer);
    UTEST_ASSERT_EQUAL(buffer[0],0xCC);
    UTEST_ASSERT_EQUAL(buffer[3],0xCC);
    UTEST_ASSERT_EQUAL(buffer[4],5);
}

/**
 * 测试相等内存比较。
 * 
 * @return 无返回值。
 */
UTEST_CASE(memory_compare_equal)
{
    uint8 a[5]={1,2,3,4,5};
    uint8 b[5]={1,2,3,4,5};
    
    bool result=memory_compare(a,b,5);
    UTEST_ASSERT_TRUE(result);
}

/**
 * 测试不相等内存比较。
 * 
 * @return 无返回值。
 */
UTEST_CASE(memory_compare_not_equal)
{
    uint8 a[5]={1,2,3,4,5};
    uint8 b[5]={1,2,3,4,6};
    
    bool result=memory_compare(a,b,5);
    UTEST_ASSERT_FALSE(result);
}

/**
 * 测试部分内存比较。
 * 
 * @return 无返回值。
 */
UTEST_CASE(memory_compare_partial) {
    uint8 a[5]={1,2,3,4,5};
    uint8 b[5]={1,2,3,9,9};
    
    bool result=memory_compare(a,b,3);
    UTEST_ASSERT_TRUE(result);
    
    result=memory_compare(a,b,4);
    UTEST_ASSERT_FALSE(result);
}

/**
 * 测试0字节内存比较。
 * 
 * @return 无返回值。
 */
UTEST_CASE(memory_compare_zero)
{
    uint8 a[3]={1,2,3};
    uint8 b[3]={4,5,6};
    
    bool result=memory_compare(a,b,0);
    UTEST_ASSERT_TRUE(result);
}

/**
 * 测试同指针内存比较。
 * 
 * @return 无返回值。
 */
UTEST_CASE(memory_compare_same)
{
    uint8 buffer[4]={1,2,3,4};
    
    bool result=memory_compare(buffer,buffer,4);
    UTEST_ASSERT_TRUE(result);
}

/**
 * 测试存在字节内存查找。
 * 
 * @return 无返回值。
 */
UTEST_CASE(memory_find_present)
{
    uint8 buffer[10]={0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xAA};
    
    void* result=memory_find(buffer,0x55,10);
    UTEST_ASSERT_NOT_NULL(result);
    UTEST_ASSERT_EQUAL(result,buffer+4);
    
    result=memory_find(buffer,0x11,10);
    UTEST_ASSERT_EQUAL(result,buffer);
}

/**
 * 测试不存在字节内存查找。
 * 
 * @return 无返回值。
 */
UTEST_CASE(memory_find_not_present)
{
    uint8 buffer[10]={0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xAA};
    
    void* result=memory_find(buffer,0xFF,10);
    UTEST_ASSERT_NULL(result);
}

/**
 * 测试部分内存查找。
 * 
 * @return 无返回值。
 */
UTEST_CASE(memory_find_partial)
{
    uint8 buffer[10]={0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xAA};
    
    void* result=memory_find(buffer,0x55,5);
    UTEST_ASSERT_NOT_NULL(result);
    UTEST_ASSERT_EQUAL(result,buffer+4);
    
    result=memory_find(buffer,0x66,5);
    UTEST_ASSERT_NULL(result);
}

/**
 * 测试0字节内存查找。
 * 
 * @return 无返回值。
 */
UTEST_CASE(memory_find_zero)
{
    uint8 buffer[5]={1,2,3,4,5};
    
    void* result=memory_find(buffer,1,0);
    UTEST_ASSERT_NULL(result);
}

/**
 * 测试结尾内存查找。
 * 
 * @return 无返回值。
 */
UTEST_CASE(memory_find_end)
{
    uint8 buffer[5]={1,2,3,4,5};
    
    void* result=memory_find(buffer,5,5);
    UTEST_ASSERT_EQUAL(result,buffer+4);
}

/**
 * 测试基础内存置零。
 * 
 * @return 无返回值。
 */
UTEST_CASE(memory_zero_basic)
{
    uint8 buffer[10];
    memory_generate_test_data(buffer,10);
    
    void* result=memory_zero(buffer,10);
    
    UTEST_ASSERT_EQUAL(result,buffer);
    for(uintn index=0;index<10;index++)
    {
        UTEST_ASSERT_EQUAL(buffer[index],0);
    }
}

/**
 * 测试0字节内存置零。
 * 
 * @return 无返回值。
 */
UTEST_CASE(memory_zero_zero)
{
    uint8 buffer[5]={1,2,3,4,5};
    
    void* result=memory_zero(buffer,0);
    
    UTEST_ASSERT_EQUAL(result,buffer);
    UTEST_ASSERT_EQUAL(buffer[0],1);
}

/**
 * 测试部分内存置零。
 * 
 * @return 无返回值。
 */
UTEST_CASE(memory_zero_partial)
{
    uint8 buffer[8]={1,2,3,4,5,6,7,8};
    
    void* result=memory_zero(buffer,4);
    
    UTEST_ASSERT_EQUAL(result,buffer);
    UTEST_ASSERT_EQUAL(buffer[0],0);
    UTEST_ASSERT_EQUAL(buffer[3],0);
    UTEST_ASSERT_EQUAL(buffer[4],5);
}

/**
 * 测试边界和大内存操作。
 * 
 * @return 无返回值。
 */
UTEST_CASE(memory_large_operations)
{
    const uintn size=4096;
    uint8* src=(uint8*)malloc(size);
    uint8* dest=(uint8*)malloc(size);
    
    memory_generate_test_data(src,size);
    
    void* result=memory_copy(dest,src,size);
    UTEST_ASSERT_EQUAL(result,dest);
    UTEST_ASSERT_FALSE(memcmp(dest,src,size));
    
    result=memory_set(dest,0xAA,size);
    UTEST_ASSERT_EQUAL(result,dest);
    for(uintn index=0;index<size;index+=50)
    {
        UTEST_ASSERT_EQUAL(dest[index],0xAA);
    }
    
    free(src);
    free(dest);
}

/**
 * 测试功能集成。
 * 
 * @return 无返回值。
 */
UTEST_CASE(memory_integration)
{
    uint8 buffer1[20];
    uint8 buffer2[20];
    
    memory_set(buffer1,0xAB,20);
    
    memory_copy(buffer2,buffer1,20);
    
    UTEST_ASSERT_TRUE(memory_compare(buffer1,buffer2,20));
    
    void* found = memory_find(buffer2,0xAB,20);
    UTEST_ASSERT_NOT_NULL(found);
    
    memory_set(buffer1 + 5,0xCD,5);
    memory_move(buffer2,buffer1,20);
    found=memory_find(buffer2+5,0xCD,5);
    UTEST_ASSERT_NOT_NULL(found);
}

/**
 * 内存库函数测试。
 * 
 * @return 失败测试数。
 */
int32 memory_test()
{
    UTEST_SUITE("aos.kernel.test.lib.memory");
    
    UTEST_RUN(memory_copy_basic);
    UTEST_RUN(memory_copy_zero);
    UTEST_RUN(memory_copy_partial);
    
    UTEST_RUN(memory_move_non_overlap);
    UTEST_RUN(memory_move_dest_after_src);
    UTEST_RUN(memory_move_src_after_dest);
    UTEST_RUN(memory_move_same);
    UTEST_RUN(memory_move_zero);
    
    UTEST_RUN(memory_set_basic);
    UTEST_RUN(memory_set_zero);
    UTEST_RUN(memory_set_partial);
    
    UTEST_RUN(memory_compare_equal);
    UTEST_RUN(memory_compare_not_equal);
    UTEST_RUN(memory_compare_partial);
    UTEST_RUN(memory_compare_zero);
    UTEST_RUN(memory_compare_same);
    
    UTEST_RUN(memory_find_present);
    UTEST_RUN(memory_find_not_present);
    UTEST_RUN(memory_find_partial);
    UTEST_RUN(memory_find_zero);
    UTEST_RUN(memory_find_end);
    
    UTEST_RUN(memory_zero_basic);
    UTEST_RUN(memory_zero_zero);
    UTEST_RUN(memory_zero_partial);

    UTEST_RUN(memory_large_operations);
    UTEST_RUN(memory_integration);
    
    UTEST_SUMMARY("aos.kernel.test.lib.memory");
}