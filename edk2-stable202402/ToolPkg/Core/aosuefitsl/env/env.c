/*
 * AOS UEFI Bootloader环境管理。
 * 2024-05-29创建。
 */
#include "env/aosenv.h"

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
	},
	.spare={
		.Uint64=0
	}
};

/*局部量。*/

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

	boot_params.current_pointer=boot_params.pool;

	/*
	ist=SIZE_4GB-1;
	status=gBS->AllocatePages(AllocateMaxAddress,EfiLoaderData,boot_params.env.cpu_info.max_processors*AOS_IST_PAGES,&ist);
	if(EFI_ERROR(status))
	{
		return status;
	}
	boot_params.env.ist=ist;
	boot_params.env.ist_length=aos_pages_to_size(boot_params.env.cpu_info.max_processors*AOS_IST_PAGES);
	DEBUG((DEBUG_INFO,"IST1:B=0x%lX,S=0x%lX,Processors=%u\n",ist,boot_params.env.ist_length,boot_params.env.cpu_info.max_processors));

	stack=SIZE_4GB-1;
	status=gBS->AllocatePages(AllocateMaxAddress,EfiLoaderData,AOS_STACK_PAGES,&stack);
	if(EFI_ERROR(status))
	{
		return status;
	}
	boot_params.stack=(VOID*)(stack+aos_pages_to_size(AOS_STACK_PAGES));
	boot_params.stack_length=aos_pages_to_size(AOS_STACK_PAGES);

	DEBUG((DEBUG_INFO,"Kernel Stack:B=0x%lX\n",stack));
	*/

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
	address_pages=aos_size_to_pages((UINTN)boot_params.current_pointer);
	address=(VOID*)aos_pages_to_size(address_pages);
	if((aos_pages_to_size(address_pages)+address_size)>pool_end)
	{
		return NULL;
	}
	boot_params.current_pointer=(VOID*)(aos_pages_to_size(address_pages)+address_size);
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
	if(((UINTN)boot_params.current_pointer+size)>pool_end)
	{
		return NULL;
	}
	address=boot_params.current_pointer;
	boot_params.current_pointer=(VOID*)((UINTN)boot_params.current_pointer+size);
	return address;
}

/*
 * 初始化CPU信息。
 */
EFI_STATUS
EFIAPI
aos_init_cpu_info(VOID)
{
	UINT32 eax,ebx,ecx,edx;
	AsmCpuid(CPUID_STD_BASE_ID,&eax,&ebx,&ecx,&edx);

	if(eax>=CPUID_STD_SEFID)
	{
		UINT32 bax,bbx,bcx,bdx;
		AsmCpuid(CPUID_STD_FEAT,&bax,&bbx,&bcx,&bdx);
		boot_params.env.cpu_info.pge=bdx&BIT13?TRUE:FALSE;
		AsmCpuidEx(CPUID_STD_SEFID,CPUID_STD_SEFID_0,&bax,&bbx,&bcx,&bdx);
		boot_params.env.cpu_info.level5=bcx&BIT16?TRUE:FALSE;
		boot_params.env.cpu_info.erms=bbx&BIT9?TRUE:FALSE;
		AsmCpuid(CPUID_EXT_MAX,&bax,&bbx,&bcx,&bdx);
		if(bax>=CPUID_EXT_FEAT)
		{
			AsmCpuid(CPUID_EXT_FEAT,&bax,&bbx,&bcx,&bdx);
			boot_params.env.cpu_info.page1gb=bdx&BIT26?TRUE:FALSE;
		}
		else
		{
			/*正常不存在这种情况*/
			DEBUG((DEBUG_ERROR,"Error CPU.\n"));
			return EFI_DEVICE_ERROR;
		}
	}
	else
	{
		/*正常不存在这种情况*/
		DEBUG((DEBUG_ERROR,"Error CPU.\n"));
		return EFI_DEVICE_ERROR;
	}

	if(ebx==CPU_INTEL_ID_EBX&&ecx==CPU_INTEL_ID_ECX&&edx==CPU_INTEL_ID_EDX)
	{
		/*Intel*/
		if(eax>=CPUID_STD_ETE)
		{
			UINT32 stdindex=eax>=CPUID_STD_V2ETE?CPUID_STD_V2ETE:CPUID_STD_ETE;
			UINT32 index,result=0;
			for(index=0;;index++)
			{
				AsmCpuidEx(stdindex,index,&eax,&ebx,&ecx,&edx);
				if(BitFieldRead32(ecx,8,15)==CPUID_ETE_LEVEL_INVALID)
				{
					DEBUG_CODE_BEGIN();
					AsmCpuid(CPUID_STD_BASE_CPU,&eax,&ebx,&ecx,&edx);
					ASSERT(result>=BitFieldRead32(ebx,16,23));
					DEBUG_CODE_END();
					boot_params.env.cpu_info.max_processors=result;
					break;
				}
				else
				{
					result=ebx;/*恒定数值越来越大。*/
				}
			}
		}
		else
		{
			/*xAPIC时期*/
			AsmCpuid(CPUID_STD_BASE_CPU,&eax,&ebx,&ecx,&edx);
			if(edx&BIT28)
			{
				boot_params.env.cpu_info.max_processors=BitFieldRead32(ebx,16,23);
			}
			else
			{
				boot_params.env.cpu_info.max_processors=1;
			}
			return EFI_SUCCESS;
		}
	}
	else if(ebx==CPU_AMD_ID_EBX&&ecx==CPU_AMD_ID_ECX&&edx==CPU_AMD_ID_EDX)
	{
		/*AMD*/
		AsmCpuid(CPUID_EXT_MAX,&eax,&ebx,&ecx,&edx);
		if(eax>=CPUID_EXT_PCP)
		{
			AsmCpuid(CPUID_EXT_PCP,&eax,&ebx,&ecx,&edx);
			UINT32 result=BitFieldRead32(ecx,12,15);
			if(result==0)
			{
				/*遗留方法*/
				boot_params.env.cpu_info.max_processors=BitFieldRead32(ecx,0,7)+1;
			}
			else
			{
				/*扩展方法*/
				boot_params.env.cpu_info.max_processors=1<<result;
			}
		}
		else
		{
			/*正常不存在这种情况*/
			DEBUG((DEBUG_ERROR,"Error AMD CPU.\n"));
			return EFI_DEVICE_ERROR;
		}
	}
	else
	{
		/*未知处理器*/
		DEBUG((DEBUG_ERROR,"Unknown CPU.EBX=%X,ECX=%X,EDX=%X\n",ebx,ecx,edx));
		return EFI_UNSUPPORTED;
	}

	if(boot_params.env.cpu_info.max_processors>255)
	{
		/*当前未打算开启x2APIC*/
		return EFI_UNSUPPORTED;
	}
	return EFI_SUCCESS;
}

