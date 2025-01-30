/*
 * AOS Loader总头文件。
 * 2024-05-28创建。 
 * 
 * Copyright (c) 2024-2025 Tony Chen Smith. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_LOADER_H__
#define __AOS_LOADER_H__

/*头文件*/

/*标准头文件*/
#include <Uefi.h>

/*库头文件*/
#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>

/*本地头文件*/
#include "aosdefine.h"
#include "graphics/aosgraphics.h"
#include "env/aosenv.h"
#include "bus/aosbus.h"
#include "file/aosfile.h"

/*函数*/

/*入口函数。*/
EFI_STATUS
EFIAPI
aos_uefi_entry(
	IN EFI_HANDLE        image_handle,
	IN EFI_SYSTEM_TABLE* system_table
);



#endif /*__AOS_LOADER_H__*/