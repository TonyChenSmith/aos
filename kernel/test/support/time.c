/**
 * 内核时间库函数测试。
 * @date 2026-05-31
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include <test/utest.h>

#include <support/const.h>
#include <support/memory.h>
#include <support/time.h>

/**
 * 测试时间戳规范化——纳秒进位。
 * 
 * @return 无返回值。
 */
UTEST_CASE(time_timestamp_normalize_basic)
{
    aos_timestamp ts={.second=100,.nanosecond=500,.timezone=480};
    timestamp_normalize(&ts);
    UTEST_ASSERT_EQUAL(ts.second,100);
    UTEST_ASSERT_EQUAL(ts.nanosecond,500);

    ts.second=100;
    ts.nanosecond=2500000000ULL;
    timestamp_normalize(&ts);
    UTEST_ASSERT_EQUAL(ts.second,102);
    UTEST_ASSERT_EQUAL(ts.nanosecond,500000000);

    ts.second=0;
    ts.nanosecond=1000000000;
    timestamp_normalize(&ts);
    UTEST_ASSERT_EQUAL(ts.second,1);
    UTEST_ASSERT_EQUAL(ts.nanosecond,0);

    ts.second=0;
    ts.nanosecond=3000000000ULL;
    timestamp_normalize(&ts);
    UTEST_ASSERT_EQUAL(ts.second,3);
    UTEST_ASSERT_EQUAL(ts.nanosecond,0);
}

/**
 * 测试时间戳规范化——时区折叠。
 * 
 * @return 无返回值。
 */
UTEST_CASE(time_timestamp_normalize_timezone)
{
    aos_timestamp ts={.timezone=2000};
    timestamp_normalize(&ts);
    UTEST_ASSERT_EQUAL(ts.timezone,-880);

    ts.timezone=-2000;
    timestamp_normalize(&ts);
    UTEST_ASSERT_EQUAL(ts.timezone,880);

    ts.timezone=1440;
    timestamp_normalize(&ts);
    UTEST_ASSERT_EQUAL(ts.timezone,1440);

    ts.timezone=-1440;
    timestamp_normalize(&ts);
    UTEST_ASSERT_EQUAL(ts.timezone,-1440);

    ts.timezone=1441;
    timestamp_normalize(&ts);
    UTEST_ASSERT_EQUAL(ts.timezone,-1439);

    ts.timezone=-1441;
    timestamp_normalize(&ts);
    UTEST_ASSERT_EQUAL(ts.timezone,1439);
}

/**
 * 测试时间戳规范化——空指针。
 * 
 * @return 无返回值。
 */
UTEST_CASE(time_timestamp_normalize_null)
{
    timestamp_normalize(null);
    UTEST_ASSERT_TRUE(true);
}

/**
 * 测试时间规范化——时分秒纳秒进位。
 * 
 * @return 无返回值。
 */
UTEST_CASE(time_normalize_basic)
{
    aos_time t;
    memory_set(&t,0,sizeof(aos_time));
    t.year=2026;

    t.second=125;
    time_normalize(&t);
    UTEST_ASSERT_EQUAL(t.minute,2);
    UTEST_ASSERT_EQUAL(t.second,5);

    memory_set(&t,0,sizeof(aos_time));
    t.year=2026;
    t.hour=25;
    time_normalize(&t);
    UTEST_ASSERT_EQUAL(t.day,1);
    UTEST_ASSERT_EQUAL(t.hour,1);

    memory_set(&t,0,sizeof(aos_time));
    t.year=2026;
    t.nanosecond=2000000000;
    time_normalize(&t);
    UTEST_ASSERT_EQUAL(t.second,2);
    UTEST_ASSERT_EQUAL(t.nanosecond,0);

    memory_set(&t,0,sizeof(aos_time));
    t.year=2026;
    t.minute=130;
    time_normalize(&t);
    UTEST_ASSERT_EQUAL(t.hour,2);
    UTEST_ASSERT_EQUAL(t.minute,10);
}

/**
 * 测试时间规范化——日期进位。
 * 
 * @return 无返回值。
 */
