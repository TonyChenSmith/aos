/*
 * 图像操作相关实现。设置VGA兼容输出相关。
 * @date 2025-01-25
 * 
 * Copyright (c) 2025 Tony Chen Smith. All rights reserved.
 * 
 * SPDX-License-Identifier: MIT
 */
#include <graphics.h>

/*
 * 获取单个像素色彩位宽。
 *
 * @param info 图像输出模式信息。
 * 
 * @return 单个像素位宽，未知返回0，不支持返回-1。
 */
STATIC UINT32 EFIAPI uefi_get_pixel_bit_width(IN EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* info)
{
	/*位标记*/
	EFI_PIXEL_BITMASK* mask;
	mask=&info->PixelInformation;

	switch(info->PixelFormat)
	{
		case PixelRedGreenBlueReserved8BitPerColor:
		case PixelBlueGreenRedReserved8BitPerColor:
			/*位宽全对齐*/
			return 32;
		case PixelBltOnly:
			/*不支持，在获取位宽时告诉外部。在选择好GOP后不应该出现该选项。*/
			return -1;
		case PixelBitMask:
			return BitFieldCountOnes32(mask->BlueMask|mask->GreenMask|mask->RedMask|mask->ReservedMask,0,31);
		default:
			return 0;
	}
}

/*
 * 从EDID上获取屏幕界限。
 *
 * @param gop  图像输出协议。
 * @param hres 水平分辨率。
 * @param vres 竖直分辨率。
 * 
 * @return 状态码，出错返回非零值。
 */
STATIC EFI_STATUS EFIAPI uefi_get_edid_info(IN EFI_HANDLE gop,OUT UINT32* hres,OUT UINT32* vres)
{
	EFI_STATUS status;
	/*第一优先协议*/
	EFI_EDID_ACTIVE_PROTOCOL* edid_active;
	/*第二优先协议*/
	EFI_EDID_DISCOVERED_PROTOCOL* edid_discovered;
	/*EDID*/
	EDID* edid;
	/*检查值*/
	UINT8 check;
	/*值检查用指针*/
	UINT8* edid_value;
	/*值检查用索引*/
	UINT8 index;

	/*第一步，获取当前显示屏对应的图像输出协议*/
	status=gBS->OpenProtocol(gop,&gEfiEdidActiveProtocolGuid,(VOID**)&edid_active,gImageHandle,NULL,EFI_OPEN_PROTOCOL_GET_PROTOCOL);
	if(EFI_ERROR(status)||edid_active->SizeOfEdid==0)
	{
		/*不支持或者EDID大小为0*/
		status=gBS->OpenProtocol(gop,&gEfiEdidDiscoveredProtocolGuid,(VOID**)&edid_discovered,gImageHandle,NULL,EFI_OPEN_PROTOCOL_GET_PROTOCOL);
		if(EFI_ERROR(status)||edid_discovered->SizeOfEdid==0)
		{
			/*都不支持*/
			return EFI_UNSUPPORTED;
		}
		else
		{
			edid=(EDID*)edid_discovered->Edid;
		}
	}
	else
	{
		edid=(EDID*)edid_active->Edid;
	}

	/*第二部，校验EDID是否合法*/
	edid_value=(UINT8*)edid;
	check=0;
	for(index=0;index<128;index++)
	{
		check=check+edid_value[index];
	}
	if(check!=0)
	{
		/*EDID校验失败*/
		return EFI_UNSUPPORTED;
	}
	if(edid->version_number!=1||edid->detailed_timings[0].pixel_clock==0)
	{
		/*不支持这种版本或没有定义最佳显示*/
		/*版本号2开始的4k、8k屏幕暂未有EDID支持想法*/
		return EFI_UNSUPPORTED;
	}

	/*第三步，获取参数*/
	*hres=edid->detailed_timings[0].horizontal_active_low|(((UINT32)(edid->detailed_timings[0].horizontal_high&0xF0))<<4);
	*vres=edid->detailed_timings[0].vertical_active_low|(((UINT32)(edid->detailed_timings[0].vertical_active_low&0xF0))<<4);
	return EFI_SUCCESS;
}

