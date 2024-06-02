/*
 * 信息显示指令，主程序入口。
 * 反汇编程序
 llvm-objdump --x86-asm-syntax=intel -d --demangle Info.efi
 * @date 2024-05-12
 */
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>

CHAR16*
EFIAPI
check_page(
  IN UINTN addr,
  IN EFI_MEMORY_DESCRIPTOR* map,
  IN UINTN map_size,
  IN UINTN map_entry_size
  );

typedef union {
  UINT64 value;
  struct {
    UINT32 P:1;
    UINT32 RW:1;
    UINT32 US:1;
    UINT32 PWT:1;
    UINT32 PCD:1;
    UINT32 A:1;
    UINT32 R1:6;
    UINT64 addr:40;
    UINT32 R2:11;
    UINT32 XD:1;
  } PDE; //目录级别 PML5 PML4 
  struct {
    UINT32 P:1;
    UINT32 RW:1;
    UINT32 US:1;
    UINT32 PWT:1;
    UINT32 PCD:1;
    UINT32 A:1;
    UINT32 D:1;
    UINT32 PS:1;
    UINT32 G:1;
    UINT32 R1:3;
    UINT32 PAT:1;
    UINT64 addr:39;
    UINT32 R3:7;
    UINT32 PK:4;
    UINT32 XD:1;
  } PTE1; //PS页表级别
  struct {
    UINT32 P:1;
    UINT32 RW:1;
    UINT32 US:1;
    UINT32 PWT:1;
    UINT32 PCD:1;
    UINT32 A:1;
    UINT32 D:1;
    UINT32 PAT:1;
    UINT32 G:1;
    UINT32 R1:3;
    UINT64 addr:40;
    UINT32 R3:7;
    UINT32 PK:4;
    UINT32 XD:1;
  } PTE2; //页表级别 4k页表
} IA32_PET;


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

  //探测后常量
  UINTN map_size;
  UINTN map_entry_size;
  UINT64 ta,tb;

  //GDT
  IA32_DESCRIPTOR Gdtr;
  IA32_SEGMENT_DESCRIPTOR* seg;

  //Paging
  IA32_CR4 cr4;
  UINTN cr3;
  UINT64 efer;
  UINT32 eax,ebx,ecx,edx;
  UINT32 test;
  BOOLEAN is5,ispcid;
  UINTN pml;
  //UINTN i1,i2,i3,i4,i5;
  IA32_PET* pet;

  is5=FALSE;
  ispcid=FALSE;
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

  map_size=size;
  map_entry_size=dsize;

  /*
   * GDT与页表信息
   */
  //GDT
  AsmReadGdtr(&Gdtr);
  Print(L"\r\n-----\r\nCurrent GDT infomation\r\nBase:%016X Limit:%04X\r\n",Gdtr.Base,Gdtr.Limit);
  
  for(offset=0;offset<map_size;offset=offset+map_entry_size)
  {
    entry=(EFI_MEMORY_DESCRIPTOR*)((VOID*)map+offset);
    ta=entry->PhysicalStart;
    tb=entry->PhysicalStart+LShiftU64(entry->NumberOfPages,12)-1;
    if(Gdtr.Base>=ta&&Gdtr.Base+Gdtr.Limit<=tb)
    {
      Print(L"Memory Type:");
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
      Print(L"\r\n");
      break;
    }
  }
  
  size=Gdtr.Limit+1;
  dsize=8;
  offset=0;
  for(index=0;offset<size;offset=offset+dsize,index=index+1)
  {
    seg=(IA32_SEGMENT_DESCRIPTOR*)(Gdtr.Base+offset);
    Print(L"%02X Base:%032X Size:%05X T:%X S:%c DPL:%u P:%c AVL:%u L:%c D/B:%ls G:%c\r\n",
      index,
      (seg->Bits.BaseHigh<<24)|(seg->Bits.BaseMid<<16)|(seg->Bits.BaseLow),
      (seg->Bits.LimitHigh<<16)|(seg->Bits.LimitLow),
      seg->Bits.Type,
      seg->Bits.S==1?'U':'S', //用户或系统
      seg->Bits.DPL,
      seg->Bits.P==1?'P':'N', //存在或不存在
      seg->Bits.AVL,
      seg->Bits.L==1?'L':'N', //长模式或短模式
      seg->Bits.DB==1?L"32b":L"16b", //32位操作数或16位操作数
      seg->Bits.G==1?'S':'U' //缩小或等大
      );
  }

  //Page
  Print(L"\r\n-----\r\nCurrent Page infomation\r\n\r\nCPU Features\r\n\r\n");
  //尝试功能检查 x64不需要PSE PAE默认支持 检查PGE PCID LA57 Page1GB
  AsmCpuid(0x01,&eax,&ebx,&ecx,&edx); //PGE PCID
  test=edx&0x00001000;
  if(test)
  {
    Print(L"Global-page support.\r\n");
  }
  else
  {
    Print(L"Global-page unsupport.\r\n");
  }
  test=ecx&0x00020000;
  if(test)
  {
    Print(L"Process-context identifiers support.\r\n");
  }
  else
  {
    Print(L"Process-context identifiers unsupport.\r\n");
  }
  AsmCpuidEx(0x07,0,&eax,&ebx,&ecx,&edx); //LA57
  test=ecx&0x00010000;
  if(test)
  {
    Print(L"5-level paging support.\r\n");
  }
  else
  {
    Print(L"5-level paging unsupport.\r\n");
  }
  AsmCpuid(0x80000001,&eax,&ebx,&ecx,&edx); //Page1GB
  test=edx&0x04000000;
  if(test)
  {
    Print(L"1-GByte pages support.\r\n");
  }
  else
  {
    Print(L"1-GByte pages unsupport.\r\n");
  }

  Print(L"\r\nCurrent paging\r\n\r\n");
  cr4.UintN=AsmReadCr4();
  cr3=AsmReadCr3();
  efer=AsmReadMsr64(0xC0000080);

  if(cr4.Bits.LA57)
  {
    is5=TRUE;
    Print(L"5-level paging enabled.\r\n");
  }
  else
  {
    Print(L"4-level paging enabled.\r\n");
  }
  if(cr4.Bits.PCIDE)
  {
    ispcid=TRUE; //不应该出现，由于AMD的支持原因也不打算使用。
    Print(L"Process-context identifiers enabled.\r\n");
  }
  else
  {
    Print(L"Common paging enable.\r\n");
  }

  //解析页表
  //qemu有个unknown1的bug 目前检查认为是cpu bug
  pml=cr3&0x000FFFFFFFFFF000LL;
  Print(L"\r\nPaging Table\r\n\r\n");
  //分析页表
  if(is5)
  {
    //5级页表
    Print(L"Root Table:PML5 Base:%013X Type:%ls\r\nEntry:\r\n",pml,check_page(pml,map,map_size,map_entry_size));
    size=pml;
    for(offset=0;offset<0x1000;offset=offset+8)
    {
      pet=(IA32_PET*)(VOID*)(size+offset);
      if(pet->PDE.P)
      {
        Print(L"%03X P:%c Base:%013X T:PDPT Type:%ls\r\n",
          offset,
          pet->PDE.P?'P':'U',
          pet->PDE.addr<<12,
          check_page((pet->PDE.addr<<12)&0x000FFFFFFFFFF000LL,map,map_size,map_entry_size));
      }
    }
  }
  else
  {
    //4级页表
    Print(L"Root Table:PML4 Base:%013X Type:%ls\r\nEntry:\r\n",pml,check_page(pml,map,map_size,map_entry_size));
    size=pml;
    for(offset=0;offset<0x1000;offset=offset+8)
    {
      pet=(IA32_PET*)(VOID*)(size+offset);
      if(pet->PTE1.PS)
      {
        Print(L"%03X P:%c Base:%013X T:PDPT1GB Type:%ls\r\n",
          offset,
          pet->PTE1.P?'P':'U',
          pet->PTE1.addr<<13,
          check_page((pet->PTE1.addr<<13)&0x000FFFFFFFFFF000LL,map,map_size,map_entry_size));
      }
      else
      {
        if(pet->PDE.P)
        {
          Print(L"%03X P:%c Base:%013X T:PDPT Type:%ls\r\n",
            offset,
            pet->PDE.P?'P':'U',
            pet->PDE.addr<<12,
            check_page((pet->PDE.addr<<12)&0x000FFFFFFFFFF000LL,map,map_size,map_entry_size));
        }
      }
    }
  }

  //结束才释放
  status=gBS->FreePool(map);
  if(EFI_ERROR(status))
  {
    return status;
  }
  return EFI_SUCCESS;
}