UTEST_CASE(time_normalize_date)
{
    aos_time t;
    memory_set(&t,0,sizeof(aos_time));

    t.year=2026;
    t.month=0;
    t.day=32;
    time_normalize(&t);
    UTEST_ASSERT_EQUAL(t.year,2026);
    UTEST_ASSERT_EQUAL(t.month,1);
    UTEST_ASSERT_EQUAL(t.day,1);

    memory_set(&t,0,sizeof(aos_time));
    t.year=2026;
    t.month=11;
    t.day=31;
    time_normalize(&t);
    UTEST_ASSERT_EQUAL(t.year,2027);
    UTEST_ASSERT_EQUAL(t.month,0);
    UTEST_ASSERT_EQUAL(t.day,0);

    memory_set(&t,0,sizeof(aos_time));
    t.year=2026;
    t.month=13;
    time_normalize(&t);
    UTEST_ASSERT_EQUAL(t.year,2027);
    UTEST_ASSERT_EQUAL(t.month,1);

    memory_set(&t,0,sizeof(aos_time));
    t.year=2024;
    t.month=1;
    t.day=29;
    time_normalize(&t);
    UTEST_ASSERT_EQUAL(t.year,2024);
    UTEST_ASSERT_EQUAL(t.month,2);
    UTEST_ASSERT_EQUAL(t.day,0);

    memory_set(&t,0,sizeof(aos_time));
    t.year=2025;
    t.month=1;
    t.day=28;
    time_normalize(&t);
    UTEST_ASSERT_EQUAL(t.year,2025);
    UTEST_ASSERT_EQUAL(t.month,2);
    UTEST_ASSERT_EQUAL(t.day,0);
}

/**
 * 测试时间规范化——时区折叠。
 * 
 * @return 无返回值。
 */
UTEST_CASE(time_normalize_timezone)
{
    aos_time t;
    memory_set(&t,0,sizeof(aos_time));
    t.year=2026;

    t.timezone=2000;
    time_normalize(&t);
    UTEST_ASSERT_EQUAL(t.timezone,-880);

    t.timezone=-2000;
    time_normalize(&t);
    UTEST_ASSERT_EQUAL(t.timezone,880);

    t.timezone=1440;
    time_normalize(&t);
    UTEST_ASSERT_EQUAL(t.timezone,1440);

    t.timezone=-1440;
    time_normalize(&t);
    UTEST_ASSERT_EQUAL(t.timezone,-1440);
}

/**
 * 测试时间规范化——空指针。
 * 
 * @return 无返回值。
 */
UTEST_CASE(time_normalize_null)
{
    time_normalize(null);
    UTEST_ASSERT_TRUE(true);
}

/**
 * 测试时间戳比较。
 * 
 * @return 无返回值。
 */
UTEST_CASE(time_timestamp_compare_basic)
{
    aos_timestamp a={.second=100};
    aos_timestamp b={.second=200};
    UTEST_ASSERT_TRUE(timestamp_compare(&a,&b)<0);
    UTEST_ASSERT_TRUE(timestamp_compare(&b,&a)>0);

    a.second=100;
    a.nanosecond=500;
    b.second=100;
    b.nanosecond=200;
    UTEST_ASSERT_TRUE(timestamp_compare(&a,&b)>0);

    a.second=100;
    a.nanosecond=100;
    b.second=100;
    b.nanosecond=100;
    UTEST_ASSERT_EQUAL(timestamp_compare(&a,&b),0);

    a.second=0;
    a.nanosecond=0;
    b.second=0;
    b.nanosecond=0;
    UTEST_ASSERT_EQUAL(timestamp_compare(&a,&b),0);

    a.second=-100;
    b.second=-50;
    UTEST_ASSERT_TRUE(timestamp_compare(&a,&b)<0);

    a.second=-1;
    b.second=1;
    UTEST_ASSERT_TRUE(timestamp_compare(&a,&b)<0);
}

/**
 * 测试时间戳比较——空指针。
 * 
 * @return 无返回值。
 */
UTEST_CASE(time_timestamp_compare_null)
{
    aos_timestamp a={.second=1};
    UTEST_ASSERT_EQUAL(timestamp_compare(null,null),0);
    UTEST_ASSERT_TRUE(timestamp_compare(&a,null)>0);
    UTEST_ASSERT_TRUE(timestamp_compare(null,&a)<0);
}

/**
 * 测试时间转时间戳再转回——公元后。
 * 
 * @return 无返回值。
 */
