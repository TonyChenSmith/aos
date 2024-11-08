/*
 * 内存模块初始化函数。
 * @date 2024-10-28
 */
#include "mem.h"
#include "boot.h"
#include "param.h"
#include "type.h"

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

/*页面位映射*/
static uint64 page_bitmap[8]={0};

/*管理页面总数*/
static const uintn page_amount=512;

/*管理页面基址，连续内存*/
static uintn page_base=0;

/*
 * 设置页面管理参数，该管理系统最大预留2MB页，以位映射形式记录空间申请。
 * 
 * @param params 启动参数结构指针。
 *
 * @return 无返回值。
 */
static void boot_setup_page_manager(const boot_params* restrict params)
{
	page_base=((uintn)params->env.max_free)+(params->env.max_pages<<12)-(512<<12);
	if(params->env.max_pages<page_amount)
	{
		uintn reserved=page_amount-params->env.max_pages;
		uintn index=0;
		while(reserved>=64)
		{
			page_bitmap[index]=UINT64_MAX;
			index++;
			reserved=reserved-64;
		}
		while(reserved>0)
		{
			page_bitmap[index]=page_bitmap[index]<<1;
			page_bitmap[index]=page_bitmap[index]|1;
			reserved--;
		}
	}
}

/*
 * 申请一页。
 *
 * @return 申请成功返回指针，申请失败返回NULL。
 */
static void* boot_page_alloc()
{
	uintn index=8;
	while(index>0)
	{
		index--;
		if(page_bitmap[index]==0)
		{
			page_bitmap[index]=(uint64)1<<63;
			return (void*)(page_base+(index<<18)+(63<<12));
		}
		else
		{
			if(__popcntq(page_bitmap[index])==64)
			{
				/*全部分配完。*/
				index--;
			}
			else
			{
				uintn bit=64;
				while(bit>0)
				{
					bit--;
					if(page_bitmap[index]&((uint64)1<<bit))
					{
						continue;
					}
					page_bitmap[index]=page_bitmap[index]|((uint64)1<<bit);
					return (void*)(page_base+(index<<18)+(bit<<12));
				}
			}
		}
	}
	return NULL;
}

/*
 * 释放一页。
 *
 * @param page 页面基址。
 *
 * @return 无返回值。
 */
static void boot_page_free(const void* restrict page)
{
	uintn index=0;
	uintn pages=((uintn)page-page_base)>>12;
	while(pages>=64)
	{
		index++;
		pages=pages-64;
	}
	page_bitmap[index]=page_bitmap[index]&(~((uint64)1<<pages));
}

/*系统页表*/
static ia32_page_entry* kernel_page_table=0;

/*
 * 设置系统页表。
 *
 * @param params	 启动参数结构指针。
 * @param base_funcs 基础模块函数表指针。
 *
 * @return 无返回值。
 */
static void boot_setup_page_table(const boot_params* restrict params,const boot_base_functions* restrict base_funcs)
{
	kernel_page_table=(ia32_page_entry*)boot_page_alloc();

}

/*
 * 初始化内存空间。其内有四个步骤：构造页表、切换页表、设置CPU寄存器和切换运行栈。
 *
 * @param params	 启动参数结构指针。
 * @param base_funcs 基础模块函数表指针。
 * 
 * @return 因为切换运行栈，不能返回。
 */
extern void boot_init_memory(const boot_params* params,const boot_base_functions* base_funcs)
{
	boot_setup_page_manager(params);
	
	while(1)
	{
		aos_cpu_hlt();
	}
}