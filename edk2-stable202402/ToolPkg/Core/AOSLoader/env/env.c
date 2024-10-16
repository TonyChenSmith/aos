/*
 * AOS UEFI Bootloader环境管理。
 * 2024-05-29创建。
 */
#include "env/aosenv.h"

#pragma pack(1)

/*AOS用段表*/
typedef struct
{
	IA32_SEGMENT_DESCRIPTOR start; /*第一个为空*/
	IA32_SEGMENT_DESCRIPTOR code32; /*32位ring0代码*/
	IA32_SEGMENT_DESCRIPTOR data32; /*32位ring0数据*/
	IA32_SEGMENT_DESCRIPTOR code64; /*64位ring0代码*/
	IA32_SEGMENT_DESCRIPTOR data64; /*64位ring0数据*/
	IA32_SEGMENT_DESCRIPTOR code64a; /*64位ring3代码*/
	IA32_SEGMENT_DESCRIPTOR data64a; /*64位ring3数据*/
} AOS_GDT_ENTRIES;

#pragma pack()

/*各个段地址*/
#define AOS_SEGMENT_START aos_offset_of(AOS_GDT_ENTRIES,start)
#define AOS_SEGMENT_CODE32 aos_offset_of(AOS_GDT_ENTRIES,code32)
#define AOS_SEGMENT_DATA32 aos_offset_of(AOS_GDT_ENTRIES,data32)
#define AOS_SEGMENT_CODE64 aos_offset_of(AOS_GDT_ENTRIES,code64)
#define AOS_SEGMENT_DATA64 aos_offset_of(AOS_GDT_ENTRIES,data64)
#define AOS_SEGMENT_CODE64A aos_offset_of(AOS_GDT_ENTRIES,code64a)
#define AOS_SEGMENT_DATA64A aos_offset_of(AOS_GDT_ENTRIES,data64a)

/*GDT内容*/
CONST AOS_GDT_ENTRIES entries={
	.start={
		.Bits={
			.LimitLow=0,
    		.BaseLow=0,
    		.BaseMid=0,
    		.Type=0,
    		.S=0,
    		.DPL=0,
    		.P=0,
			.LimitHigh=0,
    		.AVL=0,
    		.L=0,
    		.DB=0,
    		.G=0,
    		.BaseHigh=0,
		}
	},
	.code32={
		.Bits={
			.LimitLow=0xFFFF,
    		.BaseLow=0,
    		.BaseMid=0,
    		.Type=0xA,
    		.S=1,
    		.DPL=0,
    		.P=1,
			.LimitHigh=0xF,
    		.AVL=0,
    		.L=0,
    		.DB=1,
    		.G=1,
    		.BaseHigh=0,
		}
	},
	.data32={
		.Bits={
			.LimitLow=0xFFFF,
    		.BaseLow=0,
    		.BaseMid=0,
    		.Type=0x2,
    		.S=1,
    		.DPL=0,
    		.P=1,
			.LimitHigh=0xF,
    		.AVL=0,
    		.L=0,
    		.DB=1,
    		.G=1,
    		.BaseHigh=0,
		}
	},
	.code64={
		.Bits={
			.LimitLow=0xFFFF,
    		.BaseLow=0,
    		.BaseMid=0,
    		.Type=0xA,
    		.S=1,
    		.DPL=0,
    		.P=1,
			.LimitHigh=0xF,
    		.AVL=0,
    		.L=1,
    		.DB=0,
    		.G=1,
    		.BaseHigh=0,
		}
	},
	.data64={
		.Bits={
			.LimitLow=0xFFFF,
    		.BaseLow=0,
    		.BaseMid=0,
    		.Type=0x2,
    		.S=1,
    		.DPL=0,
    		.P=1,
			.LimitHigh=0xF,
    		.AVL=0,
    		.L=0,
    		.DB=1,
    		.G=1,
    		.BaseHigh=0,
		}
	},
	.code64a={
		.Bits={
			.LimitLow=0xFFFF,
    		.BaseLow=0,
    		.BaseMid=0,
    		.Type=0xA,
    		.S=1,
    		.DPL=3,
    		.P=1,
			.LimitHigh=0xF,
    		.AVL=0,
    		.L=1,
    		.DB=0,
    		.G=1,
    		.BaseHigh=0,
		}
	},
	.data64a={
		.Bits={
			.LimitLow=0xFFFF,
    		.BaseLow=0,
    		.BaseMid=0,
    		.Type=0x2,
    		.S=1,
    		.DPL=3,
    		.P=1,
			.LimitHigh=0xF,
    		.AVL=0,
    		.L=0,
    		.DB=1,
    		.G=1,
    		.BaseHigh=0,
		}
	}
};

