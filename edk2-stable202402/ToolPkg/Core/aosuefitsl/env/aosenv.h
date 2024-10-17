/*
 * AOS Loader环境头文件。
 * 2024-05-29创建。 
 */
#ifndef __AOS_ENV_H__
#define __AOS_ENV_H__

#include "aosdefine.h"

/*初始化内存*/
EFI_STATUS
EFIAPI
aos_init_memory(VOID);

/*申请内存页*/
VOID*
EFIAPI
aos_allocate_pages(
	IN UINTN pages
);

/*申请内存池*/
VOID*
EFIAPI
aos_allocate_pool(
	IN UINTN size
);

/*创建配置*/
EFI_STATUS
EFIAPI
aos_create_config(VOID);

#endif /*__AOS_ENV_H__*/