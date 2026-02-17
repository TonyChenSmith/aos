/**
 * 内核输入输出操作库函数。
 * @date 2026-01-25
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_KERNEL_SUPPORT_IO_H__
#define __AOS_KERNEL_SUPPORT_IO_H__

#include "ktype.h"

/**
 * 输入输出句柄。
 */
typedef struct _io_handle io_handle;

/**
 * 从句柄读取一段数据。
 * 
 * @param handle 输入输出句柄。
 * @param dest   目标数组。
 * @param n      请求读取长度。
 * 
 * @return 返回实际读取长度，当返回0时应获取错误状态码。
 */
typedef uintn (*io_read)(io_handle* handle,uint8* dest,uintn n);

/**
 * 往句柄写入一段数据。
 * 
 * @param handle 输入输出句柄。
 * @param src    源数组。
 * @param n      请求写入长度。
 * 
 * @return 返回实际写入长度，当返回0时应获取错误状态码。
 */
typedef uintn (*io_write)(io_handle* handle,const uint8* src,uintn n);

/**
 * 刷新句柄缓冲区。
 * 
 * @param handle 输入输出句柄。
 * 
 * @return 正常刷新返回成功，出现问题返回错误状态码。如果句柄无缓冲区机制返回成功。
 */
typedef uint64 (*io_flush)(io_handle* handle);

/**
 * 获取句柄读写位置。
 * 
 * @param handle 输入输出句柄。
 * 
 * @return 句柄当前读写位置。
 */
typedef uint64 (*io_get_position)(io_handle* handle);

/**
 * 设置句柄读写位置。
 * 
 * @param handle   输入输出句柄。
 * @param position 目标位置。
 * 
 * @return 正常返回成功，设置错误返回错误状态码。
 */
typedef uint64 (*io_set_position)(io_handle* handle,uint64 position);

/**
 * 获取句柄当前可读写区域大小。
 * 
 * @param handle 输入输出句柄。
 * 
 * @return 当前可读写区域边界的大小，按字节计数。
 */
typedef uint64 (*io_get_size)(io_handle* handle);

/**
 * 获取句柄的能力组合。
 * 
 * @param handle 输入输出句柄。
 * 
 * @return 句柄的能力组合。
 */
typedef uint64 (*io_get_capabilities)(io_handle* handle);

/**
 * 获取句柄最近一次操作的状态码。
 * 
 * @param handle 输入输出句柄。
 * 
 * @return 一般没报错时返回成功，读写出错时返回它们设置的错误状态码，并将状态码重置为成功。
 */
typedef uint64 (*io_get_io_status)(io_handle* handle);

/**
 * 关闭句柄并回收资源。关闭时会先进行刷新缓冲区操作。
 * 
 * @param handle 输入输出句柄。
 * 
 * @return 正常执行返回成功，出现问题返回错误状态码。
 */
typedef uint64 (*io_close)(io_handle* handle);

struct _io_handle
{
    uint64              magic;            /*魔数。*/
    io_read             read;             /*读取。*/
    io_write            write;            /*写入。*/
    io_flush            flush;            /*刷新。*/
    io_get_position     get_position;     /*获取位置。*/
    io_set_position     set_position;     /*设置位置。*/
    io_get_size         get_size;         /*获取大小。*/
    io_get_capabilities get_capabilities; /*获取能力组合。*/
    io_get_io_status    get_io_status;    /*获取状态码。*/
    io_close            close;            /*关闭。*/
};

/**
 * 判断输入输出状态码是否为错误状态码。
 */
#define io_error(status) ((status)!=IO_STATUS_SUCCESS)

/**
 * 可读能力。设置该标志说明句柄可以读取。
 */
#define IO_CAPABILITY_READABLE BIT0

/**
 * 可写能力。设置该标志说明句柄可以写入。
 */
#define IO_CAPABILITY_WRITABLE BIT1

/**
 * 可寻址能力。设置该标志说明句柄的位置是数据区内的字节偏移量，可以前后移动访问数据的不同部分。
 * 如果没有该标志说明位置是寄存器索引，可以选择寄存器的位置进行读写。
 */
#define IO_CAPABILITY_SEEKABLE BIT2

/**
 * 可调整大小能力。表示数据区大小可变。寄存器数目在句柄创建后原则上就是不可变的，不应该有这个标志。
 */
#define IO_CAPABILITY_RESIZABLE BIT3

/**
 * 输入输出成功。
 */
#define IO_STATUS_SUCCESS 0

/**
 * 设备问题。
 */
#define IO_STATUS_DEVICE_ERROR 1

/**
 * 超出位置操作边界。
 */
#define IO_STATUS_OUT_OF_BOUNDS 2

/**
 * 操作不支持。
 */
#define IO_STATUS_NOT_SUPPORTED 3

#endif /*__AOS_KERNEL_SUPPORT_IO_H__*/