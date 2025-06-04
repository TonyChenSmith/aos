#
# Description of the module "aos.uefi".
#
# Copyright (c) 2025 Tony Chen Smith. All rights reserved.
#
# SPDX-License-Identifier: MIT
#
[Defines]
    PLATFORM_NAME                  = AOS
    PLATFORM_GUID                  = DE82DA92-FC7F-3E64-A5BC-619E1E42E1A4 # UUID.nameUUIDFromBytes("aos.uefi.description".getBytes("UTF-8"))
    PLATFORM_VERSION               = 0.0.2                                # Major.Minor.Patch Before the official launch, do not increment the major version number.
    DSC_SPECIFICATION              = 0x00010005
    OUTPUT_DIRECTORY               = Build/AOS
    SUPPORTED_ARCHITECTURES        = X64
    BUILD_TARGETS                  = DEBUG|RELEASE
    RFC_LANGUAGES                  = "en-us"
    SKUID_IDENTIFIER               = DEFAULT

[BuildOptions]

[LibraryClasses]
    # Entry Point
    UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
    # Basic
    BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
    BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
    CpuLib|MdePkg/Library/BaseCpuLib/BaseCpuLib.inf
    PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
    RegisterFilterLib|MdePkg/Library/RegisterFilterLibNull/RegisterFilterLibNull.inf
    MtrrLib|UefiCpuPkg/Library/MtrrLib/MtrrLib.inf
    # UEFI & PI
    DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
    UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
    UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
    UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
    # Generic Modules
    PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf
    # Misc
    IoLib|MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf
    DebugLib|OvmfPkg/Library/PlatformDebugLibIoPort/PlatformDebugLibIoPort.inf
    DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
    FileHandleLib|MdePkg/Library/UefiFileHandleLib/UefiFileHandleLib.inf
    StackCheckLib|MdePkg/Library/StackCheckLib/StackCheckLib.inf
    StackCheckFailureHookLib|MdePkg/Library/StackCheckFailureHookLibNull/StackCheckFailureHookLibNull.inf

[LibraryClasses.common.UEFI_APPLICATION]
    MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf

[Components.X64]
    AOS/uefi.inf

[PcdsFixedAtBuild]
    gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x2F
    gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x8020007F