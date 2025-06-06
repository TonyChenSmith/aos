/** @file
  RedfishHostInterfaceDxe builds up SMBIOS Type 42h host interface
  record for Redfish service host interface using EFI MBIOS Protocol.
  RedfishHostInterfacePlatformLib is the platform-level library which
  provides the content of Redfish host interface type 42h record.

  Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
  (C) Copyright 2020 Hewlett Packard Enterprise Development LP<BR>
  Copyright (C) 2022 Advanced Micro Devices, Inc. All rights reserved.<BR>
  Copyright (c) 2023 - 2025, NVIDIA CORPORATION & AFFILIATES. All rights reserved.
  Copyright (c) 2023, Ampere Computing LLC. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#include <Base.h>
#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Library/RedfishHostInterfaceLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

static EFI_EVENT  mPlatformHostInterfaceReadylEvent        = NULL;
static VOID       *mPlatformHostInterfaceReadyRegistration = NULL;

/**
  Create SMBIOS type 42 record for Redfish host interface.

  @retval EFI_SUCCESS    SMBIOS type 42 record is created.
  @retval Others         Fail to create SMBIOS 42 record.

**/
EFI_STATUS
RedfishCreateSmbiosTable42 (
  VOID
  )
{
  REDFISH_INTERFACE_DATA             *DeviceDescriptor;
  UINT8                              DeviceDataLength;
  UINT8                              DeviceType;
  EFI_STATUS                         Status;
  MC_HOST_INTERFACE_PROTOCOL_RECORD  *ProtocolRecord;
  VOID                               *ProtocolRecords;
  VOID                               *NewProtocolRecords;
  UINT8                              ProtocolCount;
  UINT8                              CurrentProtocolsDataLength;
  UINT8                              NewProtocolsDataLength;
  UINT8                              ProtocolDataSize;
  SMBIOS_TABLE_TYPE42                *Type42Record;
  EFI_SMBIOS_PROTOCOL                *Smbios;
  EFI_SMBIOS_HANDLE                  MemArrayMappedAddrSmbiosHandle;
  EFI_HANDLE                         Handle;
  CHAR8                              *SerialNumber;
  UINTN                              SerialNumStrLen;

  Handle          = NULL;
  SerialNumStrLen = 0;
  SerialNumber    = NULL;
  //
  // Get platform Redfish host interface device type descriptor data.
  //
  Status = RedfishPlatformHostInterfaceDeviceDescriptor (&DeviceType, &DeviceDescriptor);
  if (EFI_ERROR (Status)) {
    if (Status == EFI_NOT_FOUND) {
      DEBUG ((DEBUG_ERROR, "%a: No Redfish host interface descriptor is provided on this platform.\n", __func__));
      return EFI_NOT_FOUND;
    }

    DEBUG ((DEBUG_ERROR, "%a: Fail to get device descriptor, %r.", __func__, Status));
    return Status;
  }

  if ((DeviceType != REDFISH_HOST_INTERFACE_DEVICE_TYPE_USB_V2) &&
      (DeviceType != REDFISH_HOST_INTERFACE_DEVICE_TYPE_PCI_PCIE_V2)
      )
  {
    DEBUG ((DEBUG_ERROR, "%a: Only support either protocol type 04h or 05h as Redfish host interface.", __func__));
    return EFI_UNSUPPORTED;
  }

  if (DeviceType == REDFISH_HOST_INTERFACE_DEVICE_TYPE_PCI_PCIE_V2) {
    DeviceDataLength = DeviceDescriptor->DeviceDescriptor.PciPcieDeviceV2.Length;
  } else {
    DeviceDataLength = DeviceDescriptor->DeviceDescriptor.UsbDeviceV2.Length;
    Status           = RedfishPlatformHostInterfaceSerialNumber (&SerialNumber);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "%a: Fail to get redfish host interface serial number, %r.", __func__, Status));
      DeviceDescriptor->DeviceDescriptor.UsbDeviceV2.SerialNumberStr = 0;
    } else {
      if (SerialNumber != NULL) {
        SerialNumStrLen                                                = (UINTN)AsciiStrLen (SerialNumber);
        DeviceDescriptor->DeviceDescriptor.UsbDeviceV2.SerialNumberStr = 1;
      }
    }
  }

  //
  // Loop to get platform Redfish host interface protocol type data.
  //
  ProtocolRecord             = NULL;
  ProtocolRecords            = NULL;
  NewProtocolRecords         = NULL;
  Type42Record               = NULL;
  ProtocolCount              = 0;
  CurrentProtocolsDataLength = 0;
  NewProtocolsDataLength     = 0;
  while (TRUE) {
    Status = RedfishPlatformHostInterfaceProtocolData (&ProtocolRecord, ProtocolCount);
    if (Status == EFI_NOT_FOUND) {
      break;
    }

    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "%a: Fail to get Redfish host interafce protocol type data.", __func__));
      if (ProtocolRecords != NULL) {
        FreePool (ProtocolRecords);
      }

      if (ProtocolRecord != NULL) {
        FreePool (ProtocolRecord);
      }

      return Status;
    }

    ProtocolDataSize        = sizeof (MC_HOST_INTERFACE_PROTOCOL_RECORD) - sizeof (ProtocolRecord->ProtocolTypeData) + ProtocolRecord->ProtocolTypeDataLen;
    NewProtocolsDataLength += ProtocolDataSize;
    if (ProtocolRecords == NULL) {
      ProtocolRecords = AllocateZeroPool (NewProtocolsDataLength);
      if (ProtocolRecords == NULL) {
        FreePool (ProtocolRecord);
        return EFI_OUT_OF_RESOURCES;
      }

      CopyMem ((VOID *)ProtocolRecords, (VOID *)ProtocolRecord, ProtocolDataSize);
      NewProtocolRecords = ProtocolRecords;
    } else {
      NewProtocolRecords = ReallocatePool (CurrentProtocolsDataLength, NewProtocolsDataLength, (VOID *)ProtocolRecords);
      if (NewProtocolRecords == NULL) {
        DEBUG ((DEBUG_ERROR, "%a: Fail to allocate memory for Redfish host interface protocol data.", __func__));
        FreePool (ProtocolRecords);
        FreePool (ProtocolRecord);
        return EFI_OUT_OF_RESOURCES;
      }

      CopyMem (
        (VOID *)((UINT8 *)NewProtocolRecords + CurrentProtocolsDataLength),
        (VOID *)ProtocolRecord,
        ProtocolDataSize
        );
    }

    FreePool (ProtocolRecord);
    CurrentProtocolsDataLength = NewProtocolsDataLength;
    ProtocolCount++;
  }

  if (ProtocolCount == 0) {
    goto ON_EXIT;
  }

  //
  // Construct SMBIOS Type 42h for Redfish host inteface.
  //
  // SMBIOS type 42 Record for Redfish Interface
  // 00h Type BYTE 42 Management Controller Host Interface structure indicator
  // 01h Length BYTE Varies Length of the structure, a minimum of 09h
  // 02h Handle WORD Varies
  // 04h Interface Type BYTE Varies Management Controller Interface Type.
  // 05h Interface Specific Data Length (n)
  // 06h Interface Specific data
  // 06h+n number of protocols defined for the host interface (typically 1)
  // 07h+n Include a Protocol Record for each protocol supported.
  //
  Type42Record = (SMBIOS_TABLE_TYPE42 *)AllocateZeroPool (
                                          sizeof (SMBIOS_TABLE_TYPE42) - 4
                                          + DeviceDataLength
                                          + 1  /// For Protocol Record Count
                                          + CurrentProtocolsDataLength
                                          + SerialNumStrLen
                                          + 2  /// Double NULL terminator/
                                          );
  if (Type42Record == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto ON_EXIT;
  }

  Type42Record->Hdr.Type   = EFI_SMBIOS_TYPE_MANAGEMENT_CONTROLLER_HOST_INTERFACE;
  Type42Record->Hdr.Length = sizeof (SMBIOS_TABLE_TYPE42) - 4
                             + DeviceDataLength
                             + 1
                             + CurrentProtocolsDataLength;
  Type42Record->Hdr.Handle    = 0;
  Type42Record->InterfaceType = MCHostInterfaceTypeNetworkHostInterface; // Network Host Interface

  //
  // Fill in InterfaceTypeSpecificDataLength field
  //
  Type42Record->InterfaceTypeSpecificDataLength = DeviceDataLength;

  //
  // Fill in InterfaceTypeSpecificData field
  //
  CopyMem (Type42Record->InterfaceTypeSpecificData, DeviceDescriptor, DeviceDataLength);
  FreePool (DeviceDescriptor);
  DeviceDescriptor = NULL;

  //
  // Fill in InterfaceTypeSpecificData Protocol Count field
  //
  *(Type42Record->InterfaceTypeSpecificData + DeviceDataLength) = ProtocolCount;

  //
  // Fill in Redfish Protocol Data
  //
  CopyMem (
    Type42Record->InterfaceTypeSpecificData + DeviceDataLength + 1,
    NewProtocolRecords,
    CurrentProtocolsDataLength
    );

  //
  // Fill in Serial Number string at the end of SMBIOS table 42
  //
  if (SerialNumStrLen) {
    CopyMem (Type42Record->InterfaceTypeSpecificData + DeviceDataLength + 1 + CurrentProtocolsDataLength, SerialNumber, SerialNumStrLen);
  }

  //
  // 5. Add Redfish interface data record to SMBIOS table 42
  //
  Status = gBS->LocateProtocol (&gEfiSmbiosProtocolGuid, NULL, (VOID **)&Smbios);
  if (EFI_ERROR (Status)) {
    goto ON_EXIT;
  }

  MemArrayMappedAddrSmbiosHandle = SMBIOS_HANDLE_PI_RESERVED;
  Status                         = Smbios->Add (
                                             Smbios,
                                             NULL,
                                             &MemArrayMappedAddrSmbiosHandle,
                                             (EFI_SMBIOS_TABLE_HEADER *)Type42Record
                                             );
  DEBUG ((DEBUG_MANAGEABILITY, "RedfishPlatformDxe: Smbios->Add() - %r\n", Status));
  if (EFI_ERROR (Status)) {
    goto ON_EXIT;
  }

  //
  // Install Redfish Host Interface ready protocol.
  //
  Status = gBS->InstallProtocolInterface (
                  &Handle,
                  &gEdkIIRedfishHostInterfaceReadyProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  (VOID *)NULL
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to install gEdkIIRedfishHostInterfaceReadyProtocolGuid.\n"));
    DEBUG ((DEBUG_ERROR, "PlatformConfigHandler driver may not be triggered to acquire Redfish service.\n"));
  }

  // Set Status to EFI_SUCCESS that indicates SMBIOS 42 record was installed
  // on the platform sucessfully.
  Status = EFI_SUCCESS;

