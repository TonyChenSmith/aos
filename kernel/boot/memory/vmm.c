/*
 * 虚拟内存管理函数。
 * @date 2024-11-14
 */
#include "include/vmm.h"

/*分配池链表头*/
static boot_mmp_node* head=NULL;

/*
 * 初始化虚拟内存管理列表。
 *
 * @param param	 启动参数结构。
 * @param bfuncs 基础模块函数表。
 *
 * @return 无返回值。
 */
extern void boot_vmm_init(boot_params* restrict param,const boot_base_functions* bfuncs)
{
	/*基址不应该为0*/
	uintn base=boot_pmm_alloc(true,LIMIT_4GB,BOOT_MMP_PAGE,KERNEL_DATA);
	
}