UTEST_CASE(time_roundtrip_ad)
{
    aos_time t,back;
    aos_timestamp ts;

    memory_set(&t,0,sizeof(aos_time));
    t.year=1970;
    UTEST_ASSERT_TRUE(time_to_timestamp(&ts,&t));
    UTEST_ASSERT_EQUAL(ts.second,0);
    UTEST_ASSERT_TRUE(timestamp_to_time(&back,&ts));
    UTEST_ASSERT_EQUAL(back.year,1970);
    UTEST_ASSERT_EQUAL(back.month,0);
    UTEST_ASSERT_EQUAL(back.day,0);

    memory_set(&t,0,sizeof(aos_time));
    t.year=2025;
    UTEST_ASSERT_TRUE(time_to_timestamp(&ts,&t));
    UTEST_ASSERT_TRUE(timestamp_to_time(&back,&ts));
    UTEST_ASSERT_EQUAL(back.year,2025);

    memory_set(&t,0,sizeof(aos_time));
    t.year=2024;
    t.month=1;
    t.day=28;
    UTEST_ASSERT_TRUE(time_to_timestamp(&ts,&t));
    UTEST_ASSERT_TRUE(timestamp_to_time(&back,&ts));
    UTEST_ASSERT_EQUAL(back.year,2024);
    UTEST_ASSERT_EQUAL(back.month,1);
    UTEST_ASSERT_EQUAL(back.day,28);

    memory_set(&t,0,sizeof(aos_time));
    t.year=1900;
    t.month=1;
    t.day=27;
    UTEST_ASSERT_TRUE(time_to_timestamp(&ts,&t));
    UTEST_ASSERT_TRUE(timestamp_to_time(&back,&ts));
    UTEST_ASSERT_EQUAL(back.year,1900);
    UTEST_ASSERT_EQUAL(back.month,1);
    UTEST_ASSERT_EQUAL(back.day,27);

    memory_set(&t,0,sizeof(aos_time));
    t.year=1900;
    t.month=2;
    t.day=0;
    UTEST_ASSERT_TRUE(time_to_timestamp(&ts,&t));
    UTEST_ASSERT_TRUE(timestamp_to_time(&back,&ts));
    UTEST_ASSERT_EQUAL(back.year,1900);
    UTEST_ASSERT_EQUAL(back.month,2);
    UTEST_ASSERT_EQUAL(back.day,0);

    memory_set(&t,0,sizeof(aos_time));
    t.year=2000;
    t.month=1;
    t.day=28;
    UTEST_ASSERT_TRUE(time_to_timestamp(&ts,&t));
    UTEST_ASSERT_TRUE(timestamp_to_time(&back,&ts));
    UTEST_ASSERT_EQUAL(back.year,2000);
    UTEST_ASSERT_EQUAL(back.month,1);
    UTEST_ASSERT_EQUAL(back.day,28);
}

/**
 * 测试时间转时间戳再转回——公元前。
 * 
 * @return 无返回值。
 */
