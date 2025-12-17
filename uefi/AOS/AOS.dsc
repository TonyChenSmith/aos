# 
# Description of the module "aos.uefi".
# @date 2025-01-24
# 
# Copyright (c) 2025 Tony Chen Smith
# 
# SPDX-License-Identifier: MIT
# 
[Defines]
    PLATFORM_NAME              = AOS
    # UUID.nameUUIDFromBytes("aos.uefi.description".getBytes("UTF-8"))
    PLATFORM_GUID              = DE82DA92-FC7F-3E64-A5BC-619E1E42E1A4
    # Major.Minor.Patch Before the official launch, do not increment the major version number.
    PLATFORM_VERSION           = 0.0.2
    DSC_SPECIFICATION          = 0x00010005
    OUTPUT_DIRECTORY           = Build/AOS
    SUPPORTED_ARCHITECTURES    = X64
    BUILD_TARGETS              = DEBUG|RELEASE
    RFC_LANGUAGES              = "en-us"
    SKUID_IDENTIFIER           = DEFAULT

[BuildOptions]
    GCC:RELEASE_*_*_CC_FLAGS   = -DMDEPKG_NDEBUG -DAOS_NDEBUG
    INTEL:RELEASE_*_*_CC_FLAGS = /D MDEPKG_NDEBUG /D AOS_NDEBUG
    MSFT:RELEASE_*_*_CC_FLAGS  = /D MDEPKG_NDEBUG /D AOS_NDEBUG
!if $(TOOL_CHAIN_TAG) != "XCODE5" && $(TOOL_CHAIN_TAG) != "CLANGPDB"
    GCC:*_*_*_CC_FLAGS         = -mno-mmx -mno-sse
!endif

[LibraryClasses]
    BaseCryptLib|CryptoPkg/Library/BaseCryptLib/BaseCryptLib.inf
    BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
    BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
    CpuLib|MdePkg/Library/BaseCpuLib/BaseCpuLib.inf
    DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
    DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
    FileHandleLib|MdePkg/Library/UefiFileHandleLib/UefiFileHandleLib.inf
    IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
    IoLib|MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf
    MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
    OpensslLib|CryptoPkg/Library/OpensslLib/OpensslLib.inf
    PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf
    PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
    RegisterFilterLib|MdePkg/Library/RegisterFilterLibNull/RegisterFilterLibNull.inf
    RngLib|MdeModulePkg/Library/BaseRngLibTimerLib/BaseRngLibTimerLib.inf
    StackCheckFailureHookLib|MdePkg/Library/StackCheckFailureHookLibNull/StackCheckFailureHookLibNull.inf
    StackCheckLib|MdePkg/Library/StackCheckLib/StackCheckLib.inf
    SynchronizationLib|MdePkg/Library/BaseSynchronizationLib/BaseSynchronizationLib.inf
    TimerLib|AOS/lib/timer/timer.inf
    UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
    UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
    UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
    UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
    
    # Debug Library
!if $(TARGET) == RELEASE
    DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
!else
    DebugLib|OvmfPkg/Library/PlatformDebugLibIoPort/PlatformDebugLibIoPort.inf
!endif

[LibraryClasses.common.UEFI_APPLICATION]

[Components.X64]
    AOS/lib/timer/timer.inf
    AOS/uefi.inf

[PcdsFixedAtBuild]
    gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x2F
    gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x8020007F