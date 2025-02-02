#
# Mirror tools describe
#
# Copyright (c) 2024-2025 Tony Chen Smith. All rights reserved.
#
# SPDX-License-Identifier: MIT
#
[Defines]
  PLATFORM_NAME                  = MirrorTool
  PLATFORM_GUID                  = CB4A0B7C-37AA-3657-A14F-6F9E0AA5589F #UUID.nameUUIDFromBytes("MirrorTool_DSC".getBytes())
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/MirrorTool
  SUPPORTED_ARCHITECTURES        = IA32|X64
  BUILD_TARGETS                  = DEBUG|RELEASE|NOOPT
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
  # DebugLib|MdePkg/Library/UefiDebugLibStdErr/UefiDebugLibStdErr.inf
  IoLib|MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf
  DebugLib|OvmfPkg/Library/PlatformDebugLibIoPort/PlatformDebugLibIoPort.inf
  DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
  FileHandleLib|MdePkg/Library/UefiFileHandleLib/UefiFileHandleLib.inf

[LibraryClasses.common.UEFI_APPLICATION]
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf

[Components.X64]
  #ToolPkg/Core/Test/Test.inf
  #ToolPkg/Application/Info/Info.inf
  #ToolPkg/Application/DisplayAdapter/DisplayAdapter.inf

  # Use the package directly without creating a new one.
  ToolPkg/Core/aosuefitsl/aosuefitsl.inf
  #UefiCpuPkg/Application/Cpuid/Cpuid.inf

#  ToolPkg/Application/Memory/Memory.inf{
#	<LibraryClasses>
#		DebugLib|MdePkg/Library/UefiDebugLibConOut/UefiDebugLibConOut.inf
#  }

[PcdsFixedAtBuild]
  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x2F
  gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x8020007F