/*
 * AOS Loader内存头文件。
 * 2024-05-29创建。 
 */
#ifndef __AOS_MEMORY_H__
#define __AOS_MEMORY_H__

/*头文件*/

/*本地头文件*/
#include "aosdefine.h"

/*全局量*/

/*物理内存表。映射完全后会从小到大排序。*/
extern AOS_PHYSICAL_MEMORY_DESCRIPTOR* aos_memory_map; 

/*物理内存表的数组长度。*/
extern UINTN aos_memory_map_length;

/*函数*/

/*创建内存映射。应在且仅在结束启动服务前调用。*/
EFI_STATUS
EFIAPI
aos_create_memory_map(VOID);

#endif /*__AOS_MEMORY_H__*/