/*获取内存图*/
EFI_STATUS
EFIAPI
aos_set_memmap(
	OUT UINTN* map_key
)
{
	EFI_STATUS status;
	UINTN entry_size;
	UINT32 version;
	UINTN key;
	EFI_MEMORY_DESCRIPTOR* memmap=NULL;
	UINTN map_size=0;
	
	status=gBS->GetMemoryMap(&map_size,memmap,&key,&entry_size,&version);
	if(status==EFI_BUFFER_TOO_SMALL)
	{
		ASSERT(map_size>0);
		memmap=(EFI_MEMORY_DESCRIPTOR*)aos_allocate_pool(map_size);
		ASSERT(memmap!=NULL);
		status=gBS->GetMemoryMap(&map_size,memmap,&key,&entry_size,&version);
		ASSERT(status==EFI_SUCCESS);
		ASSERT(version==EFI_MEMORY_DESCRIPTOR_VERSION);
		*map_key=key;
		boot_params.env.memmap_length=map_size;
		boot_params.env.memmap=memmap;
		boot_params.env.entry_size=entry_size;

		DEBUG((DEBUG_INFO,"===AOS Memory Map===\nmemmap=0x%lX,map_size=%lu.\n",memmap,map_size));

		EFI_MEMORY_DESCRIPTOR* dsc=memmap;
		UINTN offset=entry_size;
		UINTN end=map_size+(UINTN)dsc;
		while((UINTN)dsc<end)
		{
			DEBUG((DEBUG_INFO,"0x%lX-0x%1X,page=%lu,type=%lu.\n",dsc->PhysicalStart,dsc->PhysicalStart+(dsc->NumberOfPages<<12)-1,dsc->NumberOfPages,dsc->Type));
			dsc=(EFI_MEMORY_DESCRIPTOR*)((UINTN)dsc+offset);
		}

		return EFI_SUCCESS;
	}
	return status;
}

/*
 * 设置系统表。
 */
EFI_STATUS
EFIAPI
aos_set_system_table(VOID)
{
	/*至少支持ACPI 5.0*/
	boot_params.acpi=NULL;
	boot_params.smbios=NULL;
	boot_params.smbios3=NULL;
	for(UINTN index=0;index<gST->NumberOfTableEntries;index++)
	{
		if(CompareGuid(&gST->ConfigurationTable[index].VendorGuid,&gEfiAcpiTableGuid))
		{
			boot_params.acpi=gST->ConfigurationTable[index].VendorTable;
			continue;
		}
		else if(CompareGuid(&gST->ConfigurationTable[index].VendorGuid,&gEfiSmbiosTableGuid))
		{
			boot_params.smbios=gST->ConfigurationTable[index].VendorTable;
			continue;
		}
		else if(CompareGuid(&gST->ConfigurationTable[index].VendorGuid,&gEfiSmbios3TableGuid))
		{
			boot_params.smbios3=gST->ConfigurationTable[index].VendorTable;
			continue;
		}
	}
	ASSERT(boot_params.acpi!=NULL&&(boot_params.smbios!=NULL||boot_params.smbios3!=NULL));
	DEBUG((DEBUG_INFO,"===AOS Table===\nacpi=0x%lX.\nsmbios=0x%lX.\nsmbios3=0x%lX.\n",boot_params.acpi,boot_params.smbios,boot_params.smbios3));
	boot_params.runtime=gST->RuntimeServices;
	return EFI_SUCCESS;
}