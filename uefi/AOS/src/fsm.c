/**
 * 模块文件管理。
 * @date 2025-12-12
 * 
 * Copyright (c) 2025-2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include "fsmi.h"

/**
 * EFI系统分区设备句柄。
 */
STATIC EFI_HANDLE esp=NULL;

/**
 * EFI系统分区根路径。
 */
STATIC EFI_FILE_HANDLE esp_root=NULL;

/**
 * 初始化文件系统管理功能。
 * 
 * @param params 启动参数。
 * 
 * @return 一定成功。
 */
EFI_STATUS EFIAPI fsm_init(IN OUT aos_boot_params* params)
{
    EFI_LOADED_IMAGE_PROTOCOL* image;
    EFI_STATUS status=gBS->OpenProtocol(gImageHandle,&gEfiLoadedImageProtocolGuid,(VOID**)&image,gImageHandle,NULL,
        EFI_OPEN_PROTOCOL_GET_PROTOCOL);
    ASSERT(status==EFI_SUCCESS);
    esp=image->DeviceHandle;
    EFI_DEVICE_PATH_PROTOCOL* path;
    status=gBS->OpenProtocol(esp,&gEfiDevicePathProtocolGuid,(VOID**)&path,gImageHandle,NULL,
        EFI_OPEN_PROTOCOL_GET_PROTOCOL);
    ASSERT(status==EFI_SUCCESS);
    UINTN size=GetDevicePathSize(path);
    ASSERT(size>0);
    VOID* buffer=umalloc(size);
    ASSERT(buffer!=NULL);
    CopyMem(buffer,path,size);
    params->esp=(aos_efi_device_path*)buffer;

    status=gBS->OpenProtocol(esp,&gEfiSimpleFileSystemProtocolGuid,NULL,gImageHandle,NULL,
        EFI_OPEN_PROTOCOL_TEST_PROTOCOL);
    ASSERT(status==EFI_SUCCESS);

    return EFI_SUCCESS;
}

/**
 * 挂载EFI系统分区。
 * 
 * @return 无返回值。
 */
