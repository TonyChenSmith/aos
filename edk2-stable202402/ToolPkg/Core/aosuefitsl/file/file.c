/*
 * AOS Loader文件操作。
 * 2024-10-15创建。 
 */
#include "file/aosfile.h"

/*需要在引导器环节加载的内核文件*/
CONST CHAR16* loaded_files[]={
	aos_esp_file("aoskernel.ae")
};

/*文件数目，作为常量减少计算次数*/
CONST UINT8 list_size=1;

/*根目录*/
static EFI_FILE_HANDLE esp_root=NULL;

/*
 * 获取当前启动区根目录。这个同时会打开卷，运行前应假设esp_root为空。
 * 不为空的esp_root不需要获得，定位应该是唯一的。
 */
static
EFI_STATUS
EFIAPI
aos_get_esp_root(VOID)
{
	EFI_STATUS status;
	EFI_LOADED_IMAGE_PROTOCOL* image;
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* simple;

	if(esp_root!=NULL)
	{
		return EFI_SUCCESS;
	}

	/*获取加载程序映像协议，其应该有简单文件系统协议*/
	status=gBS->HandleProtocol(gImageHandle,&gEfiLoadedImageProtocolGuid,(VOID**)&image);
	if(EFI_ERROR(status))
	{
		return status;
	}
	status=gBS->HandleProtocol(image->DeviceHandle,&gEfiLoadedImageProtocolGuid,(VOID**)&simple);
	if(EFI_ERROR(status))
	{
		return status;
	}
	return simple->OpenVolume(simple,&esp_root);
}

/*
 * 加载引导程序到内存上。
 */
EFI_STATUS
EFIAPI
aos_load_bootstrap(VOID)
{
	EFI_STATUS status;
	status=aos_get_esp_root();
	if(EFI_ERROR(status))
	{
		return status;
	}
	return EFI_SUCCESS;
}