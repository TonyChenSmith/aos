/**
 * 内核原子操作函数。与C11标准库原子操作库相似。
 * @date 2026-03-29
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_KERNEL_SUPPORT_ATOMIC_H__
#define __AOS_KERNEL_SUPPORT_ATOMIC_H__

#include "const.h"

/**
 * 内存顺序。
 */
typedef enum memory_order
{
    MEMORY_ORDER_RELAXED=__ATOMIC_RELAXED, /*宽松。*/
    MEMORY_ORDER_CONSUME=__ATOMIC_CONSUME, /*消费。*/
    MEMORY_ORDER_ACQUIRE=__ATOMIC_ACQUIRE, /*获取。*/
    MEMORY_ORDER_RELEASE=__ATOMIC_RELEASE, /*释放。*/
    MEMORY_ORDER_ACQ_REL=__ATOMIC_ACQ_REL, /*获取释放。*/
    MEMORY_ORDER_SEQ_CST=__ATOMIC_SEQ_CST  /*顺序一致性。*/
} memory_order;

/**
 * 有符号8位整型。
 */
typedef _Atomic(int8) atomic_int8;

/**
 * 有符号16位整型。
 */
typedef _Atomic(int16) atomic_int16;

/**
 * 有符号32位整型。
 */
typedef _Atomic(int32) atomic_int32;

/**
 * 有符号64位整型。
 */
typedef _Atomic(int64) atomic_int64;

/**
 * 无符号8位整型。
 */
typedef _Atomic(uint8) atomic_uint8;

/**
 * 无符号16位整型。
 */
typedef _Atomic(uint16) atomic_uint16;

/**
 * 无符号32位整型。
 */
typedef _Atomic(uint32) atomic_uint32;

/**
 * 无符号64位整型。
 */
typedef _Atomic(uint64) atomic_uint64;

/**
 * 有符号指针整型。
 */
typedef _Atomic(intn) atomic_intn;

/**
 * 无符号指针整型。
 */
typedef _Atomic(uintn) atomic_uintn;

/**
 * 布尔类型。
 */
typedef _Atomic(bool) atomic_bool;

/**
 * 标志类型。
 */
typedef atomic_bool atomic_flag;

/**
 * 终止依赖。
 */
#define kill_dependency(y) (y)

/**
 * 设置线程围栏。
 */
#define atomic_thread_fence(order) __c11_atomic_thread_fence(order)

/**
 * 设置信号围栏。
 */
#define atomic_signal_fence(order) __c11_atomic_signal_fence(order)

/**
 * 检查类型大小是否支持免锁。
 */
#define atomic_is_lock_free(size) __c11_atomic_is_lock_free(size)

/**
 * 原子存储。
 */
#define atomic_store(object,desired) __c11_atomic_store(object,desired,__ATOMIC_SEQ_CST)

/**
 * 原子加载。
 */
#define atomic_load(object) __c11_atomic_load(object,__ATOMIC_SEQ_CST)

/**
 * 原子交换。
 */
#define atomic_exchange(object,desired) __c11_atomic_exchange(object,desired,__ATOMIC_SEQ_CST)

/**
 * 原子强比较并交换。
 */
#define atomic_compare_exchange_strong(object,expected,desired) \
    __c11_atomic_compare_exchange_strong(object,expected,desired,__ATOMIC_SEQ_CST,__ATOMIC_SEQ_CST)

/**
 * 原子弱比较并交换。
 */
#define atomic_compare_exchange_weak(object,expected,desired) \
    __c11_atomic_compare_exchange_weak(object,expected,desired,__ATOMIC_SEQ_CST,__ATOMIC_SEQ_CST)

/**
 * 原子加法。
 */
#define atomic_fetch_add(object,operand) __c11_atomic_fetch_add(object,operand,__ATOMIC_SEQ_CST)

/**
 * 原子减法。
 */
#define atomic_fetch_sub(object,operand) __c11_atomic_fetch_sub(object,operand,__ATOMIC_SEQ_CST)

/**
 * 原子位或。
 */
#define atomic_fetch_or(object,operand) __c11_atomic_fetch_or(object,operand,__ATOMIC_SEQ_CST)

/**
 * 原子位异或。
 */
#define atomic_fetch_xor(object,operand) __c11_atomic_fetch_xor(object,operand,__ATOMIC_SEQ_CST)

/**
 * 原子位与。
 */
#define atomic_fetch_and(object,operand) __c11_atomic_fetch_and(object,operand,__ATOMIC_SEQ_CST)

/**
 * 原子位与非。
 */
#define atomic_fetch_nand(object,operand) __c11_atomic_fetch_nand(object,operand,__ATOMIC_SEQ_CST)

/**
 * 原子最大值。
 */
#define atomic_fetch_max(object,operand) __c11_atomic_fetch_max(object,operand,__ATOMIC_SEQ_CST)

/**
 * 原子最小值。
 */
#define atomic_fetch_min(object,operand) __c11_atomic_fetch_min(object,operand,__ATOMIC_SEQ_CST)

/**
 * 原子标志测试并设置。
 */
#define atomic_flag_test_and_set(object) __c11_atomic_exchange(object,true,__ATOMIC_SEQ_CST)

/**
 * 显式内存顺序的原子标志测试并设置。
 */
#define atomic_flag_test_and_set_explicit(object,order) __c11_atomic_exchange(object,true,order)

/**
 * 原子标志清除。
 */
#define atomic_flag_clear(object) __c11_atomic_store(object,false,__ATOMIC_SEQ_CST)

/**
 * 显式内存顺序的原子标志清除。
 */
#define atomic_flag_clear_explicit(object,order) __c11_atomic_store(object,false,order)

/**
 * 原子变量初始化。
 */
#define atomic_init __c11_atomic_init

/**
 * 显式内存顺序的原子存储。
 */
#define atomic_store_explicit __c11_atomic_store

/**
 * 显式内存顺序的原子加载。
 */
#define atomic_load_explicit __c11_atomic_load

/**
 * 显式内存顺序的原子交换。
 */
#define atomic_exchange_explicit __c11_atomic_exchange

/**
 * 显式内存顺序的原子强比较并交换。
 */
#define atomic_compare_exchange_strong_explicit __c11_atomic_compare_exchange_strong

/**
 * 显式内存顺序的原子弱比较并交换。
 */
#define atomic_compare_exchange_weak_explicit __c11_atomic_compare_exchange_weak

/**
 * 显式内存顺序的原子加法。
 */
#define atomic_fetch_add_explicit __c11_atomic_fetch_add

/**
 * 显式内存顺序的原子减法。
 */
#define atomic_fetch_sub_explicit __c11_atomic_fetch_sub

/**
 * 显式内存顺序的原子位或。
 */
#define atomic_fetch_or_explicit __c11_atomic_fetch_or

/**
 * 显式内存顺序的原子位异或。
 */
#define atomic_fetch_xor_explicit __c11_atomic_fetch_xor

/**
 * 显式内存顺序的原子位与。
 */
#define atomic_fetch_and_explicit __c11_atomic_fetch_and

/**
 * 显式内存顺序的原子位与非。
 */
#define atomic_fetch_nand_explicit __c11_atomic_fetch_nand

/**
 * 显式内存顺序的原子最大值。
 */
#define atomic_fetch_max_explicit __c11_atomic_fetch_max

/**
 * 显式内存顺序的原子最小值。
 */
#define atomic_fetch_min_explicit __c11_atomic_fetch_min

/**
 * 标志初始化值。
 */
#define ATOMIC_FLAG_INIT false

#endif /*__AOS_KERNEL_SUPPORT_ATOMIC_H__*/