/*
static
VOID 
EFIAPI 
memory_scan(
  IN UINTN offset,
  IN UINTN index,
  IN UINTN size,
  IN UINTN dsize,
  IN EFI_MEMORY_DESCRIPTOR* map,
  IN EFI_MEMORY_DESCRIPTOR* entry
  )
{
  for(offset=0,index=1;offset<size;offset=offset+dsize,index=index+1)
  {
    entry=(EFI_MEMORY_DESCRIPTOR*)((VOID*)map+offset);
    Print(L"%2X",index);
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
}
*/

//单页检查，地址为页地址
CHAR16*
EFIAPI
check_page(
  IN UINTN addr,
  IN EFI_MEMORY_DESCRIPTOR* map,
  IN UINTN map_size,
  IN UINTN map_entry_size
  )
{
  UINTN offset;
  EFI_MEMORY_DESCRIPTOR* entry;
  UINTN ta,tb,tc,td;

  for(offset=0;offset<map_size;offset=offset+map_entry_size)
  {
    entry=(EFI_MEMORY_DESCRIPTOR*)((VOID*)map+offset);
    ta=entry->PhysicalStart;
    tb=entry->PhysicalStart+LShiftU64(entry->NumberOfPages,12)-1;
    tc=addr;
    td=addr+0xFFF;
    if(ta<=tc&&td<=tb)
    {
      switch(entry->Type)
      {
      case EfiReservedMemoryType:
        return L"EfiReservedMemoryType";
      case EfiLoaderCode:
        return L"EfiLoaderCode";
      case EfiLoaderData:
        return L"EfiLoaderData";
      case EfiBootServicesCode:
        return L"EfiBootServicesCode";
      case EfiBootServicesData:
        return L"EfiBootServicesData";
      case EfiRuntimeServicesCode:
        return L"EfiRuntimeServicesCode";
      case EfiRuntimeServicesData:
        return L"EfiRuntimeServicesData";
      case EfiConventionalMemory:
        return L"EfiConventionalMemory";
      case EfiUnusableMemory:
        return L"EfiUnusableMemory";
      case EfiACPIReclaimMemory:
        return L"EfiACPIReclaimMemory";
      case EfiACPIMemoryNVS:
        return L"EfiACPIMemoryNVS";
      case EfiMemoryMappedIO:
        return L"EfiMemoryMappedIO";
      case EfiMemoryMappedIOPortSpace:
        return L"EfiMemoryMappedIOPortSpace";
      case EfiPalCode:
        return L"EfiPalCode";
      case EfiMaxMemoryType:
        return L"EfiMaxMemoryType";
      default:
        return L"Unknown2";
      }
    }
  }
  return L"Unknown1";
}