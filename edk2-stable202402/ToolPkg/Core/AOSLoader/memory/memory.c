/*
 * AOS UEFI Bootloader内存管理。
 * 2024-05-29创建。
 */
#include "aosmemory.h"

/*局部量。*/

/*UEFI内存表。*/
//static EFI_MEMORY_DESCRIPTOR* efi_memory_map;

/*内存表键。这个与同步锁有关系。*/
//static UINTN efi_memory_map_key;

/*内存表大小。*/
//static UINTN efi_memory_map_size;

/*内存表项大小。*/
//static UINTN efi_memory_map_entry_size;

/*内存表项版本。*/
//static UINT32 efi_memory_map_entry_version;

/*函数*/

/*
 * 创建内存映射。应在且仅在结束启动服务前调用。
 * @retval EFI_SUCCESS 执行成功。后续应紧跟结束启动服务。
 */
EFI_STATUS
EFIAPI
aos_create_memory_map(VOID)
{
    return EFI_SUCCESS;
}