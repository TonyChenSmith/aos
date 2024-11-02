/*
 * 内存模块初始化函数。
 * @date 2024-10-28
 */
#include "mem.h"

/*
 * 初始化内存空间。其内有四个步骤：设置CPU寄存器、构造页表、切换页表和切换运行栈。
 *
 * @param params	 启动参数结构指针。
 * @param base_funcs 基础模块函数表指针。
 * 
 * @return 因为切换运行栈，不能返回。
 */
extern void boot_init_memory(const boot_params* params,const boot_base_functions* base_funcs)
{

}