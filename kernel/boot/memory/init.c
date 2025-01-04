/*
 * 内存模块初始化函数。
 * @date 2024-10-28
 */
#include "fw/efi.h"
#include "include/boot_bitmap.h"
#include "memory/memory_defs.h"
#include "memory/page_frame.h"
#include "module/base.h"
#include "basic_type.h"
#include "param.h"
#include "include/pmm.h"
#include "util/bitmap_pool.h"
#include "include/boot_tree.h"

static char buffer[512];
static char buffer1[512];
static boot_base_functions* bf;
extern void print_bytes(const char* src,uintn n)
{
	bf->boot_writeport(src,PORT_WIDTH_8,0x402,n);
}

extern void prints(const char* src)
{
	uintn l=0;
	while(src[l])
	{
		l++;
	}
	print_bytes(src,l);
}

void print_num(uintn number)
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

void print_hex(uintn number)
{
	uintn index=0;
	uintn num=number;
	do
	{
		uintn n=num&0xF;
		if(n<10)
		{
			buffer[index++]=n+'0';
		}
		else
		{
			buffer[index++]=n-10+'A';
		}
		num=num>>4;
	} while(num>0);
	for(num=0;num<index;num++)
	{
		buffer1[num]=buffer[index-1-num];
	}
	print_bytes(buffer1,index);
}

static uintn id=0;

void pnode(efi_memory_descriptor* dsc)
{
	print_bytes("Node-",sizeof("Node-")-1);
	print_num(id++);
	print_bytes(":base=0x",sizeof(":base=0x")-1);
	print_hex(dsc->physical_start);
	print_bytes(",page=",sizeof(",page=")-1);
	print_num(dsc->pages);
	print_bytes(",type=",sizeof(",type=")-1);
	print_num(dsc->type);
	print_bytes(",support=0x",sizeof(",support=0x")-1);
	print_hex(dsc->attribute);
	print_bytes("\n",sizeof("\n")-1);
}

void plist(uintn list,uintn head,uintn tail)
{
	print_bytes("List-",sizeof("List-")-1);
	print_num(list);
	print_bytes("[",sizeof("[")-1);
	print_num(head);
	print_bytes(",",sizeof(",")-1);
	print_num(tail);
	print_bytes("]:\n",sizeof("]:\n")-1);
}

void pline(uintn index,uintn node,uintn start,uintn end,uintn amount,uintn type)
{
	print_bytes("Index-",sizeof("Index-")-1);
	print_num(index);
	print_bytes("(Node-",sizeof("(Node-")-1);
	print_num(node);
	print_bytes("):0x",sizeof("):0x")-1);
	print_hex(start);
	print_bytes("-0x",sizeof("-0x")-1);
	print_hex(end);
	print_bytes(",page=",sizeof(",page=")-1);
	print_num(amount);
	print_bytes(",type=",sizeof(",type=")-1);
	print_num(type);
	print_bytes("\n",1);
}

static uintn stack_base=HANDLE_UNDEFINED;
static uintn stack_length=HANDLE_UNDEFINED;
extern void boot_init_memory_step2(const boot_params* param,const boot_base_functions* bbft,const uintn stack);

/*
 * 初始化内存空间第一阶段。负责初始化物理伙伴系统，并在物理伙伴系统中申请栈并切换栈。
 *
 * @param param	启动参数结构。
 * @param bbft	基础模块函数表。
 * 
 * @return 因为切换运行栈，不能返回。
 */
extern void boot_init_memory_step1(boot_params* restrict param,const boot_base_functions* restrict bbft)
{
	bf=(boot_base_functions*)bbft;
	boot_pmm_init(param,bbft);
	stack_length=(BOOT_INITIAL_STACK*0x200)<<12;
	stack_base=boot_pmm_alloc(MALLOC_MAX,0,UINT32_MAX,BOOT_INITIAL_STACK*0x200,AOS_KERNEL_CODE);
	if(stack_base==HANDLE_UNDEFINED)
	{
		/*可用内存不足，可能是参数设置问题，也有可能是预设内存池大小问题*/
		DEBUG_START
		/*错误：在低4GB内存空间内，系统没有足够内存申请初始内核栈，请尝试重新配置BOOT_INITIAL_STACK。系统因此将在此处关机。*/
		#define BOOT_INITS_ERROR_MSG "Error:The system does not have enough memory to apply for the initial kernel stack in the low 4GB memory space,please try to reconfigure the BOOT_INITIAL_STACK.The system will therefore shut down here.\n"
		bbft->boot_writeport(BOOT_INITS_ERROR_MSG,PORT_WIDTH_8,QEMU_DEBUGCON,sizeof(BOOT_INITS_ERROR_MSG)-1);
		DEBUG_END
		bbft->boot_ms_call_4(param->runtime->reset_system,EFI_RESET_SHUTDOWN,EFI_OUT_OF_RESOURCES,0,0);
		__builtin_unreachable();
	}
	boot_init_memory_step2(param,bbft,stack_base+stack_length);
	__builtin_unreachable();
}

/*
 * 初始化内存空间。其内有四个步骤：构造页表、切换页表、设置CPU寄存器和切换运行栈。
 *
 * @param params 启动参数结构。
 * @param bfuncs 基础模块函数表。
 * 
 * @return 因为切换运行栈，不能返回。
 */
extern void boot_init_memory_step3(boot_params* params,const boot_base_functions* bfuncs)
{
	prints("hello,aos\n");
	while(1)
	{
		__asm__("hlt"::);
	}
	__builtin_unreachable();
}