/*
 * AOS Loader图像头文件。
 * 2024-05-31创建。 
 */
#ifndef __AOS_GRAPHICS_H__
#define __AOS_GRAPHICS_H__

/*头文件*/

/*库头文件。*/
#include <Library/BaseLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/EdidActive.h>
#include <Protocol/EdidDiscovered.h>
#include <Protocol/GraphicsOutput.h>
#include <Guid/ConsoleOutDevice.h>

/*本地头文件*/
#include "aosdefine.h"

/*函数*/

/*初始化图像信息。过程中会设置图像模式。*/
EFI_STATUS
EFIAPI
aos_init_graphics_info(
    IN OUT AOS_BOOT_PARAMS* boot_params
);

#endif /*__AOS_GRAPHICS_H__*/