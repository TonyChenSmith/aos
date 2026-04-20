/**
 * 内核线性区域管理系统。
 * @date 2026-03-30
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_KERNEL_MEMORY_VMA_H__
#define __AOS_KERNEL_MEMORY_VMA_H__

#include <support/const.h>

/**
 * VMA类型掩码。
 */
#define VMA_TYPE_MASK UINT8_MAX

/**
 * 保留类型。该类型会阻止分配实际页面。
 */
#define VMA_TYPE_RESERVED 0

/**
 * 内存类型。该类型代表普通可读写内存。
 */
#define VMA_TYPE_MEMORY 1

/**
 * 设备类型。该类型代表一般设备空间。
 */
#define VMA_TYPE_DEVICE 2

/**
 * 读优化设备类型。该类型代表可以在读操作优化的设备空间。
 */
#define VMA_TYPE_READ_OPTIMIZED_DEVICE 3

/**
 * 写优化设备类型。该类型代表可以在写操作优化的设备空间。
 */
#define VMA_TYPE_WRITE_OPTIMIZED_DEVICE 4

/**
 * 可读标志。表示该区域可以读，未设置则不可读。
 */
#define VMA_FLAG_READ BIT8

/**
 * 可写标志。表示该区域可以写，未设置则不可写。
 */
#define VMA_FLAG_WRITE BIT9

/**
 * 可执行标志。表示该区域可以执行，未设置则不可执行。
 */
#define VMA_FLAG_EXECUTE BIT10

/**
 * 用户级标志。表示区域为用户级权限，反之为内核级权限。
 */
#define VMA_FLAG_USER BIT11

/**
 * 区域全局标志。表示该区域的全局可见性，优化访问性能。
 */
#define VMA_FLAG_GLOBAL BIT12

/**
 * 大页标志。表示该区域应由大小大于单位页的大页组成，
 * 如果实际硬件支持大页的话不能映射成大页就报错，但硬件无大小页区分则忽视该标志。
 */
#define VMA_FLAG_HUGEPAGE BIT13

#endif /*__AOS_KERNEL_MEMORY_VMA_H__*/