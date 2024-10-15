/*
 * 研究OVMF内页表的设计。
 * @date 2024-09-15
 */

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>

typedef union {
  UINT64 value;
  struct {
	UINT32 P:1;
	UINT32 R1:6;
	UINT32 PS:1;
	UINT32 R2:32;
	UINT32 R3:24;
  } PE; //基础表项
  struct {
	UINT32 P:1;
	UINT32 RW:1;
	UINT32 US:1;
	UINT32 PWT:1;
	UINT32 PCD:1;
	UINT32 A:1;
	UINT32 R1:6;
	UINT32 adown:20;
	UINT32 aup:20;
	UINT32 R2:11;
	UINT32 XD:1;
  } PML; //目录级别 PML5 PML4
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
	UINT32 adown:19;
	UINT32 aup:20;
	UINT32 R2:7;
	UINT32 PK:4;
	UINT32 XD:1;
  } P1G; //1GB
  struct {
	UINT32 P:1;
	UINT32 RW:1;
	UINT32 US:1;
	UINT32 PWT:1;
	UINT32 PCD:1;
	UINT32 A:1;
	UINT32 R1:6;
	UINT32 adown:20;
	UINT32 aup:20;
	UINT32 R2:11;
	UINT32 XD:1;
  } PDPTE;
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
	UINT32 adown:19;
	UINT32 aup:20;
	UINT32 R3:7;
	UINT32 PK:4;
	UINT32 XD:1;
  } P2M; //PS页表级别
  struct {
	UINT32 P:1;
	UINT32 RW:1;
	UINT32 US:1;
	UINT32 PWT:1;
	UINT32 PCD:1;
	UINT32 A:1;
	UINT32 R1:6;
	UINT32 adown:20;
	UINT32 aup:20;
	UINT32 R2:11;
	UINT32 XD:1;
  } PDE;
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
	UINT32 adown:20;
	UINT32 aup:20;
	UINT32 R2:7;
	UINT32 PK:4;
	UINT32 XD:1;
  } P4K; //页表级别 4k页表
} IA32_PT;

EFI_STATUS
EFIAPI
CommandEntry(
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
	//4 2 1
	UINT32 count[]={0,0,0};
	UINT64 pml=AsmReadCr3();
	Print(L"5l %X\n",AsmReadCr4());
	if(AsmReadCr4()&0x1000)
	{
		Print(L"PML5\n");
		//5级页逻辑。
		UINT64* pml5_base=(UINT64*)((VOID*)(BitFieldRead64(pml,12,51)<<12));
		for(int offset0=0;offset0<512;offset0++)
		{
			UINT64* pml5e=pml5_base+offset0;
			if(*pml5e&1)
			{
				//存在PML4
				UINT64* pml4_base=(UINT64*)((VOID*)(BitFieldRead64(*pml5e,12,51)<<12));
				for(int offset1=0;offset1<512;offset1++)
				{
					UINT64* pml4e=pml4_base+offset1;
					if(*pml4e&1)
					{
						//存在PDPT
                        UINT64* pdpt_base=(UINT64*)((VOID*)(BitFieldRead64(*pml4e,12,51)<<12));
						for(int offset2=0;offset2<512;offset2++)
						{
							UINT64* pdpte=pdpt_base+offset2;
							if(*pdpte&1)
							{
								//存在1GB大页或者PD
								if(*pdpte&0x80)
								{
									//1GB大页
									count[2]++;
								}
								else
								{
									UINT64* pd_base=(UINT64*)((VOID*)(BitFieldRead64(*pdpte,12,51)<<12));
									for(int offset3=0;offset3<512;offset3++)
									{
										UINT64* pde=pd_base+offset3;
										if(*pde&1)
										{
											//存在2MB大页或者PT
											if(*pde&0x80)
											{
												//2MB大页
												count[1]++;
											}
											else
											{
												UINT64* pt_base=(UINT64*)((VOID*)(BitFieldRead64(*pde,12,51)<<12));
												for(int offset4=0;offset4<512;offset4++)
												{
													UINT64* pte=pt_base+offset4;
													if(*pte&1)
													{
														//存在4k页
														count[0]++;
													}
												}
											}
										}
									}
								}
							}
						}
                    }
				}
			}
		}
		Print(L"5l 4k:%u 2M:%u 1G:%u\n",count[0],count[1],count[2]);
	}
	else
	{
		Print(L"PML4\n");
		UINT64* pml4_base=(UINT64*)((VOID*)(BitFieldRead64(pml,12,51)<<12));
		for(int offset1=0;offset1<512;offset1++)
		{
			UINT64* pml4e=pml4_base+offset1;
			if(*pml4e&1)
			{
				//存在PDPT
				UINT64* pdpt_base=(UINT64*)((VOID*)(BitFieldRead64(*pml4e,12,51)<<12));
				for(int offset2=0;offset2<512;offset2++)
				{
					UINT64* pdpte=pdpt_base+offset2;
					if(*pdpte&1)
					{
						//存在1GB大页或者PD
						if(*pdpte&0x80)
						{
							//1GB大页
							count[2]++;
						}
						else
						{
							UINT64* pd_base=(UINT64*)((VOID*)(BitFieldRead64(*pdpte,12,51)<<12));
							for(int offset3=0;offset3<512;offset3++)
							{
								UINT64* pde=pd_base+offset3;
								if(*pde&1)
								{
									//存在2MB大页或者PT
									if(*pde&0x80)
									{
										//2MB大页
										count[1]++;
									}
									else
									{
										UINT64* pt_base=(UINT64*)((VOID*)(BitFieldRead64(*pde,12,51)<<12));
										for(int offset4=0;offset4<512;offset4++)
										{
											UINT64* pte=pt_base+offset4;
											if(*pte&1)
											{
												//存在4k页
												count[0]++;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
		Print(L"4l 4k:%u 2M:%u 1G:%u\n",count[0],count[1],count[2]);
	}
	return EFI_SUCCESS;
}