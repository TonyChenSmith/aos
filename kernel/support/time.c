/**
 * 内核时间结构功能。
 * @date 2026-05-31
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include <support/const.h>
#include <support/memory.h>
#include <support/time.h>

/**
 * 判断本年是否为闰年。
 * 按格里历与前推格里历置闰，
 * 
 * @param year 年。
 * 
 * @return 闰年返回真。
 */
static bool time_is_leap_year(int64 year)
{
    return (year%400==0)||(year%4==0&&year%100!=0);
}

/**
 * 获取本年相对应月份的日数。
 * 
 * @param year  年。
 * @param month 月。
 * 
 * @return 对应天数。
 */
static uint8 time_days_in_month(int64 year,uint8 month)
{
    switch(month)
    {
        case 0:
        case 2:
        case 4:
        case 6:
        case 7:
        case 9:
        case 11:
            return 31;
        case 3:
        case 5:
        case 8:
        case 10:
            return 30;
        default:
            if(time_is_leap_year(year))
            {
                return 29;
            }
            else
            {
                return 28;
            }
    }
}

/**
 * 计算从纪元到指定日期的天数。
 * 计算前对参数进行规范化。结果可为负，表示早于纪元。
 * 
 * @param year  天文年份。
 * @param month 月。
 * @param day   日。
 * 
 * @return 距纪元的天数。
 */
static int64 time_days_from_epoch(int64 year,uint8 month,uint8 day)
{
    year+=month/12;
    month=month%12;
    int64 y=year-1;
    int64 days=0;

    if(y<0)
    {
        y=-y;
        days=-(y*365+(y+3)/4-(y+99)/100+(y+399)/400);
    }
    else
    {
        days=y*365+y/4-y/100+y/400;
    }

    for(uint8 m=0;m<month;m++)
    {
        days+=time_days_in_month(year,m);
    }

    days+=day;
    return days-((1970-1)*365+(1970-1)/4-(1970-1)/100+(1970-1)/400);
}

/**
 * 从距纪元的天数反推日期。
 * 
 * @param days  距纪元的天数。
 * @param year  天文年份。
 * @param month 月。
 * @param day   日。
 * 
 * @return 无返回值。
 */
static void time_date_from_days(int64 days,int64* year,uint8* month,uint8* day)
{
    int64 n;
    int64 y;
    int64 d;
    uint8 m;

    n=days+(1970-1)*365+(1970-1)/4-(1970-1)/100+(1970-1)/400;
    y=n/366;

    if(n>=0)
    {
        while(y*365+y/4-y/100+y/400<=n)
        {
            y++;
        }
        *year=y;
        d=n-((y-1)*365+(y-1)/4-(y-1)/100+(y-1)/400);
    }
    else
    {
        while(-y*365+(-y+3)/4-(-y+99)/100+(-y+399)/400<-n)
        {
            y--;
        }
        *year=y+1;
        d=n+(-y*365+(-y+3)/4-(-y+99)/100+(-y+399)/400);
    }

    m=0;
    while(d>=time_days_in_month(*year,m))
    {
        d-=time_days_in_month(*year,m);
        m++;
    }

    *month=m;
    *day=(uint8)d;
}

/**
 * 将内核时间规范化。
 * 确保各字段在正确范围。
 * 
 * @param time 待规范化的内核时间。
 * 
 * @return 无返回值。
 */
void time_normalize(aos_time* time)
{
    if(time==null)
    {
        return;
    }
    uint32 diff=time->nanosecond/1000000000;
    if(time->nanosecond>=1000000000)
    {
        time->nanosecond%=1000000000;
    }
    diff+=time->second;
    time->second=(uint8)(diff%60);
    diff=diff/60;
    diff+=time->minute;
    time->minute=(uint8)(diff%60);
    diff=diff/60;
    diff+=time->hour;
    time->hour=(uint8)(diff%24);
    diff=diff/24;
    diff+=time->day;

    int64 days=time_days_from_epoch(time->year,time->month,diff);
    time_date_from_days(days,&time->year,&time->month,&time->day);

    time->timezone%=2880;
    if(time->timezone>1440)
    {
        time->timezone-=2880;
    }
    else if(time->timezone<-1440)
    {
        time->timezone+=2880;
    }

    time->daylight=time->daylight?true:false;
}

