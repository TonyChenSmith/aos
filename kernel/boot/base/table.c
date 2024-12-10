/*
 * 基础模块函数表实现。
 * @date 2024-10-27
 */
#include "include/table.h"

/*基础模块函数表*/
static boot_base_functions table;

/*
 * 基础模块初始化函数，主要是在支持ERMS的CPU上确保开启了ERMS。
 *
 * @param params 启动参数结构指针。
 * 
 * @return 基础模块函数表指针。
 */
extern boot_base_functions* boot_init_base(const boot_params* params)
{
	if(params->env.cpu_info.erms)
	{
		builtin_wrmsr(IA32_MISC_ENABLE,builtin_rdmsr(IA32_MISC_ENABLE)|1);
	}
	table.boot_ms_call_2=base_ms_call_2;
	table.boot_ms_call_4=base_ms_call_4;
	table.boot_memcpy=base_memcpy;
	table.boot_memset=base_memset;
	return &table;
}