#include <stdio.h>
#include "mem.h"

/*页面位映射*/
static uint64 page_bitmap[BOOT_PTP_BITMAP]={0};

/*管理页面基址，连续内存*/
static uintn page_base = 0;

/*管理页面物理基址*/
static uintn page_pbase = 0;

/*
 * 初始化分页管理，以位映射形式记录空间申请。pm是paging manager的缩写。
 * 
 * @param params 启动参数结构指针。
 *
 * @return 无返回值。
 */
static void boot_init_pm()
{
	page_base=(uintn)_aligned_malloc(BOOT_PTP_SIZE,4096);
}

/*
 * 申请一页。
 *
 * @return 申请成功返回指针，申请失败返回NULL。
 */
static void* boot_page_alloc()
{
	uintn index = BOOT_PTP_BITMAP;
	while (index > 0)
	{
		index--;
		if (page_bitmap[index] == 0)
		{
			page_bitmap[index] = (uint64)1 << 63;
			void* result = (void*)(page_base + (index << 18) + (63 << 12));
			__builtin_memset_inline(result, 0, 4096);
			return result;
		}
		else
		{
			if (page_bitmap[index] == UINT64_MAX)
			{
				/*全部分配完。*/
				index--;
			}
			else
			{
				uintn bit = 64;
				while (bit > 0)
				{
					bit--;
					if (page_bitmap[index] & ((uint64)1 << bit))
					{
						continue;
					}
					page_bitmap[index] = page_bitmap[index] | ((uint64)1 << bit);
					void* result = (void*)(page_base + (index << 18) + (bit << 12));
					__builtin_memset_inline(result, 0, 4096);
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
	uintn index = 0;
	uintn pages = ((uintn)page - page_base) >> 12;
	while (pages >= 64)
	{
		index++;
		pages = pages - 64;
	}
	page_bitmap[index] = page_bitmap[index] & (~((uint64)1 << pages));
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
	if (page_pbase == 0)
	{
		return (uintn)vaddr;
	}
	else
	{
		return (((uintn)vaddr) - page_base) + page_pbase;
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
	if (page_pbase == 0)
	{
		return (uintn)paddr;
	}
	else
	{
		return (((uintn)paddr) - page_pbase) + page_base;
	}
}

/*系统页表，一般是PML5或PML4*/
static ia32_page_entry* kernel_page_table = 0;

int main(int argc, char** argv)
{
	boot_init_pm();
	void* a=boot_page_alloc();
	((uint64*)a)[0]=0xFFFFFFFF;
	boot_page_free(a);
    printf("Hello, from paging!\n");
}
