/**
 * 内核时间结构功能。
 * @date 2026-05-17
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_KERNEL_SUPPORT_TIME_H__
#define __AOS_KERNEL_SUPPORT_TIME_H__

#include "type.h"

/**
 * 内核时间戳。
 * 
 * 以Unix纪元为起点的线性时间表示。
 * 精度为纳秒。用于时间计算、比较和内部时间存储。
 * 时区和夏令时状态仅存储和转换用，时间戳计算一直按UTC处理。
 */
typedef struct _aos_timestamp
{
    int64  second;     /*秒数。*/
    uint32 nanosecond; /*纳秒。*/
    int16  timezone;   /*时区偏移。分钟为单位，东为正。*/
    bool   daylight;   /*夏令时状态。*/
} aos_timestamp;

/**
 * 内核时间。
 * 
 * 以日历字段表示墙上时钟时间点，可表示范围与aos_timestamp一致。
 * 公元前年份用负数表示，遵循天文年份惯例：公元前1年为0，公元前2年为-1，以此类推。
 */
typedef struct _aos_time
{
    int64  year;       /*年。*/
    uint32 nanosecond; /*纳秒。*/
    int16  timezone;   /*时区偏移。分钟为单位，东为正，范围[-1440,1440]。*/
    uint8  month;      /*月。范围[0,11]。*/
    uint8  day;        /*日。范围[0,当月天数)。*/
    uint8  hour;       /*时。范围[0,23]。*/
    uint8  minute;     /*分。范围[0,59]。*/
    uint8  second;     /*秒。范围[0,59]。*/
    bool   daylight;   /*夏令时状态。*/
} aos_time;

/**
 * 将内核时间转换成内核时间戳。
 * 
 * @param timestamp 内核时间戳。
 * @param time      内核时间。
 * 
 * @return 正常转换返回真，错误转换返回假。
 */
bool time_to_timestamp(aos_timestamp* timestamp,const aos_time* time);

/**
 * 将内核时间戳转换成内核时间。
 * 
 * @param time      内核时间。
 * @param timestamp 内核时间戳。
 * 
 * @return 正常转换返回真，错误转换返回假。
 */
bool timestamp_to_time(aos_time* time,const aos_timestamp* timestamp);

/**
 * 将内核时间规范化。
 * 确保各字段在正确范围。
 * 
 * @param time 待规范化的内核时间。
 * 
 * @return 无返回值。
 */
void time_normalize(aos_time* time);

/**
 * 将内核时间戳规范化。
 * 确保纳秒字段归位到[0,999999999]范围内，将溢出或借位的纳秒正确进位到秒字段。
 * 
 * @param timestamp 待规范化的内核时间戳。
 * 
 * @return 无返回值。
 */
void timestamp_normalize(aos_timestamp* timestamp);

/**
 * 比较两个内核时间戳。
 *
 * @param a 第一个内核时间戳。
 * @param b 第二个内核时间戳。
 *
 * @return a早于b返回负数，a等于b返回0，a晚于b返回正数。
 */
int8 timestamp_compare(const aos_timestamp* a,const aos_timestamp* b);

#endif /*__AOS_KERNEL_SUPPORT_TIME_H__*/