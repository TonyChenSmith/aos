/**
 * 内核恐慌处理。
 * @date 2026-04-07
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include <panic/callback.h>
#include <panic/output.h>
#include <support/control.h>

/**
 * 恐慌回调链表头结点。
 */
static panic_callback_node* callback_head=null;

/**
 * 恐慌回调链表尾结点。
 */
static panic_callback_node* callback_tail=null;

/**
 * 注册一个恐慌回调函数。
 * 
 * @param node 恐慌回调结点。结点内存应该由各模块提供稳定内存区域。
 * 
 * @return 无返回值。
 */
void register_panic_callback(panic_callback_node* node)
{
    if(callback_head==null)
    {
        callback_head=node;
        callback_tail=node;
        node->prev=null;
        node->next=null;
    }
    else
    {
        node->next=callback_head;
        callback_head->prev=node;
        callback_head=node;
        callback_head->prev=null;
    }
}

/**
 * 注册一个恐慌回调函数在某一已注册恐慌回调之前。
 * 
 * @param node 恐慌回调结点。结点内存应该由各模块提供稳定内存区域。
 * @param next 已经在回调链表里的预计后一结点。
 * 
 * @return 无返回值。
 */
void register_panic_callback_before(panic_callback_node* node,panic_callback_node* next)
{
    panic_callback_node* prev=next->prev;

    if(prev==null)
    {
        node->next=next;
        next->prev=node;
        callback_head=node;
        callback_head->prev=null;
    }
    else
    {
        prev->next=node;
        next->prev=node;
        node->prev=prev;
        node->next=next;
    }
}

/**
 * 注册一个恐慌回调函数在某一已注册恐慌回调之后。
 * 
 * @param node 恐慌回调结点。结点内存应该由各模块提供稳定内存区域。
 * @param prev 已经在回调链表里的预计前一结点。
 * 
 * @return 无返回值。
 */
void register_panic_callback_after(panic_callback_node* node,panic_callback_node* prev)
{
    panic_callback_node* next=prev->next;

    if(next==null)
    {
        node->prev=prev;
        prev->next=node;
        callback_tail=node;
        callback_tail->next=null;
    }
    else
    {
        prev->next=node;
        next->prev=node;
        node->prev=prev;
        node->next=next;
    }
}

/**
 * 注销一个恐慌回调函数。
 * 
 * @param node 恐慌回调结点。结点内存应该由各模块提供稳定内存区域。
 * 
 * @return 无返回值。
 */
void unregister_panic_callback(panic_callback_node* node)
{
    panic_callback_node* prev=node->prev;
    panic_callback_node* next=node->next;

    if(prev==null&&next==null)
    {
        callback_head=null;
        callback_tail=null;
    }
    else if(prev==null)
    {
        callback_head=next;
        callback_head->prev=null;
        node->next=null;
    }
    else if(next==null)
    {
        callback_tail=prev;
        callback_tail->next=null;
        node->prev=null;
    }
    else
    {
        prev->next=next;
        next->prev=prev;
        node->prev=null;
        node->next=null;
    }
}

/**
 * 恐慌输出链表头结点。
 */
static panic_output_node* output_head=null;

/**
 * 恐慌输出链表尾结点。
 */
static panic_output_node* output_tail=null;

/**
 * 注册一个恐慌输出函数。
 * 
 * @param node 恐慌输出结点。结点内存应该由各模块提供稳定内存区域。
 * 
 * @return 无返回值。
 */
void register_panic_output(panic_output_node* node)
{
    if(output_head==null)
    {
        output_head=node;
        output_tail=node;
        node->prev=null;
        node->next=null;
    }
    else
    {
        node->next=output_head;
        output_head->prev=node;
        output_head=node;
        output_head->prev=null;
    }
}

/**
 * 注销一个恐慌输出函数。
 * 
 * @param node 恐慌输出结点。结点内存应该由各模块提供稳定内存区域。
 * 
 * @return 无返回值。
 */
void unregister_panic_output(panic_output_node* node)
{
    panic_output_node* prev=node->prev;
    panic_output_node* next=node->next;

    if(prev==null&&next==null)
    {
        output_head=null;
        output_tail=null;
    }
    else if(prev==null)
    {
        output_head=next;
        output_head->prev=null;
        node->next=null;
    }
    else if(next==null)
    {
        output_tail=prev;
        output_tail->next=null;
        node->prev=null;
    }
    else
    {
        prev->next=next;
        next->prev=prev;
        node->prev=null;
        node->next=null;
    }
}

/**
 * 托管输出。
 */
static panic_output_handler managed=null;

/**
 * 设置恐慌托管输出函数。当开始调用该函数后将不再返回。
 * 
 * @param output 托管输出函数。参数为空可以清空设置，使用恐慌默认方案。
 * 
 * @return 无返回值。
 */
void set_panic_managed_output(panic_output_handler output)
{
    managed=output;
}

/**
 * 恐慌函数。调用后不再返回。
 * 
 * @param format 适用于支持库格式化规则的格式化字符串。
 * 
 * @return 无法返回。
 */
void noreturn panic(const char8* format,...)
{
    disable_interrupts();

    /*多核通知恐慌，这里还未实现*/

    panic_callback_node* callback=callback_head;
    while(callback!=null)
    {
        callback->callback();
        callback=callback->next;
    }

    panic_output_node* output=output_head;
    va_list args;
    va_start(args,format);
    while(output!=null)
    {
        va_list list;
        va_copy(list,args);
        output->output(format,list);
        va_end(list);
        output=output->next;
    }

    if(managed!=null)
    {
        managed(format,args);

        /*写个特权死循环，但原则上回调不应该能执行该位置*/
        while(true)
        {
            cpu_halt();
        }
    }
    va_end(args);

    while(true)
    {
        cpu_halt();
    }
}