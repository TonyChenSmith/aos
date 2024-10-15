/*
 * AOS Loader图像程序。
 * 2024-05-31创建。
 */
#include "aosgraphics.h"

/*
 * 获取单个像素色彩位宽。
 */
static
UINT32
EFIAPI
aos_get_pixel_bit_width(
	IN EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* info
);

/*
 * 从EDID上获取屏幕界限。
 */
static
EFI_STATUS
EFIAPI
aos_get_edid_info(
	IN EFI_HANDLE gop,
	OUT UINT32* hlim,
	OUT UINT32* vlim
);

/*
 * 初始化图像信息。
 * @param boot_params 启动参数。
 * @retval EFI_SUCCESS 执行成功。
 * @retval EFI_UNSUPPORTED 不支持GOP。
 * @retval EFI_DEVICE_ERROR 查询时出现硬件错误。
 * @retcal EFI_PROTOCOL_ERROR 输出设备只支持Blt函数方式输出。
 * @retval EFI_DEVICE_ERROR 设置时出现硬件错误。
 * @retval EFI_NOT_FOUND 未找到GOP。
 * @retval EFI_OUT_OF_RESOURCES 缺乏内存空间。
 */
EFI_STATUS
EFIAPI
aos_init_graphics_info(
	IN OUT AOS_BOOT_PARAMS* boot_params
)
{
	EFI_STATUS status; /*状态*/
	EFI_HANDLE* gops; /*全部GOP*/
	UINTN gops_count,index; /*GOP数目，索引*/
	EFI_GRAPHICS_OUTPUT_PROTOCOL* current_gop; /*当前GOP*/
	EFI_GRAPHICS_OUTPUT_PROTOCOL* gop; /*GOP*/
	EFI_HANDLE gop_handle;

	EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE* current_mode; /*当前模式*/
	UINT32 max_mode; /*最大模式*/
	UINT32 hres; /*水平分辨率*/
	UINT32 vres; /*竖直分辨率*/
	UINT32 depth; /*色彩深度*/
	UINT32 final_mode; /*最终选择模式*/
	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* info; /*模式信息*/
	UINTN info_size; /*模式信息大小*/
	UINT32 hlim,vlim; /*水平竖直极限*/
	UINT32 h,v,d,modei; /*三个临时对应量，用于方便调用和比较，modei为模式遍历*/

	AOS_GRAPHICS_INFO* ginfo; /*图像信息*/

	/*
	 * 第一步，找到我们需要的GOP。GOP至少含有ConsoleOutDevice，否则取第一个GOP。
	 * 我们OS不打算对EFI1.x版本进行兼容，所以不搜索UGA。
	 * 因为我们OS是一个图形OS，缺少屏幕对我们使用没有意义。
	 * 这次打开的协议不关闭，后续内存大整理时直接视为可用内存。
	 * GOP的模式不能为仅Blt()输出，否则视为无效GOP。
	 */
	status=gBS->LocateHandleBuffer(
		ByProtocol,
		&gEfiGraphicsOutputProtocolGuid,
		NULL,
		&gops_count,
		&gops
	);
	if(EFI_ERROR(status))
	{
		return status;
	}
	gop=NULL;
	for(index=0;index<gops_count;index++)
	{
		status=gBS->OpenProtocol(
			gops[index],
			&gEfiGraphicsOutputProtocolGuid,
			(VOID**)&current_gop,
			gImageHandle,
			NULL,
			EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL
		);
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
		status=gBS->OpenProtocol(
			gops[index],
			&gEfiConsoleOutDeviceGuid,
			NULL,
			gImageHandle,
			NULL,
			EFI_OPEN_PROTOCOL_TEST_PROTOCOL
		);
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
	 * 1.配置文件信息，该文件仅在第一次运行OS后才创建。(后续决定好再添加该优先级)
	 * 2.EDID。屏幕显示大小应为正常格式，非标屏幕只能兼容黑边运行。
	 * 3.非标默认显示大小，先检查当前大小，再检查非VGA(640x480x32)大小的模式0。(1针对VBox，2针对QEMU)
	 * 4.毫无参数情况下默认XGA(1024x768x32)或更小。
	 */
	current_mode=gop->Mode;
	max_mode=current_mode->MaxMode;
	hres=current_mode->Info->HorizontalResolution;
	vres=current_mode->Info->VerticalResolution;
	final_mode=current_mode->Mode;
	depth=aos_get_pixel_bit_width(current_mode->Info);
	status=aos_get_edid_info(gop_handle,&hlim,&vlim);
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
		d=aos_get_pixel_bit_width(info);
		if(h>hres||(h==hres&&v>vres)||d>depth)
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
	ginfo=&boot_params->graphics_info;
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

/*
 * 获取单个像素色彩位宽。
 * @param info 模式信息。
 * @retval 位宽大小。
 */
static
UINT32
EFIAPI
aos_get_pixel_bit_width(
	IN EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* info
)
{
	EFI_PIXEL_BITMASK* mask; /*位标记*/
	mask=&info->PixelInformation;

	switch(info->PixelFormat)
	{
		case PixelRedGreenBlueReserved8BitPerColor:
		case PixelBlueGreenRedReserved8BitPerColor:
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
 * @param gop 当前使用GOP的句柄。
 * @param hres 水平分辨率。
 * @param vres 竖直分辨率。
 * @retval EFI_NOT_FOUND 未找到有效EDID。
 * @retval EFI_SUCCESS 成功设置边界。
 */
static
EFI_STATUS
EFIAPI
aos_get_edid_info(
	IN EFI_HANDLE gop,
	OUT UINT32* hres,
	OUT UINT32* vres
)
{
	EFI_STATUS status; /*状态*/
	EFI_EDID_ACTIVE_PROTOCOL* edid_active; /*第一优先协议*/
	EFI_EDID_DISCOVERED_PROTOCOL* edid_discovered; /*第二优先协议*/
	AOS_EDID* edid; /*EDID*/
	UINT8 check; /*检查值*/
	UINT8* edid_value; /*值检查用指针*/
	UINT8 index; /*值检查用索引*/

	/*第一步，获取*/
	status=gBS->OpenProtocol(
		gop,
		&gEfiEdidActiveProtocolGuid,
		(VOID**)&edid_active,
		gImageHandle,
		NULL,
		EFI_OPEN_PROTOCOL_GET_PROTOCOL
	);
	if(EFI_ERROR(status)||edid_active->SizeOfEdid==0)
	{
		/*不支持或者EDID大小为0*/
		status=gBS->OpenProtocol(
			gop,
			&gEfiEdidDiscoveredProtocolGuid,
			(VOID**)&edid_discovered,
			gImageHandle,
			NULL,
			EFI_OPEN_PROTOCOL_GET_PROTOCOL
		);
		if(EFI_ERROR(status)||edid_discovered->SizeOfEdid==0)
		{
			/*都不支持*/
			return EFI_UNSUPPORTED;
		}
		else
		{
			edid=(AOS_EDID*)edid_discovered->Edid;
		}
	}
	else
	{
		edid=(AOS_EDID*)edid_active->Edid;
	}

	/*第二部，校验*/
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
	*hres=edid->detailed_timings[0].horizontal_active_low|
		(((UINT32)(edid->detailed_timings[0].horizontal_high&0xF0))<<4);
	*vres=edid->detailed_timings[0].vertical_active_low|
		(((UINT32)(edid->detailed_timings[0].vertical_active_low&0xF0))<<4);
	return EFI_SUCCESS;
}