/*局部量。*/

/*当前未使用内存指针*/
static VOID* current_pointer;

/*池结尾*/
static UINTN pool_end;

/*函数*/

/*
 * 初始化内存。提前申请预留空间，后续需要长期保存的数据可以申请该空间。
 * 在此过程中，假设硬件支持16MB及以上大小的空间。
 */
EFI_STATUS
EFIAPI
aos_init_memory(VOID)
{
	EFI_STATUS status; /*状态*/
	EFI_PHYSICAL_ADDRESS pool;
	EFI_PHYSICAL_ADDRESS stack;

	pool=SIZE_4GB-1;
	status=gBS->AllocatePages(AllocateMaxAddress,EfiLoaderData,AOS_MEMORY_POOL_PAGES,&pool);
	if(EFI_ERROR(status))
	{
		return status;
	}
	pool_end=pool+aos_pages_to_size(AOS_MEMORY_POOL_PAGES);
	boot_params.pool=(VOID*)pool;
	boot_params.pool_length=aos_pages_to_size(AOS_MEMORY_POOL_PAGES);

	DEBUG((DEBUG_INFO,"==AOS Memory Allocate==\n"));
	DEBUG((DEBUG_INFO,"Memory Pool:B=0x%lX,E=0x%lX\n",pool,pool_end));

	current_pointer=boot_params.pool;
	stack=SIZE_4GB-1;
	status=gBS->AllocatePages(AllocateMaxAddress,EfiLoaderData,AOS_STACK_PAGES,&stack);
	if(EFI_ERROR(status))
	{
		return status;
	}
	boot_params.stack=(VOID*)stack;
	boot_params.stack_length=aos_pages_to_size(AOS_STACK_PAGES);

	DEBUG((DEBUG_INFO,"Stack:B=0x%lX\n",stack));

	return EFI_SUCCESS;
}

/*
 * 申请内存页，数目足够就返回地址，否则为空。
 */
VOID*
EFIAPI
aos_allocate_pages(
	IN UINTN pages
)
{
	UINTN address_pages;
	VOID* address=NULL;
	UINTN address_size=aos_pages_to_size(pages);
	address_pages=aos_size_to_pages((UINTN)current_pointer);
	address=(VOID*)aos_pages_to_size(address_pages);
	if((aos_pages_to_size(address_pages)+address_size)>pool_end)
	{
		return NULL;
	}
	current_pointer=(VOID*)(aos_pages_to_size(address_pages)+address_size);
	return address;
}

/*
 * 申请内存池，数目足够就返回地址，否则为空。
 */
VOID*
EFIAPI
aos_allocate_pool(
	IN UINTN size
)
{
	VOID* address=NULL;
	if(((UINTN)current_pointer+size)>pool_end)
	{
		return NULL;
	}
	address=current_pointer;
	current_pointer=(VOID*)((UINTN)current_pointer+size);
	return address;
}

/*
 * 创建配置。主要是BSP的段表、中断表、页表和MTRR。
 * 由于TSS结合了APIC，所以该函数需要在结束启动服务后调用。
 */
EFI_STATUS
EFIAPI
aos_create_config(VOID)
{
	VOID* page;

	/*设置GDT*/
	page=aos_allocate_pages(1);
	CopyMem(page,(VOID*)&entries,sizeof(entries));
	boot_params.env.gdtr.Limit=sizeof(entries)-1;
	boot_params.env.gdtr.Base=(UINTN)page;
	
	return EFI_SUCCESS;
}