/** @file

  Null implementation of the fw_cfg library.

  Copyright (C) 2020, Rebecca Cran <rebecca@bsdio.com>
  Copyright (C) 2013, Red Hat, Inc.
  Copyright (c) 2011 - 2013, Intel Corporation. All rights reserved.<BR>
  Copyright (c) 2017, Advanced Micro Devices. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/DebugLib.h>
#include <Library/QemuFwCfgLib.h>

/**
  Returns a boolean indicating if the firmware configuration interface
  is available or not.

  This function may change fw_cfg state.

  @retval    TRUE   The interface is available
  @retval    FALSE  The interface is not available

**/
BOOLEAN
EFIAPI
QemuFwCfgIsAvailable (
  VOID
  )
{
  return FALSE;
}

/**
  Selects a firmware configuration item for reading.

  Following this call, any data read from this item will start from
  the beginning of the configuration item's data.

  @param[in] QemuFwCfgItem - Firmware Configuration item to read

**/
VOID
EFIAPI
QemuFwCfgSelectItem (
  IN FIRMWARE_CONFIG_ITEM  QemuFwCfgItem
  )
{
  ASSERT (FALSE);
}

/**
  Reads firmware configuration bytes into a buffer

  If called multiple times, then the data read will
  continue at the offset of the firmware configuration
  item where the previous read ended.

  @param[in] Size - Size in bytes to read
  @param[in] Buffer - Buffer to store data into

**/
VOID
EFIAPI
QemuFwCfgReadBytes (
  IN UINTN  Size,
  IN VOID   *Buffer  OPTIONAL
  )
{
  ASSERT (FALSE);
}

/**
  Writes firmware configuration bytes from a buffer

  If called multiple times, then the data written will
  continue at the offset of the firmware configuration
  item where the previous write ended.

  @param[in] Size - Size in bytes to write
  @param[in] Buffer - Buffer to read data from

**/
VOID
EFIAPI
QemuFwCfgWriteBytes (
  IN UINTN  Size,
  IN VOID   *Buffer
  )
{
  ASSERT (FALSE);
}

/**
  Skip bytes in the firmware configuration item.

  Increase the offset of the firmware configuration item without transferring
  bytes between the item and a caller-provided buffer. Subsequent read, write
  or skip operations will commence at the increased offset.

  @param[in] Size  Number of bytes to skip.
**/
VOID
EFIAPI
QemuFwCfgSkipBytes (
  IN UINTN  Size
  )
{
  ASSERT (FALSE);
}

/**
  Reads a UINT8 firmware configuration value

  @return    Value of Firmware Configuration item read

**/
UINT8
EFIAPI
QemuFwCfgRead8 (
  VOID
  )
{
  ASSERT (FALSE);
  return 0;
}

/**
  Reads a UINT16 firmware configuration value

  @return    Value of Firmware Configuration item read

**/
UINT16
EFIAPI
QemuFwCfgRead16 (
  VOID
  )
{
  ASSERT (FALSE);
  return 0;
}

/**
  Reads a UINT32 firmware configuration value

  @return    Value of Firmware Configuration item read

**/
UINT32
EFIAPI
QemuFwCfgRead32 (
  VOID
  )
{
  ASSERT (FALSE);
  return 0;
}

/**
  Reads a UINT64 firmware configuration value

  @return    Value of Firmware Configuration item read

**/
UINT64
EFIAPI
QemuFwCfgRead64 (
  VOID
  )
{
  ASSERT (FALSE);
  return 0;
}

/**
  Find the configuration item corresponding to the firmware configuration file.

  @param[in]  Name - Name of file to look up.
  @param[out] Item - Configuration item corresponding to the file, to be passed
                     to QemuFwCfgSelectItem ().
  @param[out] Size - Number of bytes in the file.

  @return    RETURN_SUCCESS       If file is found.
             RETURN_NOT_FOUND     If file is not found.
             RETURN_UNSUPPORTED   If firmware configuration is unavailable.

**/
RETURN_STATUS
EFIAPI
QemuFwCfgFindFile (
  IN   CONST CHAR8           *Name,
  OUT  FIRMWARE_CONFIG_ITEM  *Item,
  OUT  UINTN                 *Size
  )
{
  return RETURN_UNSUPPORTED;
}

RETURN_STATUS
EFIAPI
QemuFwCfgInitCache (
  IN OUT EFI_HOB_PLATFORM_INFO  *PlatformInfoHob
  )
{
  return RETURN_UNSUPPORTED;
}
