/*
 * 内存模块初始化函数。
 * @date 2024-10-28
 */
#include "mem.h"
#include "boot.h"

static char buffer[512];
static char buffer1[512];

static void print_num(uintn number)
{
	uintn index=0;
	uintn num=number;
	do
	{
		buffer[index++]=num%10+'0';
		num=num/10;
	} while(num>0);
	for(num=0;num<index;num++)
	{
		buffer1[num]=buffer[index-1-num];
	}
	print_bytes(buffer1,index);
}

static void line(const char* str,uintn number)
{
	uintn size=0;
	while(str[size]!=0)
	{
		size++;
	}
	print_bytes(str,size);
	print_num(number);
	print_bytes("\n",1);
}

#undef offset_of
#define offset_of(a,b) ((uintn)__builtin_offsetof(a,b))

static void print_boot(void)
{
	line("===AOS Boot Module Summary===\nboot_params_size=",sizeof(boot_params));
	line(".pool=",offset_of(boot_params,pool));
	line(".pool_length=",offset_of(boot_params,pool_length));
	line(".stack=",offset_of(boot_params,stack));
	line(".stack_length=",offset_of(boot_params,stack_length));
	line(".root_bridges=",offset_of(boot_params,root_bridges));
	line(".root_bridge_length=",offset_of(boot_params,root_bridge_length));
	line(".devices=",offset_of(boot_params,devices));
	line(".device_length=",offset_of(boot_params,device_length));
	line(".graphics_info=",offset_of(boot_params,graphics_info));
	line(".env=",offset_of(boot_params,env));
	line(".acpi=",offset_of(boot_params,acpi));
	line(".smbios=",offset_of(boot_params,smbios));
	line(".smbios3=",offset_of(boot_params,smbios3));
	line(".runtime=",offset_of(boot_params,runtime));
	line(".boot_device=",offset_of(boot_params,boot_device));
	line(".modules=",offset_of(boot_params,modules));
}

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
	print_bytes("Hello aos kernel mem.\n",sizeof("Hello aos kernel mem.\n")-1);
	print_boot();
	while(1)
	{
		aos_cpu_hlt();
	}
}