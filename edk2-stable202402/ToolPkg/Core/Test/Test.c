/*
 * 测试用应用程序。
 *
 * Copyright (c) 2024-2025 Tony Chen Smith. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/DevicePath.h>
#include <Library/DevicePathLib.h>

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  UINTN NumberOfHandles;
  EFI_DEVICE_PATH_PROTOCOL** Buffer;
  EFI_STATUS status;
  UINTN Index;

  NumberOfHandles=0;
  Buffer=NULL;
  status=EfiLocateProtocolBuffer(&gEfiDevicePathProtocolGuid,&NumberOfHandles,(VOID***)&Buffer);
  if(EFI_ERROR(status))
  {
    return status;
  }

  for(Index=0;Index<NumberOfHandles;Index++)
  {
    Print(ConvertDevicePathToText(Buffer[Index],TRUE,TRUE));
    Print(L"\r\n");
  }
  return EFI_SUCCESS;
}