/*
 * 信息显示指令，主程序入口。
 * @date 2024-05-12
 */
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>

EFI_STATUS
EFIAPI
CommandEntry(
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  //内存信息
  UINTN size;
  EFI_STATUS status;
  EFI_MEMORY_DESCRIPTOR* map;
  UINTN dsize;
  UINT32 ver;
  UINTN key;
  UINTN offset;
  UINTN index;
  EFI_MEMORY_DESCRIPTOR* entry;

  //GDT
  IA32_DESCRIPTOR Gdtr;
  IA32_SEGMENT_DESCRIPTOR* seg;

  size=0;

  status=gBS->GetMemoryMap(&size,NULL,&key,&dsize,&ver);
  if(status!=EFI_BUFFER_TOO_SMALL)
  {
    return status;
  }

  status=gBS->AllocatePool(EfiBootServicesData,size,(VOID**)&map);
  if(EFI_ERROR(status))
  {
    return status;
  }

  /*
   * Addr ST.
   */
  Print(L"System Table:%016lX\r\n",(VOID*)SystemTable);
  status=gBS->GetMemoryMap(&size,map,&key,&dsize,&ver);
  if(EFI_ERROR(status))
  {
    return status;
  }
  for(offset=0,index=1;offset<size;offset=offset+dsize,index=index+1)
  {
    entry=(EFI_MEMORY_DESCRIPTOR*)((VOID*)map+offset);
    Print(L"%2d",index);
    switch(entry->Type)
    {
      case EfiReservedMemoryType:
        Print(L"%27ls",L"EfiReservedMemoryType");
        break;
      case EfiLoaderCode:
        Print(L"%27ls",L"EfiLoaderCode");
        break;
      case EfiLoaderData:
        Print(L"%27ls",L"EfiLoaderData");
        break;
      case EfiBootServicesCode:
        Print(L"%27ls",L"EfiBootServicesCode");
        break;
      case EfiBootServicesData:
        Print(L"%27ls",L"EfiBootServicesData");
        break;
      case EfiRuntimeServicesCode:
        Print(L"%27ls",L"EfiRuntimeServicesCode");
        break;
      case EfiRuntimeServicesData:
        Print(L"%27ls",L"EfiRuntimeServicesData");
        break;
      case EfiConventionalMemory:
        Print(L"%27ls",L"EfiConventionalMemory");
        break;
      case EfiUnusableMemory:
        Print(L"%27ls",L"EfiUnusableMemory");
        break;
      case EfiACPIReclaimMemory:
        Print(L"%27ls",L"EfiACPIReclaimMemory");
        break;
      case EfiACPIMemoryNVS:
        Print(L"%27ls",L"EfiACPIMemoryNVS");
        break;
      case EfiMemoryMappedIO:
        Print(L"%27ls",L"EfiMemoryMappedIO");
        break;
      case EfiMemoryMappedIOPortSpace:
        Print(L"%27ls",L"EfiMemoryMappedIOPortSpace");
        break;
      case EfiPalCode:
        Print(L"%27ls",L"EfiPalCode");
        break;
      case EfiMaxMemoryType:
        Print(L"%27ls",L"EfiMaxMemoryType");
        break;
      default:
        Print(L"%27ls",L"Unknown");
        break;
    }
    Print(L" P:%016lX-%016lX V:%016lX-%016lX A:%016lX\r\n",
      entry->PhysicalStart,
      entry->PhysicalStart+LShiftU64(entry->NumberOfPages,12),
      entry->VirtualStart,
      entry->VirtualStart+LShiftU64(entry->NumberOfPages,12),
      entry->Attribute);
  }
  status=gBS->FreePool(map);
  if(EFI_ERROR(status))
  {
    return status;
  }

  /*
   * GDT与页表信息
   */
  AsmReadGdtr(&Gdtr);
  Print(L"-----\r\nCurrent GDT infomation\r\nBase:%016X Limit:%04X\r\n\r\n",Gdtr.Base,Gdtr.Limit);
  size=Gdtr.Limit+1;
  dsize=8;
  offset=0;
  for(index=0;offset<size;offset=offset+1,index=index+1)
  {
    seg=(IA32_SEGMENT_DESCRIPTOR*)(Gdtr.Base+offset);
    Print(L"%01d %05X:%032X \r\n");
  }
  return EFI_SUCCESS;
}