/*
 * 初始化设备图像信息。
 *
 * @param params 启动核参数。
 * 
 * @return 状态码，出错返回非零值。 
 */
EFI_STATUS EFIAPI uefi_init_graphics(IN OUT boot_params* restrict params)
{
	EFI_STATUS status;
	/*全部GOP*/
	EFI_HANDLE* gops;
	/*GOP数目，索引*/
	UINTN gops_count,index;
	/*当前GOP*/
	EFI_GRAPHICS_OUTPUT_PROTOCOL* current_gop;
	/*GOP*/
	EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;
	EFI_HANDLE gop_handle;

	/*当前模式*/
	EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE* current_mode;
	/*最大模式*/
	UINT32 max_mode;
	/*水平分辨率*/
	UINT32 hres;
	/*竖直分辨率*/
	UINT32 vres;
	/*色彩深度*/
	UINT32 depth;
	/*最终选择模式*/
	UINT32 final_mode;
	/*模式信息*/
	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* info;
	/*模式信息大小*/
	UINTN info_size;
	/*水平竖直极限*/
	UINT32 hlim,vlim;
	/*三个临时对应量，用于方便调用和比较，modei为模式遍历*/
	UINT32 h,v,d,modei;

	/*图像信息*/
	boot_graphics_info* ginfo;

	/*
	 * 第一步，找到我们需要的图像输出协议(GOP)。GOP至少含有ConsoleOutDevice，否则取第一个GOP。
	 * 我们OS不打算对EFI1.x版本进行兼容，所以不搜索UGA。
	 * 因为我们OS是一个图形OS，缺少屏幕对我们使用没有意义。
	 * 这次打开的协议不关闭，后续内存大整理时直接视为可用内存。
	 * GOP的模式不能为仅Blt()输出，否则视为无效GOP。
	 */
	status=gBS->LocateHandleBuffer(ByProtocol,&gEfiGraphicsOutputProtocolGuid,NULL,&gops_count,&gops);
	if(EFI_ERROR(status))
	{
		return status;
	}
	gop=NULL;
	for(index=0;index<gops_count;index++)
	{
		status=gBS->OpenProtocol(gops[index],&gEfiGraphicsOutputProtocolGuid,(VOID**)&current_gop,gImageHandle,NULL,EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
		if(EFI_ERROR(status))
		{
			/*不应该出现*/
			return status;
		}
		if(current_gop->Mode->Info->PixelFormat==PixelBltOnly||current_gop->Mode->Info->PixelFormat>=PixelFormatMax)
		{
			/*非目标GOP*/
			continue;
		}
		status=gBS->OpenProtocol(gops[index],&gEfiConsoleOutDeviceGuid,NULL,gImageHandle,NULL,EFI_OPEN_PROTOCOL_TEST_PROTOCOL);
		if(status==EFI_SUCCESS)
		{
			/*支持ConsoleOutDevice即为目标输出设备*/
			gop=current_gop;
			gop_handle=gops[index];
			break;
		}
		if(gop==NULL)
		{
			/*假设GOP可显示*/
			gop=current_gop;
			gop_handle=gops[index];
		}
	}
	gBS->FreePool(gops);
	if(gop==NULL)
	{
		/*没有GOP可用*/
		return EFI_UNSUPPORTED;
	}

	/*
	 * 第二步，查找并自动设置显示模式，查找方式最大最佳。色彩空间由于以前的协约，RGB位宽基本一致。
	 * 这一步会先尝试找EDID。设置参数优先级如下：
	 * 1.配置文件信息，该文件仅在第一次运行OS后才创建。(后续决定好再添加该优先级)(未做)
	 * 2.EDID。屏幕显示大小应为正常格式，非标屏幕只能兼容黑边运行。
	 * 3.非标默认显示大小，先检查当前大小，再检查非VGA(640x480x32)大小的模式0。(1针对VBox，2针对QEMU)
	 * 4.毫无参数情况下默认XGA(1024x768x32)或更小。
	 */
	current_mode=gop->Mode;
	max_mode=current_mode->MaxMode;
	hres=current_mode->Info->HorizontalResolution;
	vres=current_mode->Info->VerticalResolution;
	final_mode=current_mode->Mode;
	depth=uefi_get_pixel_bit_width(current_mode->Info);

	status=uefi_get_edid_info(gop_handle,&hlim,&vlim);
	if(EFI_ERROR(status))
	{
		/*没有EDID。检查模式0。*/
		status=gop->QueryMode(gop,0,&info_size,&info);
		if(EFI_ERROR(status))
		{
			/*不可能是非法参数。*/
			return status;
		}
		hlim=info->HorizontalResolution;
		vlim=info->VerticalResolution;
		/*检查水平分辨率，一般不会用纵屏。*/
		if(hlim<1024)
		{
			/*未特殊设置模式0*/
			if(hres>1024)
			{
				hlim=hres;
				vlim=vres;
			}
			else
			{
				/*至少要支持XGA*/
				hlim=1024;
				vlim=768;
			}
		}
		else
		{
			/*与现有进行比较，选择较大模式的参数*/
			if(hres>hlim||(hres==hlim&&vres>vlim))
			{
				hlim=hres;
				vlim=vres;
			}
		}
		gBS->FreePool(info);
	}
	info=NULL;

	for(modei=0;modei<max_mode;modei++)
	{
		status=gop->QueryMode(gop,modei,&info_size,&info);
		if(EFI_ERROR(status))
		{
			/*不可能是非法参数。*/
			return status;
		}

		h=info->HorizontalResolution;
		v=info->VerticalResolution;
		d=uefi_get_pixel_bit_width(info);

		/*
		 * 第一条件：水平分辨率大。
		 * 第二条件：同等水平分辨率下竖直分辨率大。
		 * 第三条件：同等分辨率下色宽大。
		 */
		if(h>hres||(h==hres&&v>vres)||(h==hres&&v==vres&&d>depth))
		{
			if(h<=hlim&&v<=vlim)
			{
				final_mode=modei;
				hres=h;
				vres=v;
				depth=d;
			}
		}
		gBS->FreePool(info);
	}

	if(depth==-1)
	{
		return EFI_PROTOCOL_ERROR;
	}

	status=gop->SetMode(gop,final_mode);
	if(EFI_ERROR(status))
	{
		return status;
	}

	/*
	 * 第三步，设置参数。
	 */
	current_mode=gop->Mode;
	info=current_mode->Info;
	ginfo=&params->graphics_info;
	ginfo->frame_buffer_base=current_mode->FrameBufferBase;
	ginfo->frame_buffer_size=current_mode->FrameBufferSize;
	ginfo->horizontal_resolution=info->HorizontalResolution;
	ginfo->vertical_resolution=info->VerticalResolution;
	ginfo->scan_line_length=info->PixelsPerScanLine;
	if(info->PixelFormat==PixelBitMask)
	{
		ginfo->blue_mask=info->PixelInformation.BlueMask;
		ginfo->red_mask=info->PixelInformation.RedMask;
		ginfo->green_mask=info->PixelInformation.GreenMask;
		ginfo->reserved_mask=info->PixelInformation.ReservedMask;
	}
	else if(info->PixelFormat==PixelBlueGreenRedReserved8BitPerColor)
	{
		ginfo->blue_mask=0x000000FF;
		ginfo->red_mask=0x00FF0000;
		ginfo->green_mask=0x0000FF00;
		ginfo->reserved_mask=0xFF000000;
	}
	else
	{
		/*仅有可能是RGBR模式*/
		ginfo->blue_mask=0x00FF0000;
		ginfo->red_mask=0x000000FF;
		ginfo->green_mask=0x0000FF00;
		ginfo->reserved_mask=0xFF000000;
	}

	return EFI_SUCCESS;
}