UTEST_CASE(time_roundtrip_bc)
{
    aos_time t,back;
    aos_timestamp ts;

    memory_set(&t,0,sizeof(aos_time));
    t.year=0;
    UTEST_ASSERT_TRUE(time_to_timestamp(&ts,&t));
    UTEST_ASSERT_TRUE(timestamp_to_time(&back,&ts));
    UTEST_ASSERT_EQUAL(back.year,0);
    UTEST_ASSERT_EQUAL(back.month,0);
    UTEST_ASSERT_EQUAL(back.day,0);

    memory_set(&t,0,sizeof(aos_time));
    t.year=0;
    t.month=1;
    t.day=28;
    UTEST_ASSERT_TRUE(time_to_timestamp(&ts,&t));
    UTEST_ASSERT_TRUE(timestamp_to_time(&back,&ts));
    UTEST_ASSERT_EQUAL(back.year,0);
    UTEST_ASSERT_EQUAL(back.month,1);
    UTEST_ASSERT_EQUAL(back.day,28);

    memory_set(&t,0,sizeof(aos_time));
    t.year=-3;
    t.month=1;
    t.day=27;
    UTEST_ASSERT_TRUE(time_to_timestamp(&ts,&t));
    UTEST_ASSERT_TRUE(timestamp_to_time(&back,&ts));
    UTEST_ASSERT_EQUAL(back.year,-3);
    UTEST_ASSERT_EQUAL(back.month,1);
    UTEST_ASSERT_EQUAL(back.day,27);

    memory_set(&t,0,sizeof(aos_time));
    t.year=-3;
    t.month=2;
    t.day=0;
    UTEST_ASSERT_TRUE(time_to_timestamp(&ts,&t));
    UTEST_ASSERT_TRUE(timestamp_to_time(&back,&ts));
    UTEST_ASSERT_EQUAL(back.year,-3);
    UTEST_ASSERT_EQUAL(back.month,2);
    UTEST_ASSERT_EQUAL(back.day,0);

    memory_set(&t,0,sizeof(aos_time));
    t.year=-4;
    t.month=1;
    t.day=28;
    UTEST_ASSERT_TRUE(time_to_timestamp(&ts,&t));
    UTEST_ASSERT_TRUE(timestamp_to_time(&back,&ts));
    UTEST_ASSERT_EQUAL(back.year,-4);
    UTEST_ASSERT_EQUAL(back.month,1);
    UTEST_ASSERT_EQUAL(back.day,28);

    memory_set(&t,0,sizeof(aos_time));
    t.year=-100;
    t.month=1;
    t.day=27;
    UTEST_ASSERT_TRUE(time_to_timestamp(&ts,&t));
    UTEST_ASSERT_TRUE(timestamp_to_time(&back,&ts));
    UTEST_ASSERT_EQUAL(back.year,-100);
    UTEST_ASSERT_EQUAL(back.month,1);
    UTEST_ASSERT_EQUAL(back.day,27);

    memory_set(&t,0,sizeof(aos_time));
    t.year=-400;
    t.month=1;
    t.day=28;
    UTEST_ASSERT_TRUE(time_to_timestamp(&ts,&t));
    UTEST_ASSERT_TRUE(timestamp_to_time(&back,&ts));
    UTEST_ASSERT_EQUAL(back.year,-400);
    UTEST_ASSERT_EQUAL(back.month,1);
    UTEST_ASSERT_EQUAL(back.day,28);
}

/**
 * 测试时间转时间戳再转回——带时分秒和时区。
 * 
 * @return 无返回值。
 */
UTEST_CASE(time_roundtrip_hms)
{
    aos_time t,back;
    aos_timestamp ts;

    memory_set(&t,0,sizeof(aos_time));
    t.year=2026;
    t.hour=15;
    t.minute=30;
    t.second=45;
    t.nanosecond=123456789;
    t.timezone=480;
    UTEST_ASSERT_TRUE(time_to_timestamp(&ts,&t));
    UTEST_ASSERT_TRUE(timestamp_to_time(&back,&ts));
    UTEST_ASSERT_EQUAL(back.year,2026);
    UTEST_ASSERT_EQUAL(back.hour,15);
    UTEST_ASSERT_EQUAL(back.minute,30);
    UTEST_ASSERT_EQUAL(back.second,45);
    UTEST_ASSERT_EQUAL(back.nanosecond,123456789);
    UTEST_ASSERT_EQUAL(back.timezone,480);

    memory_set(&t,0,sizeof(aos_time));
    t.year=2026;
    t.hour=12;
    t.timezone=-300;
    UTEST_ASSERT_TRUE(time_to_timestamp(&ts,&t));
    UTEST_ASSERT_TRUE(timestamp_to_time(&back,&ts));
    UTEST_ASSERT_EQUAL(back.year,2026);
    UTEST_ASSERT_EQUAL(back.hour,12);
    UTEST_ASSERT_EQUAL(back.timezone,-300);

    memory_set(&t,0,sizeof(aos_time));
    t.year=2026;
    t.hour=2;
    t.timezone=480;
    UTEST_ASSERT_TRUE(time_to_timestamp(&ts,&t));
    UTEST_ASSERT_TRUE(timestamp_to_time(&back,&ts));
    UTEST_ASSERT_EQUAL(back.year,2026);
    UTEST_ASSERT_EQUAL(back.hour,2);
    UTEST_ASSERT_EQUAL(back.timezone,480);
}

/**
 * 测试时间戳转时间再转回——正时间戳。
 * 
 * @return 无返回值。
 */