ON_EXIT:
  if (DeviceDescriptor != NULL) {
    FreePool (DeviceDescriptor);
  }

  if (NewProtocolRecords != NULL) {
    FreePool (NewProtocolRecords);
  }

  if (Type42Record != NULL) {
    FreePool (Type42Record);
  }

  if (SerialNumber != NULL) {
    FreePool (SerialNumber);
  }

  return Status;
}

/**
  Notification event of platform Redfish Host Interface readiness.

  @param[in]  Event     Event whose notification function is being invoked.
  @param[in]  Context   The pointer to the notification function's context,
                        which is implementation-dependent.

**/
VOID
EFIAPI
PlatformHostInterfaceInformationReady (
  IN  EFI_EVENT  Event,
  IN  VOID       *Context
  )
{
  DEBUG ((DEBUG_MANAGEABILITY, "%a: Platform Redfish Host Interface informtion is ready\n", __func__));

  RedfishCreateSmbiosTable42 ();

  //
  // Close event so we don't create multiple type 42 records
  //
  gBS->CloseEvent (Event);
  mPlatformHostInterfaceReadylEvent = NULL;

  return;
}

/**
  Main entry for this driver.

  @param ImageHandle     Image handle this driver.
  @param SystemTable     Pointer to SystemTable.

  @retval EFI_SUCCESS    This function always complete successfully.

**/
EFI_STATUS
EFIAPI
RedfishHostInterfaceDxeEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;
  EFI_GUID    *ReadyGuid;

  DEBUG ((DEBUG_MANAGEABILITY, "%a: Entry\n.", __func__));

  //
  // Check if the Redfish Host Interface depends on
  // the specific protocol installation.
  //
  Status = RedfishPlatformHostInterfaceNotification (&ReadyGuid);
  if (Status == EFI_SUCCESS) {
    DEBUG ((DEBUG_MANAGEABILITY, "    Create protocol install notification to know the installation of platform Redfish host interface readiness\n"));
    DEBUG ((DEBUG_MANAGEABILITY, "    Protocol GUID: %g\n", ReadyGuid));
    //
    // Register event for ReadyGuid protocol installed by
    // platform Redfish host interface library.
    //
    Status = gBS->CreateEvent (
                    EVT_NOTIFY_SIGNAL,
                    TPL_CALLBACK,
                    PlatformHostInterfaceInformationReady,
                    NULL,
                    &mPlatformHostInterfaceReadylEvent
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "    Fail to create event for the installation of platform Redfish host interface readiness.\n"));
      return Status;
    }

    Status = gBS->RegisterProtocolNotify (
                    ReadyGuid,
                    mPlatformHostInterfaceReadylEvent,
                    &mPlatformHostInterfaceReadyRegistration
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "    Fail to register event for the installation of platform Redfish host interface readiness.\n"));
      return Status;
    }

    return EFI_SUCCESS;
  }

  if ((Status == EFI_UNSUPPORTED) || (Status == EFI_ALREADY_STARTED)) {
    Status = RedfishCreateSmbiosTable42 ();
  }

  // Return other erros.
  return Status;
}
