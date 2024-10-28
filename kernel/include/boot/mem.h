/*
 * 内存模块函数和结构定义。定义了所有导出函数和入口函数。
 * @date 2024-10-28
 */
#ifndef __AOS_BOOT_MEM_H__
#define __AOS_BOOT_MEM_H__

#include "type.h"
#include "param.h"
#include "base.h"

/*
 * 初始化内存空间。其内有四个步骤：设置CPU寄存器、构造页表、切换页表和切换运行栈。
 *
 * @param params 被调用函数地址。
 * @param functions 被调用函数的第一个参数。
 * @param arg2 被调用函数的第二个参数。
 * 
 * @return 因为切换运行栈，不能返回。
 */
extern void boot_init_memory(const boot_params* restrict params,const boot_base_functions* restrict functions);

#endif /*__AOS_BOOT_MEM_H__*/