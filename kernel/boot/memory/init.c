/*
 * 内存模块初始化函数。
 * @date 2024-10-28
 */
#include "module/memory.h"
#include "type.h"

static char buffer[512];
static char buffer1[512];
extern void print_bytes(const char*,uintn);

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

void list(uintn list,uintn head,uintn tail)
{
	print_bytes("List-",sizeof("List-")-1);
	print_num(list);
	print_bytes("[",sizeof("[")-1);
	print_num(head);
	print_bytes(",",sizeof(",")-1);
	print_num(tail);
	print_bytes("]:\n",sizeof("]:\n")-1);
}

void line(uintn index,uintn node,uintn start,uintn end,uintn amount,uintn type)
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

/*
 * 初始化内存空间。其内有四个步骤：构造页表、切换页表、设置CPU寄存器和切换运行栈。
 *
 * @param params	 启动参数结构指针。
 * @param base_funcs 基础模块函数表指针。
 * 
 * @return 因为切换运行栈，不能返回。
 */
extern void boot_init_memory(boot_params* params,const boot_base_functions* base_funcs)
{
	boot_pmm_init(params);
	while(1)
	{
		__asm__("hlt"::);
	}
}