/*
 * AOS Loader文件操作头文件。
 * 2024-10-15创建。 
 */
#ifndef __AOS_FILE_H__
#define __AOS_FILE_H__

#include "aosdefine.h"
#include <Protocol/LoadedImage.h>
#include <Library/FileHandleLib.h>

/*
 * 加载引导程序到内存上。
 */
EFI_STATUS
EFIAPI
aos_load_bootstrap(VOID);

#endif /*__AOS_FILE_H__*/