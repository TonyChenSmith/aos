/*
 * AOS UEFI Bootloader总线管理。
 * 2024-09-22创建。
 * 
 * Copyright (c) 2024-2025 Tony Chen Smith. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef __AOS_BUS_H__
#define __AOS_BUS_H__

/*头文件*/

/*库头文件*/
#include <Protocol/PciIo.h>
#include <Protocol/PciRootBridgeIo.h>

/*本地头文件*/
#include "aosdefine.h"
#include "env/aosenv.h"

/*函数*/

/*扫描PCI总线。将把扫描信息进行统计和记录*/
EFI_STATUS
EFIAPI
aos_scan_pci_device(
	VOID
);

#endif /*__AOS_BUS_H__*/