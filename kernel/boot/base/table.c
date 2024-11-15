/*
 * 基础模块函数表实现。
 * @date 2024-10-27
 */
#include "base.h"
#include "impl.h"
#include "param.h"

/*基础模块函数表*/
static boot_base_functions table;

/*MSR地址*/
static const uint32 IA32_MISC_ENABLE=0x000001A0;

/*
 * 基础模块初始化函数,主要是在支持ERMS的CPU上确保开启了ERMS。
 *
 * @param params 启动参数结构指针。
 * 
 * @return 基础模块函数表指针。
 */
extern boot_base_functions* boot_init_base(const boot_params* params)
{
	if(params->env.cpu_info.erms)
	{
		_base_wrmsr(IA32_MISC_ENABLE,_base_rdmsr(IA32_MISC_ENABLE)|1);
	}
	table.boot_ms_call_2=base_ms_call_2;
	table.boot_ms_call_4=base_ms_call_4;
	table.boot_memcpy=base_memcpy;
	return &table;
}