/*
 * aos内核基础类型定义。
 * @date 2024-10-23
 */
#ifndef __AOS_BASE_TYPE_H__
#define __AOS_BASE_TYPE_H__

/*这里的定义以Clang C11为基准*/
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long uint64;

typedef char int8;
typedef short int16;
typedef int int32;
typedef long int64;

/*x64平台，地址8字节*/
typedef uint64 uintn;

/*逻辑类型*/
typedef uint8 boolean;
#define true ((boolean)(1==1))
#define false ((boolean)(1==0))

#endif /*__AOS_BASE_TYPE_H__*/