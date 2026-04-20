/**
 * 内核恐慌回调管理。
 * @date 2026-04-06
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_KERNEL_PANIC_CALLBACK_H__
#define __AOS_KERNEL_PANIC_CALLBACK_H__

#include <support/type.h>

/**
 * 恐慌回调函数。
 * 
 * @return 无返回值。
 */
typedef void (*panic_callback)(void);

/**
 * 恐慌回调结点。
 */
typedef struct _panic_callback_node panic_callback_node;

struct _panic_callback_node
{
    panic_callback_node* prev;     /*前一结点。*/
    panic_callback_node* next;     /*后一结点。*/
    panic_callback       callback; /*回调函数。*/
};

/**
 * 注册一个恐慌回调函数。
 * 
 * @param node 恐慌回调结点。结点内存应该由各模块提供稳定内存区域。
 * 
 * @return 无返回值。
 */
void register_panic_callback(panic_callback_node* node);

/**
 * 注册一个恐慌回调函数在某一已注册恐慌回调之前。
 * 
 * @param node 恐慌回调结点。结点内存应该由各模块提供稳定内存区域。
 * @param next 已经在回调链表里的预计后一结点。
 * 
 * @return 无返回值。
 */
void register_panic_callback_before(panic_callback_node* node,panic_callback_node* next);

/**
 * 注册一个恐慌回调函数在某一已注册恐慌回调之后。
 * 
 * @param node 恐慌回调结点。结点内存应该由各模块提供稳定内存区域。
 * @param prev 已经在回调链表里的预计前一结点。
 * 
 * @return 无返回值。
 */
void register_panic_callback_after(panic_callback_node* node,panic_callback_node* prev);

/**
 * 注销一个恐慌回调函数。
 * 
 * @param node 恐慌回调结点。结点内存应该由各模块提供稳定内存区域。
 * 
 * @return 无返回值。
 */
void unregister_panic_callback(panic_callback_node* node);

#endif /*__AOS_KERNEL_PANIC_CALLBACK_H__*/