/**
 * 将内核时间戳规范化。
 * 确保纳秒字段归位到[0,999999999]范围内，将溢出或借位的纳秒正确进位到秒字段。
 * 
 * @param timestamp 待规范化的内核时间戳。
 * 
 * @return 无返回值。
 */
void timestamp_normalize(aos_timestamp* timestamp)
{
    if(timestamp==null)
    {
        return;
    }
    uint32 diff=timestamp->nanosecond/1000000000;
    if(timestamp->nanosecond>=1000000000)
    {
        timestamp->nanosecond%=1000000000;
    }
    timestamp->second+=diff;

    timestamp->timezone%=2880;
    if(timestamp->timezone>1440)
    {
        timestamp->timezone-=2880;
    }
    else if(timestamp->timezone<-1440)
    {
        timestamp->timezone+=2880;
    }

    timestamp->daylight=timestamp->daylight?true:false;
}

/**
 * 将内核时间转换成内核时间戳。
 * 
 * @param timestamp 内核时间戳。
 * @param time      内核时间。
 * 
 * @return 正常转换返回真，错误转换返回假。
 */
bool time_to_timestamp(aos_timestamp* timestamp,const aos_time* time)
{
    if(timestamp==null||time==null)
    {
        return false;
    }

    aos_time t;
    memory_copy(&t,time,sizeof(aos_time));
    time_normalize(&t);

    timestamp->second=time_days_from_epoch(t.year,t.month,t.day)*(24*60*60);
    timestamp->second+=t.hour*60*60;
    timestamp->second+=t.minute*60;
    timestamp->second+=t.second;
    timestamp->nanosecond=t.nanosecond;
    timestamp->timezone=t.timezone;
    timestamp->daylight=t.daylight;

    timestamp->second-=t.timezone*60;

    return true;
}

/**
 * 将内核时间戳转换成内核时间。
 * 
 * @param time      内核时间。
 * @param timestamp 内核时间戳。
 * 
 * @return 正常转换返回真，错误转换返回假。
 */
bool timestamp_to_time(aos_time* time,const aos_timestamp* timestamp)
{
    if(timestamp==null||time==null)
    {
        return false;
    }

    aos_timestamp t;
    memory_copy(&t,timestamp,sizeof(aos_timestamp));
    timestamp_normalize(&t);

    t.second+=t.timezone*60;
    time->timezone=t.timezone;
    time->daylight=t.daylight;
    time->nanosecond=t.nanosecond;

    int64 n=t.second/(24*60*60);
    if(t.second<0&&t.second%(24*60*60)!=0)
    {
        n--;
    }
    time_date_from_days(n,&time->year,&time->month,&time->day);

    n=t.second%(24*60*60);
    if(n<0)
    {
        n+=(24*60*60);
    }
    time->hour=n/(60*60);
    n=n%(60*60);
    time->minute=n/60;
    time->second=n%60;

    return true;
}

/**
 * 比较两个内核时间戳。
 *
 * @param a 第一个内核时间戳。
 * @param b 第二个内核时间戳。
 *
 * @return a小于b返回负数，a等于b返回0，a大于b返回正数。空代表最小值。
 */
int8 timestamp_compare(const aos_timestamp* a,const aos_timestamp* b)
{
    if(a==null&&b==null)
    {
        return 0;
    }
    else if(a!=null&&b==null)
    {
        return 1;
    }
    else if(a==null&&b!=null)
    {
        return -1;
    }
    else
    {
        if(a->second==b->second)
        {
            if(a->nanosecond<b->nanosecond)
            {
                return -1;
            }
            else if(a->nanosecond>b->nanosecond)
            {
                return 1;
            }
            else
            {
                return 0;
            }
        }
        else if(a->second<b->second)
        {
            return -1;
        }
        else
        {
            return 1;
        }
    }
}