/*
 * 内存模块分页管理函数。
 * @date 2024-11-10
 */
#include "mem.h"

/*页面位映射*/
static uint64 page_bitmap[BOOT_PTP_BITMAP]={0};

/*管理页面基址，连续内存*/
static uintn page_base=0;

/*管理页面物理基址*/
static uintn page_pbase=0;

/*
 * 初始化分页管理，以位映射形式记录空间申请。pm是paging manager的缩写。
 * 
 * @param params 启动参数结构指针。
 *
 * @return 无返回值。
 */
static void boot_init_pm(boot_params* restrict params)
{
	page_base=((uintn)params->env.max_free)+(params->env.max_pages<<12)-BOOT_PTP_SIZE;
	if(params->env.max_pages<BOOT_PTP_PAGE)
	{
		/*按照搜索规则，原则上这一段是无意义的，但为了未来代码参考依旧保留*/
		uintn reserved=BOOT_PTP_PAGE-params->env.max_pages;
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
		params->env.max_pages=0;
	}
	else
	{
		params->env.max_pages=params->env.max_pages-BOOT_PTP_PAGE;
	}
}

/*
 * 申请一页。
 *
 * @return 申请成功返回指针，申请失败返回NULL。
 */
static void* boot_page_alloc()
{
	uintn index=BOOT_PTP_BITMAP;
	while(index>0)
	{
		index--;
		if(page_bitmap[index]==0)
		{
			page_bitmap[index]=(uint64)1<<63;
			void* result=(void*)(page_base+(index<<18)+(63<<12));
			__builtin_memset_inline(result,0,4096);
			return result;
		}
		else
		{
			if(page_bitmap[index]==UINT64_MAX)
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
					void* result=(void*)(page_base+(index<<18)+(bit<<12));
					__builtin_memset_inline(result,0,4096);
					return result;
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

/*
 * 获取地址对应的物理地址。
 * 
 * @param vaddr 虚拟地址。
 *
 * @return 对应物理地址，没有进入虚拟地址空间时仅强制转换。
 */
static uintn boot_get_paddr(const void* vaddr)
{
	if(page_pbase==0)
	{
		return (uintn)vaddr;
	}
	else
	{
		return (((uintn)vaddr)-page_base)+page_pbase;
	}
}

/*
 * 获取地址对应的物理地址。
 * 
 * @param paddr 物理地址。
 *
 * @return 对应虚拟地址，没有进入虚拟地址空间时仅强制转换。
 */
static uintn boot_get_vaddr(const uintn paddr)
{
	if(page_pbase==0)
	{
		return (uintn)paddr;
	}
	else
	{
		return (((uintn)paddr)-page_pbase)+page_base;
	}
}

/*系统页表，一般是PML5或PML4*/
static ia32_page_entry* kernel_page_table=0;

/*系统是否开启五级页表*/
static bool is5level=false;

/*1GB大页支持*/
static bool support1gb=false;

/*
 * 进入虚拟地址空间。
 *
 * @param ptp 页面池虚拟基址。
 *
 * @return 无返回值。
 */
static void boot_into_virtual(const void* ptp)
{
	page_pbase=page_base;
	page_base=(uintn)ptp;
	kernel_page_table=(ia32_page_entry*)((uintn)kernel_page_table-page_pbase+(uintn)ptp);
}

/*
 * 映射页表。
 * 
 * @param pt	页表。
 * @param paddr	物理地址。
 * @param vaddr	虚拟地址。
 * @param pages	页数。在此指代4kB页的页数。
 * @param attr	页面属性。
 *
 * @return 成功分配时返回页面数目，失败返回0。
 */
static uintn boot_map_pt(ia32_page_entry* restrict pt,const uintn paddr,const uintn vaddr,const uintn pages,const uint32 attr)
{

}

/*
 * 映射页目录。
 * 
 * @param pd	页目录。
 * @param paddr	物理地址。
 * @param vaddr	虚拟地址。
 * @param pages	页数。在此指代4kB页的页数。
 * @param attr	页面属性。
 *
 * @return 成功分配时返回页面数目，失败返回0。
 */
static uintn boot_map_pd(ia32_page_entry* restrict pd,const uintn paddr,const uintn vaddr,const uintn pages,const uint32 attr)
{

}

/*
 * 映射页目录指针表。
 * 
 * @param pdpt	页目录指针表。
 * @param paddr	物理地址。
 * @param vaddr	虚拟地址。
 * @param pages	页数。在此指代4kB页的页数。
 * @param attr	页面属性。
 *
 * @return 成功分配时返回页面数目，失败返回0。
 */
static uintn boot_map_pdpt(ia32_page_entry* restrict pdpt,const uintn paddr,const uintn vaddr,const uintn pages,const uint32 attr)
{
	uintn cpages=pages;
	uintn cpaddr=paddr;
	uintn cvaddr=vaddr;
	while(cpages>0)
	{
		uintn index=(cvaddr>>30)&0x1FF;
		ia32_page_entry* pd;
		if(pdpt[index].pml_entry.p)
		{
		}
		else
		{
			pdpt=(ia32_page_entry*)boot_page_alloc();
			if(pdpt==NULL)
			{
				return 0;
			}
		}
		uintn count=boot_map_pdpt(pdpt,cpaddr,cvaddr,cpages,attr);
		if(count==0)
		{
			return 0;
		}
		if(index==511)
		{
			return pages-cpages;
		}
		else
		{
			cpaddr=cpaddr+(count<<12);
			cvaddr=cvaddr+(count<<12);
			cpages=cpages-count;
		}
	}
	return pages;
}

/*
 * 映射四级页表。
 * 
 * @param pml4	四级页表。
 * @param paddr	物理地址。
 * @param vaddr	虚拟地址。
 * @param pages	页数。在此指代4kB页的页数。
 * @param attr	页面属性。
 *
 * @return 成功分配时返回页面数目，失败返回0。
 */
static uintn boot_map_pml4(ia32_page_entry* restrict pml4,const uintn paddr,const uintn vaddr,const uintn pages,const uint32 attr)
{
	uintn cpages=pages;
	uintn cpaddr=paddr;
	uintn cvaddr=vaddr;
	while(cpages>0)
	{
		uintn index=(cvaddr>>39)&0x1FF;
		ia32_page_entry* pdpt;
		if(pml4[index].pml_entry.p)
		{
			pdpt=(ia32_page_entry*)boot_get_vaddr((uintn)pml4[index].pml_entry.addr_up<<32|pml4[index].pml_entry.addr_down<<12);
		}
		else
		{
			pdpt=(ia32_page_entry*)boot_page_alloc();
			if(pdpt==NULL)
			{
				return 0;
			}
			pml4[index].pml_entry.p=1;
			pml4[index].pml_entry.rw=1;
			pml4[index].pml_entry.us=1;
			pml4[index].pml_entry.pwt=0;
			pml4[index].pml_entry.pcd=0;
			uintn ppdpt=boot_get_paddr(pdpt)>>12;
			pml4[index].pml_entry.addr_down=ppdpt&0xFFFFF;
			pml4[index].pml_entry.addr_up=(ppdpt>>20)&0xFFFFF;
		}
		uintn count=boot_map_pdpt(pdpt,cpaddr,cvaddr,cpages,attr);
		if(count==0)
		{
			return 0;
		}
		if(index==511)
		{
			return pages-cpages;
		}
		else
		{
			cpaddr=cpaddr+(count<<12);
			cvaddr=cvaddr+(count<<12);
			cpages=cpages-count;
		}
	}
	return pages;
}

/*
 * 映射五级页表。
 * 
 * @param paddr	物理地址。
 * @param vaddr	虚拟地址。
 * @param pages	页数。在此指代4kB页的页数。
 * @param attr	页面属性。
 *
 * @return 成功分配时返回页面数目，失败返回0。
 */
static uintn boot_map_pml5(const uintn paddr,const uintn vaddr,const uintn pages,const uint32 attr)
{
	
}

/*
 * 将物理地址映射到页表对应虚拟地址。我们在此假设虚拟地址是规范的，页数不应该超过寻址范围，且不与其他映射重叠。
 * 
 * @param paddr	物理地址。
 * @param vaddr	虚拟地址。
 * @param pages	页数。在此指代4kB页的页数。
 * @param attr	页面属性。
 *
 * @return 无返回值。
 */
static void boot_map_address(const uintn paddr,const uintn vaddr,const uintn pages,const uint32 attr)
{

}

/*
 * 初始化内核页表。
 *
 * @param params	 启动参数结构指针。
 * @param base_funcs 基础模块函数表指针。
 *
 * @return 无返回值。
 */
extern void boot_init_kpt(const boot_params* restrict params,const boot_base_functions* restrict base_funcs)
{
	kernel_page_table=(ia32_page_entry*)boot_page_alloc();
	if(params->env.cpu_info.level5)
	{
		is5level=_base_rdcr4()&0x1000;
	}
	else
	{
		is5level=false;
	}
	support1gb=params->env.cpu_info.page1gb;
}