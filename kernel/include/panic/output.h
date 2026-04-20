/**
 * 内核恐慌输出管理。
 * @date 2026-04-18
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_KERNEL_PANIC_OUTPUT_H__
#define __AOS_KERNEL_PANIC_OUTPUT_H__

#include <support/const.h>
#include <support/varargs.h>

/**
 * 恐慌输出。
 * 
 * @param format 适用于支持库格式化规则的格式化字符串。
 * @param args   可变参数列表。
 * 
 * @return 无返回值。
 */
typedef void (*panic_output_handler)(const char8* format,va_list args);

/**
 * 恐慌输出结点。
 */
typedef struct _panic_output_node panic_output_node;

struct _panic_output_node
{
    panic_output_node*   prev;   /*前一结点。*/
    panic_output_node*   next;   /*后一结点。*/
    panic_output_handler output; /*输出函数。*/
};

/**
 * 注册一个恐慌输出函数。
 * 
 * @param node 恐慌输出结点。结点内存应该由各模块提供稳定内存区域。
 * 
 * @return 无返回值。
 */
void register_panic_output(panic_output_node* node);

/**
 * 注销一个恐慌输出函数。
 * 
 * @param node 恐慌输出结点。结点内存应该由各模块提供稳定内存区域。
 * 
 * @return 无返回值。
 */
void unregister_panic_output(panic_output_node* node);

/**
 * 设置恐慌托管输出函数。当开始调用该函数后将不再返回。
 * 
 * @param output 托管输出函数。参数为空可以清空设置，使用恐慌默认方案。
 * 
 * @return 无返回值。
 */
void set_panic_managed_output(panic_output_handler output);

#endif /*__AOS_KERNEL_PANIC_OUTPUT_H__*/