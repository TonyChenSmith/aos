/*
 * 基础模块函数表实现。
 * @date 2024-10-27
 */
#include "base.h"
#include "baseimpl.h"
#include "param.h"

/*基础模块函数表*/
static boot_base_functions table;

/*
 * 读取64位MSR内容，作为内联函数实现。
 * 
 * @param reg 寄存器地址。
 * 
 * @return 寄存器的值。
 */
static inline uint64 __attribute__((__always_inline__,__nodebug__)) _boot_base_rdmsr(uint32 reg)
{
	uint32 edx,eax;
	__asm__("rdmsr":"=d"(edx),"=a"(eax):"c"(reg));
	return (((uint64)edx)<<32)|eax;
}

/*
 * 写入64位MSR内容，作为内联函数实现。
 * 
 * @param reg 寄存器地址。
 * @param val 写入值。
 */
static inline void __attribute__((__always_inline__,__nodebug__)) _boot_base_wrmsr(uint32 reg,uint64 value)
{
	uint32 edx=(uint32)(value>>32),eax=(uint32)value;
	__asm__("wrmsr"::"d"(edx),"a"(eax),"c"(reg));
}

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
		_boot_base_wrmsr(IA32_MISC_ENABLE,_boot_base_rdmsr(IA32_MISC_ENABLE)|1);
	}
	table.boot_ms_call_2=base_ms_call_2;
	table.boot_ms_call_4=base_ms_call_4;
	table.boot_memcpy=base_memcpy;
	return &table;
}