UTEST_CASE(time_reverse_roundtrip_positive)
{
    aos_time t;
    aos_timestamp ts,back;

    ts.second=1735689600;
    ts.nanosecond=0;
    ts.timezone=0;
    UTEST_ASSERT_TRUE(timestamp_to_time(&t,&ts));
    UTEST_ASSERT_TRUE(time_to_timestamp(&back,&t));
    UTEST_ASSERT_EQUAL(back.second,1735689600);

    ts.second=0;
    ts.nanosecond=500000000;
    ts.timezone=0;
    UTEST_ASSERT_TRUE(timestamp_to_time(&t,&ts));
    UTEST_ASSERT_TRUE(time_to_timestamp(&back,&t));
    UTEST_ASSERT_EQUAL(back.second,0);
    UTEST_ASSERT_EQUAL(back.nanosecond,500000000);

    ts.second=1000;
    ts.nanosecond=123456789;
    ts.timezone=0;
    UTEST_ASSERT_TRUE(timestamp_to_time(&t,&ts));
    UTEST_ASSERT_TRUE(time_to_timestamp(&back,&t));
    UTEST_ASSERT_EQUAL(back.second,1000);
    UTEST_ASSERT_EQUAL(back.nanosecond,123456789);
}

/**
 * 测试时间戳转时间再转回——负时间戳。
 * 
 * @return 无返回值。
 */
UTEST_CASE(time_reverse_roundtrip_negative)
{
    aos_time t;
    aos_timestamp ts,back;

    ts.second=-86400;
    ts.nanosecond=0;
    ts.timezone=0;
    UTEST_ASSERT_TRUE(timestamp_to_time(&t,&ts));
    UTEST_ASSERT_EQUAL(t.year,1969);
    UTEST_ASSERT_TRUE(time_to_timestamp(&back,&t));
    UTEST_ASSERT_EQUAL(back.second,-86400);

    ts.second=-1;
    ts.nanosecond=0;
    ts.timezone=0;
    UTEST_ASSERT_TRUE(timestamp_to_time(&t,&ts));
    UTEST_ASSERT_EQUAL(t.hour,23);
    UTEST_ASSERT_EQUAL(t.minute,59);
    UTEST_ASSERT_EQUAL(t.second,59);
    UTEST_ASSERT_TRUE(time_to_timestamp(&back,&t));
    UTEST_ASSERT_EQUAL(back.second,-1);

    ts.second=-86400;
    ts.nanosecond=0;
    ts.timezone=0;
    UTEST_ASSERT_TRUE(timestamp_to_time(&t,&ts));
    UTEST_ASSERT_EQUAL(t.hour,0);
    UTEST_ASSERT_EQUAL(t.minute,0);
    UTEST_ASSERT_EQUAL(t.second,0);
    UTEST_ASSERT_TRUE(time_to_timestamp(&back,&t));
    UTEST_ASSERT_EQUAL(back.second,-86400);
}

/**
 * 测试转换函数空指针。
 * 
 * @return 无返回值。
 */
UTEST_CASE(time_convert_null)
{
    aos_time t;
    aos_timestamp ts={.second=0};
    memory_set(&t,0,sizeof(aos_time));
    t.year=2026;

    UTEST_ASSERT_FALSE(time_to_timestamp(null,&t));
    UTEST_ASSERT_FALSE(time_to_timestamp(null,null));
    UTEST_ASSERT_FALSE(timestamp_to_time(null,&ts));
    UTEST_ASSERT_FALSE(timestamp_to_time(null,null));
}

/**
 * 时间库函数测试。
 * 
 * @return 失败测试数。
 */
int32 time_test(void)
{
    UTEST_SUITE("aos.kernel.test.support.time");

    UTEST_RUN(time_timestamp_normalize_basic);
    UTEST_RUN(time_timestamp_normalize_timezone);
    UTEST_RUN(time_timestamp_normalize_null);

    UTEST_RUN(time_normalize_basic);
    UTEST_RUN(time_normalize_date);
    UTEST_RUN(time_normalize_timezone);
    UTEST_RUN(time_normalize_null);

    UTEST_RUN(time_timestamp_compare_basic);
    UTEST_RUN(time_timestamp_compare_null);

    UTEST_RUN(time_roundtrip_ad);
    UTEST_RUN(time_roundtrip_bc);
    UTEST_RUN(time_roundtrip_hms);

    UTEST_RUN(time_reverse_roundtrip_positive);
    UTEST_RUN(time_reverse_roundtrip_negative);

    UTEST_RUN(time_convert_null);

    UTEST_SUMMARY("aos.kernel.test.support.time");
}