/*
 * 基础类型。
 * @date 2024-12-12
 *
 * Copyright (c) 2024-2025 Tony Chen Smith. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_BASIC_TYPE_H__
#define __AOS_BASIC_TYPE_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/*基本类型*/
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

/*地址位宽类型*/
typedef size_t uintn;

/*句柄类型*/
typedef uintn handle;

/*句柄未定义*/
#define HANDLE_UNDEFINED SIZE_MAX

/*QEMU调试端口地址*/
#define QEMU_DEBUGCON 0x402

/*调试信息*/
#ifdef DEBUG
#define DEBUG_START do{
#define DEBUG_END }while(false);
#else
#define DEBUG_START while(false){
#define DEBUG_END }
#endif

#endif /*__AOS_BASIC_TYPE_H__*/