VOID EFIAPI esp_mount()
{
    if(esp_root!=NULL)
    {
        return;
    }

    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* fs;
    EFI_STATUS status=gBS->OpenProtocol(esp,&gEfiSimpleFileSystemProtocolGuid,(VOID**)&fs,gImageHandle,NULL,
        EFI_OPEN_PROTOCOL_GET_PROTOCOL);
    ASSERT(status==EFI_SUCCESS);
    status=fs->OpenVolume(fs,&esp_root);
    if(EFI_ERROR(status))
    {
        /*在挂载EFI系统分区时遇到问题*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.fsm] "
            "An issue occurred while mounting the EFI System Partition. EFI status:%r.\n",status));
    }
}

/**
 * 卸载EFI系统分区。
 * 
 * @return 无返回值。
 */
VOID EFIAPI esp_unmount()
{
    if(esp_root==NULL)
    {
        return;
    }

    esp_root->Close(esp_root);
    esp_root=NULL;
}

/**
 * 打开EFI系统分区内文件或目录。
 * 
 * @param path  文件路径。
 * @param mode  打开模式。
 * @param attrs 打开属性。
 * 
 * @return 打开成功返回句柄，文件找不到和其它错误返回空。其它错误会打印调试输出。
 */
EFI_FILE_HANDLE EFIAPI esp_open(IN CHAR16* path,IN UINT64 mode,IN UINT64 attrs)
{
    ASSERT(path!=NULL&&*path!=0);

    if(esp_root==NULL)
    {
        /*EFI系统分区未挂载*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.fsm] The EFI System Partition is not mounted.\n"));
        return NULL;
    }

    EFI_FILE_HANDLE result=NULL;
    EFI_STATUS status=esp_root->Open(esp_root,&result,path,mode,attrs);

    if(status==EFI_NOT_FOUND)
    {
        return NULL;
    }
    else if(EFI_ERROR(status))
    {
        /*打开文件失败*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.fsm] Failed to open the file. EFI status:%r.\n",status));
        return NULL;
    }

    return result;
}

/**
 * 快速获取EFI系统分区内文件的大小。调用者有义务保证参数为文件而非目录。
 * 
 * @param file 句柄。
 * 
 * @return 文件大小。出现问题返回最大值。
 */
UINT64 EFIAPI esp_get_size(IN EFI_FILE_HANDLE file)
{
    ASSERT(file!=NULL);

    UINT64 current;
    EFI_STATUS status=file->GetPosition(file,&current);
    if(EFI_ERROR(status))
    {
        /*获取文件指针位置失败*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.fsm] Failed to get the file position. EFI status:%r.\n",
            status));
        return MAX_UINT64;
    }

    UINT64 result=MAX_UINT64;
    status=file->SetPosition(file,MAX_UINT64);
    if(EFI_ERROR(status))
    {
        /*设置文件指针位置失败*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.fsm] Failed to set the file position. EFI status:%r.\n",
            status));
        return MAX_UINT64;
    }

    status=file->GetPosition(file,&result);
    if(EFI_ERROR(status))
    {
        /*获取文件指针位置失败*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.fsm] Failed to get the file position. EFI status:%r.\n",
            status));
        return MAX_UINT64;
    }

    status=file->SetPosition(file,current);
    if(EFI_ERROR(status))
    {
        /*设置文件指针位置失败*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.fsm] Failed to set the file position. EFI status:%r.\n",
            status));
        return MAX_UINT64;
    }
    return result;
}

/**
 * AOS系统卷对应设备句柄。
 */
STATIC EFI_HANDLE asv_device=NULL;

/**
 * AOS系统卷文件系统根。
 */
STATIC asv_file asv_root;

/**
 * 挂载AOS系统卷。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI (*asv_mount_function)()=NULL;

/**
 * 卸载AOS系统卷。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI (*asv_unmount_function)()=NULL;

/**
 * 打开AOS系统卷内的文件，按照输入的文件路径与模式打开。
 * 
 * @param path 文件路径，要求为相对于分区根路径的路径，支持.和..语法，路径分割符由/表示。
 * @param mode 打开模式。
 * 
 * @return 打开成功返回非空指针，否则为空。
 */
STATIC asv_file* EFIAPI (*asv_open_function)(IN CONST CHAR8* path,IN UINT64 mode)=NULL;

/**
 * 关闭AOS系统卷文件。
 * 
 * @param file 文件指针。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI (*asv_close_function)(IN asv_file* file)=NULL;

/**
 * 读取AOS系统卷文件数据。
 * 
 * @param file 文件指针。
 * @param dest 目标数组。
 * @param size 期望读取大小。
 * 
 * @return 实际读取大小。
 */
STATIC UINT64 EFIAPI (*asv_read_function)(IN asv_file* file,OUT VOID* dest,IN UINT64 size)=NULL;

/**
 * 写入AOS系统卷文件数据。
 * 
 * @param file 文件指针。
 * @param dest 源数组。
 * @param size 期望写入大小。
 * 
 * @return 实际写入大小。
 */
STATIC UINT64 EFIAPI (*asv_write_function)(IN asv_file* file,IN VOID* src,IN UINT64 size)=NULL;

/**
 * 对AOS系统卷文件指针调整指针位置。偏移量的容许范围是最终结果在0到文件大小上限之间，未写入过的区域自动按0填充。
 * 
 * @param file   文件指针。
 * @param offset 偏移量。
 * @param ref    位置参考。
 * 
 * @return 调整位置状态。
 */
STATIC EFI_STATUS EFIAPI (*asv_reposition_function)(IN asv_file* file,INT64 offset,position_reference ref)=NULL;

/**
 * 获取AOS系统卷文件指针当前指针位置。
 * 
 * @param file 文件指针。
 * @param pos  位置指针。
 * 
 * @return 获取位置状态。
 */
STATIC EFI_STATUS EFIAPI (*asv_get_position_function)(IN asv_file* file,UINT64* pos)=NULL;

/**
 * 获取AOS系统卷文件大小。
 * 
 * @param file 文件指针。
 * @param size 文件大小指针。
 * 
 * @return 获取文件大小状态。
 */
STATIC EFI_STATUS EFIAPI (*asv_get_size_function)(IN asv_file* file,OUT UINT64* size)=NULL;

/**
 * 挂载简单文件协议下的AOS系统卷。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI fsm_simple_file_mount()
{
    if(asv_root.handle.simple_file!=NULL)
    {
        return;
    }

    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* fs;
    EFI_STATUS status=gBS->OpenProtocol(asv_device,&gEfiSimpleFileSystemProtocolGuid,(VOID**)&fs,gImageHandle,NULL,
        EFI_OPEN_PROTOCOL_GET_PROTOCOL);
    ASSERT(status==EFI_SUCCESS);
    status=fs->OpenVolume(fs,&asv_root.handle.simple_file);
    if(EFI_ERROR(status))
    {
        /*在挂载AOS系统卷时发生问题*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.fsm] An issue occurred while mounting the AOS System Volume. "
            "EFI status:%r.\n",status));
    }
}

/**
 * 卸载简单文件协议下的AOS系统卷。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI fsm_simple_file_unmount()
{
    if(asv_root.handle.simple_file==NULL)
    {
        return;
    }

    asv_root.handle.simple_file->Close(asv_root.handle.simple_file);
    asv_root.handle.simple_file=NULL;
}

/**
 * 获取8位字符串转码为16位字符串所需大小。
 * 
 * @param s 8位字符串。
 * 
 * @return 16位字符串所需空间大小，无效编码返回0。
 */
STATIC UINTN fsm_char16_required8(IN CONST CHAR8* s)
{
    if(s==NULL)
    {
        return 0;
    }
    UINTN index=0;
    UINTN result=2;
    while(s[index]!=0)
    {
        if((s[index]&0x80)==0)
        {
            result+=2;
            index++;
        }
        else if((s[index]&0xE0)==0xC0)
        {
            if(s[index]>0xC1&&(s[index+1]&0xC0)==0x80)
            {
                result+=2;
                index+=2;
            }
            else 
            {
                return 0;            
            }
        }
        else if((s[index]&0xF0)==0xE0)
        {
            if((s[index+1]&0xC0)==0x80&&(s[index+2]&0xC0)==0x80&&!(s[index]==0xE0&&s[index+1]<0xA0)&&
                !(s[index]==0xED&&s[index+1]>0x9F))
            {
                result+=2;
                index+=3;
            }
            else 
            {
                return 0;            
            }
        }
        else if((s[index]&0xF8)==0xF0)
        {
            if(s[index]<0xF5&&(s[index+1]&0xC0)==0x80&&(s[index+2]&0xC0)==0x80&&(s[index+3]&0xC0)==0x80&&
                !(s[index]==0xF0&&s[index+1]<0x90)&&!(s[index]==0xF4&&s[index+1]>0x8F))
            {
                result+=4;
                index+=4;
            }
            else 
            {
                return 0;            
            }
        }
        else
        {
            return 0;
        }
    }
    return result;
}

/**
 * 将8位字符串转换成16位字符串。
 * 
 * @param dest 目标内存空间。
 * @param src  8位字符串。
 * @param size 目标字符串预留空间大小。
 * 
 * @return 已使用空间大小，当预留大小不足时返回期望大小，无效编码返回0。
 */
STATIC UINTN fsm_convert16_char8(CHAR16* dest,CONST CHAR8* src,UINTN size)
{
    if(src==NULL||dest==NULL)
    {
        return 0;
    }
    UINTN expect=fsm_char16_required8(src);
    if(expect==0||expect>size)
    {
        return expect;
    }
    UINTN index=0;
    UINTN result=0;
    while(src[index]!=0)
    {
        if((src[index]&0x80)==0)
        {
            dest[result++]=src[index++];
        }
        else if((src[index]&0xE0)==0xC0)
        {
            dest[result]=((CHAR16)(src[index++]&0x1F))<<6;
            dest[result++]|=src[index++]&0x3F;
        }
        else if((src[index]&0xF0)==0xE0)
        {
            dest[result]=((CHAR16)(src[index++]&0xF))<<12;
            dest[result]|=((CHAR16)(src[index++]&0x3F))<<6;
            dest[result++]|=src[index++]&0x3F;
        }
        else
        {
            UINT32 cp=((UINT32)(src[index++]&0x7))<<18;
            cp|=((UINT32)(src[index++]&0x3F))<<12;
            cp|=((UINT32)(src[index++]&0x3F))<<6;
            cp|=src[index++]&0x3F;
            cp-=0x10000;
            dest[result++]=(CHAR16)(0xDB00|(cp>>10));
            dest[result++]=(CHAR16)(0xDC00|(cp&0x3FF));
        }
    }
    dest[result++]=0;
    return result<<1;
}

/**
 * 打开简单文件协议下的AOS系统卷内的文件，按照输入的文件路径与模式打开。
 * 
 * @param path 文件路径，要求为相对于分区根路径的路径，支持.和..语法，路径分割符由/表示。
 * @param mode 打开模式。
 * 
 * @return 打开成功返回非空指针，否则为空。
 */
STATIC asv_file* EFIAPI fsm_simple_file_open(IN CONST CHAR8* path,IN UINT64 mode)
{
    if(path==NULL||*path==0)
    {
        /*输入参数为空*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.fsm] The input parameter is null.\n"));
        return NULL;
    }
    else if(asv_root.handle.simple_file==NULL)
    {
        /*AOS系统卷未挂载*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.fsm] The AOS System Volume is not mounted.\n"));
        return NULL;
    }

    /*将8位字符串转换成16位字符串，这里按UTF-8解释*/
    UINTN buffer_size=fsm_char16_required8(path);
    if(buffer_size<=2)
    {
        /*输入不是一个可能的文件路径*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.fsm] Input is not a possible file path.\n"));
        return NULL;
    }

    CHAR16* buffer=(CHAR16*)umalloc(buffer_size);
    if(buffer==NULL)
    {
        /*引导内存池空间不足*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.fsm] Insufficient space in the boot memory pool.\n"));
        return NULL;
    }
    fsm_convert16_char8(buffer,path,buffer_size);

    buffer=PathCleanUpDirectories(buffer);
    CHAR16* last_item=buffer;
    CHAR16* c=buffer;
    while(*c!=0)
    {
        if(c[0]==L'\\'&&c[1]!=0)
        {
            last_item=&c[1];
        }
        c++;
    }

    if(*buffer==0||*(c-1)==L'\\'||StrCmp(last_item,L".")==0||
        StrCmp(last_item,L"..")==0)
    {
        /*不支持打开目录*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.fsm] Directory opening is not supported.\n"));
        return NULL;
    }

    UINT64 simple_file_mode;
    switch(mode)
    {
        case ASV_OPEN_MODE_READ:
            simple_file_mode=EFI_FILE_MODE_READ;
            break;
        case ASV_OPEN_MODE_WRITE:
            simple_file_mode=EFI_FILE_MODE_READ|EFI_FILE_MODE_WRITE;
            break;
        case ASV_OPEN_MODE_READ|ASV_OPEN_MODE_WRITE:
            simple_file_mode=EFI_FILE_MODE_READ|EFI_FILE_MODE_WRITE;
            break;
        case ASV_OPEN_MODE_CREATE:
            simple_file_mode=EFI_FILE_MODE_READ|EFI_FILE_MODE_WRITE|EFI_FILE_MODE_CREATE;
            break;
        case ASV_OPEN_MODE_READ|ASV_OPEN_MODE_CREATE:
            simple_file_mode=EFI_FILE_MODE_READ|EFI_FILE_MODE_WRITE|EFI_FILE_MODE_CREATE;
            break;
        case ASV_OPEN_MODE_WRITE|ASV_OPEN_MODE_CREATE:
            simple_file_mode=EFI_FILE_MODE_READ|EFI_FILE_MODE_WRITE|EFI_FILE_MODE_CREATE;
            break;
        case ASV_OPEN_MODE_READ|ASV_OPEN_MODE_WRITE|ASV_OPEN_MODE_CREATE:
            simple_file_mode=EFI_FILE_MODE_READ|EFI_FILE_MODE_WRITE|EFI_FILE_MODE_CREATE;
            break;
        default:
            /*打开模式不支持*/
            DEBUG((DEBUG_ERROR,"[aos.uefi.fsm] The open mode is not supported.\n"));
            return NULL;
    }

    asv_file* file=(asv_file*)umalloc(sizeof(asv_file));
    EFI_STATUS status=asv_root.handle.simple_file->Open(asv_root.handle.simple_file,&file->handle.simple_file,buffer,
        simple_file_mode,0);
    ufree(buffer);
    if(EFI_ERROR(status))
    {
        /*打开文件失败*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.fsm] Failed to open the file. EFI status:%r.\n",status));
        return NULL;
    }
    else
    {
        return file;
    }
}

/**
 * 关闭简单文件协议下的AOS系统卷文件。
 * 
 * @param file 文件指针。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI fsm_simple_file_close(IN asv_file* file)
{
    if(file!=NULL&&file->handle.simple_file!=NULL)
    {
        file->handle.simple_file->Close(file->handle.simple_file);
        ufree(file);
    }
}

/**
 * 读取简单文件协议下的AOS系统卷文件数据。
 * 
 * @param file 文件指针。
 * @param dest 目标数组。
 * @param size 期望读取大小。
 * 
 * @return 实际读取大小。
 */
STATIC UINT64 EFIAPI fsm_simple_file_read(IN asv_file* file,OUT VOID* dest,IN UINT64 size)
{
    if(file!=NULL&&file->handle.simple_file!=NULL)
    {
        UINTN result=size;
        EFI_STATUS status=file->handle.simple_file->Read(file->handle.simple_file,&result,dest);
        if(EFI_ERROR(status))
        {
            /*读取文件失败*/
            DEBUG((DEBUG_ERROR,"[aos.uefi.fsm] Failed to read the file. EFI status:%r.\n",
                status));
            return 0;
        }
        else
        {
            return result;
        }
    }
    return 0;
}

/**
 * 写入简单文件协议下的AOS系统卷文件数据。
 * 
 * @param file 文件指针。
 * @param dest 源数组。
 * @param size 期望写入大小。
 * 
 * @return 实际写入大小。
 */
STATIC UINT64 EFIAPI fsm_simple_file_write(IN asv_file* file,IN VOID* src,IN UINT64 size)
{
    if(file!=NULL&&file->handle.simple_file!=NULL)
    {
        UINTN result=size;
        EFI_STATUS status=file->handle.simple_file->Write(file->handle.simple_file,&result,src);
        if(EFI_ERROR(status))
        {
            /*写入文件失败*/
            DEBUG((DEBUG_ERROR,"[aos.uefi.fsm] Failed to write the file. EFI status:%r.\n",
                status));
            return 0;
        }
        else
        {
            return result;
        }
    }
    return 0;
}

/**
 * 对简单文件协议下的AOS系统卷文件指针调整指针位置。偏移量的容许范围是最终结果在0到文件大小上限之间，未写入过的区域自动按0填充。
 * 
 * @param file   文件指针。
 * @param offset 偏移量。
 * @param ref    位置参考。
 * 
 * @return 调整位置状态。
 */
STATIC EFI_STATUS EFIAPI fsm_simple_file_reposition(IN asv_file* file,INT64 offset,position_reference ref)
{
    if(ref>=POSITION_REFERENCE_MAX||file==NULL||file->handle.simple_file==NULL)
    {
        return EFI_UNSUPPORTED;
    }

    UINT64 pos=0;
    EFI_STATUS status=file->handle.simple_file->GetPosition(file->handle.simple_file,&pos);
    if(EFI_ERROR(status))
    {
        /*获取文件指针位置失败*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.fsm] Failed to get the position of the file. "
            "EFI status:%r.\n",status));
        return status;
    }
    if(ref==POSITION_START)
    {
        if(offset<0)
        {
            return EFI_UNSUPPORTED;
        }
        status=file->handle.simple_file->SetPosition(file->handle.simple_file,(UINT64)offset);
        if(EFI_ERROR(status))
        {
            /*重定位文件失败*/
            DEBUG((DEBUG_ERROR,"[aos.uefi.fsm] Failed to reposition the file. "
                "EFI status:%r.\n",status));
            file->handle.simple_file->SetPosition(file->handle.simple_file,pos);
        }
    }
    else if(ref==POSITION_END)
    {
        UINT64 size=esp_get_size(file->handle.simple_file);
        if(size==MAX_UINT64)
        {
            /*获取文件大小失败*/
            DEBUG((DEBUG_ERROR,"[aos.uefi.fsm] Failed to get the size of the file.\n"));
            return EFI_DEVICE_ERROR;
        }
        else
        {
            if(offset>=0)
            {
                UINT64 space=MAX_UINT64-1-size;
                if(space>=(UINT64)offset)
                {
                    status=file->handle.simple_file->SetPosition(file->handle.simple_file,size+offset);
                    if(EFI_ERROR(status))
                    {
                        /*重定位文件失败*/
                        DEBUG((DEBUG_ERROR,"[aos.uefi.fsm] Failed to reposition the file. "
                            "EFI status:%r.\n",status));
                        file->handle.simple_file->SetPosition(file->handle.simple_file,pos);
                    }
                }
                else
                {
                    status=EFI_UNSUPPORTED;
                }
            }
            else
            {
                if(size>=(UINT64)ABS(offset))
                {
                    status=file->handle.simple_file->SetPosition(file->handle.simple_file,size-(UINT64)ABS(offset));
                    if(EFI_ERROR(status))
                    {
                        /*重定位文件失败*/
                        DEBUG((DEBUG_ERROR,"[aos.uefi.fsm] Failed to reposition the file. "
                            "EFI status:%r.\n",status));
                        file->handle.simple_file->SetPosition(file->handle.simple_file,pos);
                    }
                }
                else
                {
                    status=EFI_UNSUPPORTED;
                }
            }
        }
    }
    else
    {
        if(offset>=0)
        {
            UINT64 space=MAX_UINT64-1-pos;
            if(space>=(UINT64)offset)
            {
                status=file->handle.simple_file->SetPosition(file->handle.simple_file,pos+offset);
                if(EFI_ERROR(status))
                {
                    /*重定位文件失败*/
                    DEBUG((DEBUG_ERROR,"[aos.uefi.fsm] Failed to reposition the file. "
                        "EFI status:%r.\n",status));
                    file->handle.simple_file->SetPosition(file->handle.simple_file,pos);
                }
            }
            else
            {
                status=EFI_UNSUPPORTED;
            }
        }
        else
        {
            if(pos>=(UINT64)ABS(offset))
            {
                status=file->handle.simple_file->SetPosition(file->handle.simple_file,pos-(UINT64)ABS(offset));
                if(EFI_ERROR(status))
                {
                    /*重定位文件失败*/
                    DEBUG((DEBUG_ERROR,"[aos.uefi.fsm] Failed to reposition the file. "
                        "EFI status:%r.\n",status));
                    file->handle.simple_file->SetPosition(file->handle.simple_file,pos);
                }
            }
            else
            {
                status=EFI_UNSUPPORTED;
            }
        }
    }
    return status;
}

/**
 * 获取简单文件协议下的AOS系统卷文件指针当前指针位置。
 * 
 * @param file 文件指针。
 * @param pos  位置指针。
 * 
 * @return 获取位置状态。
 */
STATIC EFI_STATUS EFIAPI fsm_simple_file_get_position(IN asv_file* file,UINT64* pos)
{
    if(pos==NULL||file==NULL||file->handle.simple_file==NULL)
    {
        return EFI_UNSUPPORTED;
    }
    else
    {
        EFI_STATUS status=file->handle.simple_file->GetPosition(file->handle.simple_file,pos);
        if(EFI_ERROR(status))
        {
            /*获取文件指针位置失败*/
            DEBUG((DEBUG_ERROR,"[aos.uefi.fsm] Failed to get the position of the file. "
                "EFI status:%r.\n",status));
        }
        return status;
    }
}

/**
 * 获取简单文件协议下的AOS系统卷文件大小。
 * 
 * @param file 文件指针。
 * @param size 文件大小指针。
 * 
 * @return 获取文件大小状态。
 */
STATIC EFI_STATUS EFIAPI fsm_simple_file_get_size(IN asv_file* file,OUT UINT64* size)
{
    if(size==NULL||file==NULL||file->handle.simple_file==NULL)
    {
        return EFI_UNSUPPORTED;
    }
    else
    {
        *size=esp_get_size(file->handle.simple_file);
        if(*size==MAX_UINT64)
        {
            /*获取文件大小失败*/
            DEBUG((DEBUG_ERROR,"[aos.uefi.fsm] Failed to get the size of the file.\n"));
            return EFI_DEVICE_ERROR;
        }
        else
        {
            return EFI_SUCCESS;
        }
    }
}

/**
 * 根据输入参数初始化AOS系统卷文件系统管理。
 * 
 * @param type       AOS系统卷类型。
 * @param identifier 识别符，用于找到唯一对应的设备。
 * @param params     启动参数。
 * 
 * @return 设备检测状态。
 */
EFI_STATUS EFIAPI asv_init(IN asv_type type,IN VOID* identifier,OUT aos_boot_params* params)
{
    if(type>=ASV_TYPE_MAX||(type>ASV_ESP&&identifier==NULL))
    {
        /*输入参数不支持初始化*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.fsm] The input parameters do not support initialization.\n"));
        return EFI_UNSUPPORTED;
    }

    EFI_STATUS status;
    if(type==ASV_ESP)
    {
        EFI_HANDLE asvd=esp;
        params->asv=params->esp;

        CHAR16* str=ConvertDevicePathToText(DevicePathFromHandle(esp),FALSE,
            FALSE);
        /*EFI系统分区的设备路径*/
        DEBUG((DEBUG_INFO,"[aos.uefi.fsm] The device path of the EFI System Partition is: %s.\n",
            str));
        FreePool(str);
        str=ConvertDevicePathToText(DevicePathFromHandle(asvd),FALSE,
            FALSE);
        /*AOS系统卷的设备路径*/
        DEBUG((DEBUG_INFO,"[aos.uefi.fsm] The device path of the AOS System Volume is: %s.\n",
            str));
        FreePool(str);
        
        status=gBS->OpenProtocol(asvd,&gEfiBlockIoProtocolGuid,NULL,gImageHandle,NULL,
            EFI_OPEN_PROTOCOL_TEST_PROTOCOL);
        if(EFI_ERROR(status))
        {
            /*指定的AOS系统卷设备不是一个可用的块设备*/
            DEBUG((DEBUG_ERROR,"[aos.uefi.fsm] The specified AOS System Volume device "
                "is not an available block device.\n"));
            return EFI_UNSUPPORTED;
        }

        asv_device=asvd;
        status=gBS->OpenProtocol(asvd,&gEfiSimpleFileSystemProtocolGuid,NULL,gImageHandle,NULL,
            EFI_OPEN_PROTOCOL_TEST_PROTOCOL);
        if(status==EFI_SUCCESS)
        {
            asv_mount_function=fsm_simple_file_mount;
            asv_unmount_function=fsm_simple_file_unmount;
            asv_open_function=fsm_simple_file_open;
            asv_close_function=fsm_simple_file_close;
            asv_read_function=fsm_simple_file_read;
            asv_write_function=fsm_simple_file_write;
            asv_reposition_function=fsm_simple_file_reposition;
            asv_get_position_function=fsm_simple_file_get_position;
            asv_get_size_function=fsm_simple_file_get_size;
            asv_root.handle.simple_file=NULL;
            /*AOS系统卷设备将通过简单文件协议访问*/
            DEBUG((DEBUG_INFO,"[aos.uefi.fsm] The AOS System Volume will be accessed "
                "using the Simple File Protocol.\n"));
            return EFI_SUCCESS;
        }
        else
        {
            /*指定的AOS系统卷设备未找到文件系统驱动*/
            DEBUG((DEBUG_ERROR,"[aos.uefi.fsm] No filesystem driver was found for "
                "the specified AOS System Volume device.\n"));
            return EFI_UNSUPPORTED;
        }
    }
    else if(type==ASV_HARD_DRIVE_GPT)
    {
        UINTN count=0;
        EFI_HANDLE* device_paths=NULL;
        status=gBS->LocateHandle(ByProtocol,&gEfiDevicePathProtocolGuid,NULL,&count,device_paths);
        if(status==EFI_BUFFER_TOO_SMALL)
        {
            ASSERT(count>0);
            device_paths=(EFI_HANDLE*)umalloc(count);
            if(device_paths==NULL)
            {
                /*引导内存池空间不足*/
                DEBUG((DEBUG_ERROR,"[aos.uefi.fsm] Insufficient space in the boot memory pool.\n"));
                return EFI_OUT_OF_RESOURCES;
            }
            status=gBS->LocateHandle(ByProtocol,&gEfiDevicePathProtocolGuid,NULL,&count,device_paths);
            if(EFI_ERROR(status))
            {
                ufree(device_paths);
                return status;
            }
            count/=sizeof(EFI_HANDLE);
        }
        else
        {
            /*设备路径协议未找到*/
            DEBUG((DEBUG_ERROR,"[aos.uefi.fsm] The Device Path Protocol is not found.\n"));
            return EFI_NOT_FOUND;
        }

        EFI_HANDLE asvd=NULL;
        EFI_GUID* guid=(EFI_GUID*)identifier;
        for(UINTN index=0;index<count;index++)
        {
            EFI_DEVICE_PATH_PROTOCOL* device_path=DevicePathFromHandle(device_paths[index]);
            ASSERT(device_path!=NULL&&!IsDevicePathEnd(device_path));
            EFI_DEVICE_PATH_PROTOCOL* last=device_path;
            EFI_DEVICE_PATH_PROTOCOL* next=NextDevicePathNode(last);
            while(!IsDevicePathEnd(next))
            {
                last=next;
                next=NextDevicePathNode(next);
            }
            if(last->Type==MEDIA_DEVICE_PATH&&last->SubType==MEDIA_HARDDRIVE_DP)
            {
                HARDDRIVE_DEVICE_PATH* hd=(HARDDRIVE_DEVICE_PATH*)last;
                if(hd->MBRType==MBR_TYPE_EFI_PARTITION_TABLE_HEADER&&hd->SignatureType==SIGNATURE_TYPE_GUID)
                {
                    if(CompareGuid(guid,(EFI_GUID*)&hd->Signature))
                    {
                        asvd=device_paths[index];

                        UINTN asvdp_size=GetDevicePathSize(device_path);
                        VOID* asvdp=umalloc(asvdp_size);
                        if(asvdp==NULL)
                        {
                            /*引导内存池空间不足*/
                            DEBUG((DEBUG_ERROR,"[aos.uefi.fsm] Insufficient space in the boot "
                                "memory pool.\n"));
                            return EFI_OUT_OF_RESOURCES;
                        }
                        CopyMem(asvdp,device_path,asvdp_size);
                        params->asv=(aos_efi_device_path*)asvdp;

                        break;
                    }
                }
            }
        }
        ufree(device_paths);
        if(asvd==NULL)
        {
            asvd=esp;
            params->asv=params->esp;
        }

        CHAR16* str=ConvertDevicePathToText(DevicePathFromHandle(esp),FALSE,
            FALSE);
        /*EFI系统分区的设备路径*/
        DEBUG((DEBUG_INFO,"[aos.uefi.fsm] The device path of the EFI System Partition is: %s.\n",
            str));
        FreePool(str);
        str=ConvertDevicePathToText(DevicePathFromHandle(asvd),FALSE,
            FALSE);
        /*AOS系统卷的设备路径*/
        DEBUG((DEBUG_INFO,"[aos.uefi.fsm] The device path of the AOS System Volume is: %s.\n",
            str));
        FreePool(str);
        
        status=gBS->OpenProtocol(asvd,&gEfiBlockIoProtocolGuid,NULL,gImageHandle,NULL,
            EFI_OPEN_PROTOCOL_TEST_PROTOCOL);
        if(EFI_ERROR(status))
        {
            /*指定的AOS系统卷设备不是一个可用的块设备*/
            DEBUG((DEBUG_ERROR,"[aos.uefi.fsm] The specified AOS System Volume device "
                "is not an available block device.\n"));
            return EFI_UNSUPPORTED;
        }

        asv_device=asvd;
        status=gBS->OpenProtocol(asvd,&gEfiSimpleFileSystemProtocolGuid,NULL,gImageHandle,NULL,
            EFI_OPEN_PROTOCOL_TEST_PROTOCOL);
        if(status==EFI_SUCCESS)
        {
            asv_mount_function=fsm_simple_file_mount;
            asv_unmount_function=fsm_simple_file_unmount;
            asv_open_function=fsm_simple_file_open;
            asv_close_function=fsm_simple_file_close;
            asv_read_function=fsm_simple_file_read;
            asv_write_function=fsm_simple_file_write;
            asv_reposition_function=fsm_simple_file_reposition;
            asv_get_position_function=fsm_simple_file_get_position;
            asv_get_size_function=fsm_simple_file_get_size;
            asv_root.handle.simple_file=NULL;
            /*AOS系统卷设备将通过简单文件协议访问*/
            DEBUG((DEBUG_INFO,"[aos.uefi.fsm] The AOS System Volume will be accessed "
                "using the Simple File Protocol.\n"));
            return EFI_SUCCESS;
        }
        else
        {
            /*指定的AOS系统卷设备未找到文件系统驱动*/
            DEBUG((DEBUG_ERROR,"[aos.uefi.fsm] No filesystem driver was found for "
                "the specified AOS System Volume device.\n"));
            return EFI_UNSUPPORTED;
        }
    }

    /*到达不可达位置*/
    DEBUG((DEBUG_ERROR,"[aos.uefi.fsm] Reached an unreachable location.\n"));
    UNREACHABLE();
    return EFI_UNSUPPORTED;
}

/**
 * 挂载AOS系统卷。
 * 
 * @return 无返回值。
 */
VOID EFIAPI asv_mount()
{
    asv_mount_function!=NULL?asv_mount_function():NULL;
}

/**
 * 卸载AOS系统卷。
 * 
 * @return 无返回值。
 */
VOID EFIAPI asv_unmount()
{
    asv_unmount_function!=NULL?asv_unmount_function():NULL;
}

/**
 * 打开AOS系统卷内的文件，按照输入的文件路径与模式打开。
 * 
 * @param path 文件路径，要求为相对于分区根路径的路径，支持.和..语法，路径分割符由/表示。
 * @param mode 打开模式。
 * 
 * @return 打开成功返回非空指针，否则为空。
 */
asv_file* EFIAPI asv_open(IN CONST CHAR8* path,IN UINT64 mode)
{
    return asv_open_function!=NULL?asv_open_function(path,mode):NULL;
}

/**
 * 关闭AOS系统卷文件。
 * 
 * @param file 文件指针。
 * 
 * @return 无返回值。
 */
VOID EFIAPI asv_close(IN asv_file* file)
{
    asv_close_function!=NULL?asv_close_function(file):NULL;
}

/**
 * 读取AOS系统卷文件数据。
 * 
 * @param file 文件指针。
 * @param dest 目标数组。
 * @param size 期望读取大小。
 * 
 * @return 实际读取大小。
 */
UINT64 EFIAPI asv_read(IN asv_file* file,OUT VOID* dest,IN UINT64 size)
{
    return asv_read_function!=NULL?asv_read_function(file,dest,size):0;
}

/**
 * 写入AOS系统卷文件数据。
 * 
 * @param file 文件指针。
 * @param dest 源数组。
 * @param size 期望写入大小。
 * 
 * @return 实际写入大小。
 */
UINT64 EFIAPI asv_write(IN asv_file* file,IN VOID* src,IN UINT64 size)
{
    return asv_write_function!=NULL?asv_write_function(file,src,size):0;
}

/**
 * 对AOS系统卷文件指针调整指针位置。偏移量的容许范围是最终结果在0到文件大小上限之间，未写入过的区域自动按0填充。
 * 
 * @param file   文件指针。
 * @param offset 偏移量。
 * @param ref    位置参考。
 * 
 * @return 调整位置状态。
 */
EFI_STATUS EFIAPI asv_reposition(IN asv_file* file,INT64 offset,position_reference ref)
{
    return asv_reposition_function!=NULL?asv_reposition_function(file,offset,ref):EFI_UNSUPPORTED;
}

/**
 * 获取AOS系统卷文件指针当前指针位置。
 * 
 * @param file 文件指针。
 * @param pos  位置指针。
 * 
 * @return 获取位置状态。
 */
EFI_STATUS EFIAPI asv_get_position(IN asv_file* file,UINT64* pos)
{
    return asv_get_position_function!=NULL?asv_get_position_function(file,pos):EFI_UNSUPPORTED;
}

/**
 * 获取AOS系统卷文件大小。
 * 
 * @param file 文件指针。
 * @param size 文件大小指针。
 * 
 * @return 获取文件大小状态。
 */
EFI_STATUS EFIAPI asv_get_size(IN asv_file* file,OUT UINT64* size)
{
    return asv_get_size_function!=NULL?asv_get_size_function(file,size):EFI_UNSUPPORTED;
}