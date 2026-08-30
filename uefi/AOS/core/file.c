/**
 * 模块文件管理。
 * @date 2026-08-05
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DevicePathLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/BlockIo.h>
#include <Protocol/DevicePath.h>
#include <Protocol/LoadedImage.h>

#include "file.h"
#include "panic.h"
#include "pool.h"

/**
 * AOS系统卷文件结构。
 */
struct _asv_file
{
    UINT8 type;                   /*文件类型。*/
    union
    {
        EFI_FILE_HANDLE efi_file; /*EFI文件句柄。*/
    } handle;                     /*文件系统根句柄。*/
};

/**
 * EFI文件类型。
 */
#define FILE_TYPE_EFI 1

/**
 * EFI系统分区设备句柄。
 */
STATIC EFI_HANDLE esp=NULL;

/**
 * EFI系统分区根路径。
 */
STATIC EFI_FILE_HANDLE esp_root=NULL;

/**
 * EFI系统分区挂载状态。
 */
STATIC BOOLEAN esp_root_mount=FALSE;

/**
 * AOS系统卷对应设备句柄。
 */
STATIC EFI_HANDLE asv_device=NULL;

/**
 * AOS系统卷文件系统根。
 */
STATIC asv_file asv_root;

/**
 * AOS系统卷挂载状态。
 */
STATIC BOOLEAN asv_root_mount=FALSE;

/**
 * 挂载AOS系统卷。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI (*asv_mount_function)(VOID)=NULL;

/**
 * 卸载AOS系统卷。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI (*asv_unmount_function)(VOID)=NULL;

/**
 * 打开AOS系统卷内的文件，按照输入的文件路径与模式打开。
 * 
 * @param path 文件路径，要求为相对于分区根路径的路径，支持.和..语法，路径分割符由/表示。
 * 
 * @return 打开成功返回非空指针，否则为空。
 */
STATIC asv_file* EFIAPI (*asv_open_function)(IN CONST CHAR8* path)=NULL;

/**
 * 关闭AOS系统卷文件。
 * 
 * @param file 文件指针。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI (*asv_close_function)(IN asv_file* file)=NULL;

/**
 * 读取AOS系统卷文件数据。指针会最后指向未读的第一个字节。
 * 
 * @param file 文件指针。
 * @param dest 目标数组。
 * @param size 期望读取大小。
 * 
 * @return 实际读取大小。
 */
STATIC UINT64 EFIAPI (*asv_read_function)(IN asv_file* file,OUT VOID* dest,IN UINT64 size)=NULL;

/**
 * 读取AOS系统卷文件指定位置起始的数据。指针会最后指向未读的第一个字节。
 * 
 * @param file   文件指针。
 * @param offset 起始偏移。
 * @param dest   目标数组。
 * @param size   期望读取大小。
 * 
 * @return 实际读取大小。
 */
STATIC UINT64 EFIAPI (*asv_read_at_function)(IN asv_file* file,IN UINT64 offset,OUT VOID* dest,IN UINT64 size)=NULL;

/**
 * 重置AOS系统卷文件指针位置。
 * 
 * @param file 文件指针。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI (*asv_reset_function)(IN asv_file* file)=NULL;

/**
 * 从AOS系统卷文件当前指针位置跳过N个字节。
 * 
 * @param file 文件指针。
 * @param n    跳过字节数。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI (*asv_skip_function)(IN asv_file* file,IN UINT64 n)=NULL;

/**
 * 获取AOS系统卷文件大小。
 * 
 * @param file 文件指针。
 * 
 * @return 文件大小。
 */
STATIC UINT64 EFIAPI (*asv_get_size_function)(IN asv_file* file)=NULL;

/**
 * 初始化EFI系统分区读写。
 * 
 * @param params 启动参数。
 * 
 * @return 无返回值。
 */
VOID EFIAPI init_esp_io(IN OUT aos_boot_params* params)
{
    EFI_LOADED_IMAGE_PROTOCOL* image;
    panic_if_error(gBS->OpenProtocol(gImageHandle,&gEfiLoadedImageProtocolGuid,(VOID**)&image,gImageHandle,NULL,
        EFI_OPEN_PROTOCOL_GET_PROTOCOL),"Failed to open Loaded Image Protocol on the current image handle.");
    esp=image->DeviceHandle;

    EFI_DEVICE_PATH_PROTOCOL* path;
    panic_if_error(gBS->OpenProtocol(esp,&gEfiDevicePathProtocolGuid,(VOID**)&path,gImageHandle,NULL,
        EFI_OPEN_PROTOCOL_GET_PROTOCOL),"Failed to open Device Path Protocol on the EFI System Partition"
        " device handle.");
    UINTN size=GetDevicePathSize(path);
    if(size==0)
    {
        panic(EFI_DEVICE_ERROR,"The EFI System Partition device path has zero size and cannot be used.");
    }
    VOID* buffer=pool_alloc(size);
    if(buffer==NULL)
    {
        panic(EFI_OUT_OF_RESOURCES,"Failed to allocate memory from bootstrap pool for the EFI System"
            " Partition device path.");
    }
    CopyMem(buffer,path,size);
    params->esp=(aos_efi_device_path*)buffer;

    panic_if_error(gBS->OpenProtocol(esp,&gEfiSimpleFileSystemProtocolGuid,NULL,gImageHandle,NULL,
        EFI_OPEN_PROTOCOL_TEST_PROTOCOL),"The EFI System Partition device handle does not support the Simple"
        " File System Protocol.");
}

/**
 * 挂载EFI系统分区。
 * 
 * @return 无返回值。
 */
VOID EFIAPI esp_mount(VOID)
{
    if(esp_root_mount)
    {
        return;
    }

    if(esp==NULL)
    {
        panic(EFI_UNSUPPORTED,"The EFI System Partition has not been initialized.");
    }
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* fs;
    panic_if_error(gBS->OpenProtocol(esp,&gEfiSimpleFileSystemProtocolGuid,(VOID**)&fs,gImageHandle,NULL,
        EFI_OPEN_PROTOCOL_GET_PROTOCOL),"Failed to open Simple File System Protocol on the EFI System"
        " Partition device handle.");
    panic_if_error(fs->OpenVolume(fs,&esp_root),"Failed to open the root directory of the EFI System"
        " Partition.");
    esp_root_mount=TRUE;
}

/**
 * 卸载EFI系统分区。
 * 
 * @return 无返回值。
 */
VOID EFIAPI esp_unmount(VOID)
{
    if(!esp_root_mount)
    {
        return;
    }
    esp_root->Close(esp_root);
    esp_root=NULL;
    esp_root_mount=FALSE;
}

/**
 * 打开EFI系统分区内文件或目录。
 * 
 * @param path  文件路径。
 * @param mode  打开模式。
 * @param attrs 打开属性。
 * 
 * @return 打开成功返回句柄，文件找不到返回空。
 */
EFI_FILE_HANDLE EFIAPI esp_open(IN CHAR16* path,IN UINT64 mode,IN UINT64 attrs)
{
    if(path==NULL||*path==0)
    {
        panic(EFI_INVALID_PARAMETER,"The file path is null or empty.");
    }

    if(!esp_root_mount)
    {
        panic(EFI_UNSUPPORTED,"The EFI System Partition has not been mounted.");
    }

    EFI_FILE_HANDLE result=NULL;
    EFI_STATUS status=esp_root->Open(esp_root,&result,path,mode,attrs);

    if(status==EFI_NOT_FOUND)
    {
        return NULL;
    }
    else if(EFI_ERROR(status))
    {
        panic(status,"Failed to open file on the EFI System Partition.");
    }

    return result;
}

/**
 * 快速获取EFI系统分区内文件的大小。调用者有义务保证参数为文件而非目录。
 * 
 * @param file 句柄。
 * 
 * @return 文件大小。
 */
UINT64 EFIAPI esp_get_size(IN EFI_FILE_HANDLE file)
{
    if(file==NULL)
    {
        panic(EFI_INVALID_PARAMETER,"The file handle is null.");
    }

    UINT64 current;
    panic_if_error(file->GetPosition(file,&current),"Failed to save the current file position.");

    UINT64 result=MAX_UINT64;
    panic_if_error(file->SetPosition(file,MAX_UINT64),"Failed to seek to the end of the file to determine"
        " its size.");
    panic_if_error(file->GetPosition(file,&result),"Failed to read the file size from the end position.");
    panic_if_error(file->SetPosition(file,current),"Failed to restore the original file position.");
    return result;
}

/**
 * 获取8位字符串转码为16位字符串所需大小。
 * 
 * @param s 8位字符串。
 * 
 * @return 16位字符串所需空间大小，无效编码返回0。
 */
STATIC UINTN file_char16_required8(IN CONST CHAR8* s)
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
STATIC UINTN file_convert16_char8(OUT CHAR16* dest,IN CONST CHAR8* src,IN UINTN size)
{
    if(src==NULL||dest==NULL)
    {
        return 0;
    }
    UINTN expect=file_char16_required8(src);
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
            dest[result++]=(CHAR16)(0xD800|(cp>>10));
            dest[result++]=(CHAR16)(0xDC00|(cp&0x3FF));
        }
    }
    dest[result++]=0;
    return result<<1;
}

/**
 * 挂载EFI文件下的AOS系统卷。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI file_efi_file_mount(VOID)
{
    if(asv_root_mount)
    {
        return;
    }

    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* fs;
    panic_if_error(gBS->OpenProtocol(asv_device,&gEfiSimpleFileSystemProtocolGuid,(VOID**)&fs,gImageHandle,NULL,
        EFI_OPEN_PROTOCOL_GET_PROTOCOL),"Failed to open Simple File System Protocol on the AOS System"
        " Volume device handle.");
    panic_if_error(fs->OpenVolume(fs,&asv_root.handle.efi_file),"Failed to open the root directory of the AOS"
        " System Volume.");
    asv_root.type=FILE_TYPE_EFI;
    asv_root_mount=TRUE;
}

/**
 * 卸载EFI文件下的AOS系统卷。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI file_efi_file_unmount(VOID)
{
    if(!asv_root_mount)
    {
        return;
    }
    if(asv_root.type!=FILE_TYPE_EFI)
    {
        panic(EFI_DEVICE_ERROR,"The ASV File Type has been changed.");
    }
    asv_root.type=0;
    asv_root.handle.efi_file->Close(asv_root.handle.efi_file);
    asv_root.handle.efi_file=NULL;
    asv_root_mount=FALSE;
}

/**
 * 打开EFI文件下的AOS系统卷内的文件，按照输入的文件路径与模式打开。
 * 
 * @param path 文件路径，要求为相对于分区根路径的路径，支持.和..语法，路径分割符由/表示。
 * 
 * @return 打开成功返回非空指针，否则为空。
 */
STATIC asv_file* EFIAPI file_efi_file_open(IN CONST CHAR8* path)
{
    if(path==NULL||*path==0)
    {
        panic(EFI_INVALID_PARAMETER,"The file path is null or empty.");
    }
    else if(!asv_root_mount)
    {
        panic(EFI_UNSUPPORTED,"The AOS System Volume has not been mounted.");
    }

    /*将8位字符串转换成16位字符串，这里按UTF-8解释*/
    UINTN buffer_size=file_char16_required8(path);
    if(buffer_size<=2)
    {
        panic(EFI_INVALID_PARAMETER,"The path is not a possible file path.");
    }

    CHAR16* buffer=(CHAR16*)pool_alloc(buffer_size);
    if(buffer==NULL)
    {
        panic(EFI_OUT_OF_RESOURCES,"Failed to allocate memory from bootstrap pool.");
    }
    file_convert16_char8(buffer,path,buffer_size);

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
        panic(EFI_INVALID_PARAMETER,"The path is a directory path.");
    }

    asv_file* file=(asv_file*)pool_alloc(sizeof(asv_file));
    if(file==NULL)
    {
        panic(EFI_OUT_OF_RESOURCES,"Failed to allocate memory from bootstrap pool.");
    }
    EFI_STATUS status=asv_root.handle.efi_file->Open(asv_root.handle.efi_file,&file->handle.efi_file,buffer,
        EFI_FILE_MODE_READ,0);
    pool_free(buffer);
    if(status==EFI_NOT_FOUND)
    {
        pool_free(file);
        return NULL;
    }
    else if(EFI_ERROR(status))
    {
        panic(status,"Failed to open file on the AOS System Volume.");
    }

    if(FileHandleIsDirectory(file->handle.efi_file)!=EFI_NOT_FOUND)
    {
        panic(EFI_INVALID_PARAMETER,"The path is not a file.");
    }

    file->type=FILE_TYPE_EFI;
    return file;
}

/**
 * 关闭EFI文件下的AOS系统卷文件。
 * 
 * @param file 文件指针。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI file_efi_file_close(IN asv_file* file)
{
    if(file!=NULL)
    {
        if(file->type!=FILE_TYPE_EFI)
        {
            panic(EFI_DEVICE_ERROR,"The ASV File Type has been changed.");
        }
        file->handle.efi_file->Close(file->handle.efi_file);
        pool_free(file);
    }
}

/**
 * 读取EFI文件下的AOS系统卷文件数据。指针会最后指向未读的第一个字节。
 * 
 * @param file 文件指针。
 * @param dest 目标数组。
 * @param size 期望读取大小。
 * 
 * @return 实际读取大小。
 */
STATIC UINT64 EFIAPI file_efi_file_read(IN asv_file* file,OUT VOID* dest,IN UINT64 size)
{
    if(file==NULL)
    {
        panic(EFI_INVALID_PARAMETER,"The ASV File is null.");
    }
    if(dest==NULL)
    {
        panic(EFI_INVALID_PARAMETER,"The destination buffer is null.");
    }
    if(file->type!=FILE_TYPE_EFI)
    {
        panic(EFI_DEVICE_ERROR,"The ASV File Type has been changed.");
    }
    UINTN result=size;
    panic_if_error(file->handle.efi_file->Read(file->handle.efi_file,&result,dest),"Failed to read the file.");
    return result;
}

/**
 * 读取EFI文件下的AOS系统卷文件指定位置起始的数据。指针会最后指向未读的第一个字节。
 * 
 * @param file   文件指针。
 * @param offset 起始偏移。
 * @param dest   目标数组。
 * @param size   期望读取大小。
 * 
 * @return 实际读取大小。
 */
STATIC UINT64 EFIAPI file_efi_file_read_at(IN asv_file* file,IN UINT64 offset,OUT VOID* dest,IN UINT64 size)
{
    if(file==NULL)
    {
        panic(EFI_INVALID_PARAMETER,"The ASV File is null.");
    }
    if(dest==NULL)
    {
        panic(EFI_INVALID_PARAMETER,"The destination buffer is null.");
    }
    if(file->type!=FILE_TYPE_EFI)
    {
        panic(EFI_DEVICE_ERROR,"The ASV File Type has been changed.");
    }
    panic_if_error(file->handle.efi_file->SetPosition(file->handle.efi_file,offset),"Failed to set the file"
        " offset.");
    UINTN result=size;
    panic_if_error(file->handle.efi_file->Read(file->handle.efi_file,&result,dest),"Failed to read the file.");
    return result;
}

/**
 * 重置EFI文件下的AOS系统卷文件指针位置。
 * 
 * @param file 文件指针。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI file_efi_file_reset(IN asv_file* file)
{
    if(file==NULL)
    {
        panic(EFI_INVALID_PARAMETER,"The ASV File is null.");
    }
    if(file->type!=FILE_TYPE_EFI)
    {
        panic(EFI_DEVICE_ERROR,"The ASV File Type has been changed.");
    }
    panic_if_error(file->handle.efi_file->SetPosition(file->handle.efi_file,0),"Failed to reset the file"
        " position.");
}

/**
 * 从EFI文件下的AOS系统卷文件当前指针位置跳过N个字节。
 * 
 * @param file 文件指针。
 * @param n    跳过字节数。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI file_efi_file_skip(IN asv_file* file,IN UINT64 n)
{
    if(file==NULL)
    {
        panic(EFI_INVALID_PARAMETER,"The ASV File is null.");
    }
    if(file->type!=FILE_TYPE_EFI)
    {
        panic(EFI_DEVICE_ERROR,"The ASV File Type has been changed.");
    }
    UINT64 current;
    panic_if_error(file->handle.efi_file->GetPosition(file->handle.efi_file,&current),"Failed to get the"
        " current file position.");
    panic_if_error(file->handle.efi_file->SetPosition(file->handle.efi_file,current+n),"Failed to set the file"
        " offset.");
}

/**
 * 获取AOS系统卷文件大小。
 * 
 * @param file 文件指针。
 * 
 * @return 文件大小。
 */
STATIC UINT64 EFIAPI file_efi_file_get_size(IN asv_file* file)
{
    if(file==NULL)
    {
        panic(EFI_INVALID_PARAMETER,"The ASV File is null.");
    }
    if(file->type!=FILE_TYPE_EFI)
    {
        panic(EFI_DEVICE_ERROR,"The ASV File Type has been changed.");
    }

    UINT64 current;
    panic_if_error(file->handle.efi_file->GetPosition(file->handle.efi_file,&current),"Failed to save the"
        " current file position.");

    UINT64 result=MAX_UINT64;
    panic_if_error(file->handle.efi_file->SetPosition(file->handle.efi_file,MAX_UINT64),"Failed to seek to the"
        " end of the file to determine its size.");
    panic_if_error(file->handle.efi_file->GetPosition(file->handle.efi_file,&result),"Failed to read the file"
        " size from the end position.");
    panic_if_error(file->handle.efi_file->SetPosition(file->handle.efi_file,current),"Failed to restore the"
        " original file position.");
    return result;
}

/**
 * 挂载AOS系统卷。
 * 
 * @return 无返回值。
 */
VOID EFIAPI asv_mount(VOID)
{
    if(asv_mount_function==NULL)
    {
        panic(EFI_UNSUPPORTED,"The AOS System Volume has not been initialized.");
    }
    asv_mount_function();
}

/**
 * 卸载AOS系统卷。
 * 
 * @return 无返回值。
 */
VOID EFIAPI asv_unmount(VOID)
{
    if(asv_unmount_function==NULL)
    {
        panic(EFI_UNSUPPORTED,"The AOS System Volume has not been initialized.");
    }
    asv_unmount_function();
}

/**
 * 打开AOS系统卷内的文件，按照输入的文件路径与模式打开。
 * 
 * @param path 文件路径，要求为相对于分区根路径的路径，支持.和..语法，路径分割符由/表示。
 * 
 * @return 打开成功返回非空指针，否则为空。
 */
asv_file* EFIAPI asv_open(IN CONST CHAR8* path)
{
    if(asv_open_function==NULL)
    {
        panic(EFI_UNSUPPORTED,"The AOS System Volume has not been initialized.");
    }
    return asv_open_function(path);
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
    if(asv_close_function==NULL)
    {
        panic(EFI_UNSUPPORTED,"The AOS System Volume has not been initialized.");
    }
    asv_close_function(file);
}

/**
 * 读取AOS系统卷文件数据。指针会最后指向未读的第一个字节。
 * 
 * @param file 文件指针。
 * @param dest 目标数组。
 * @param size 期望读取大小。
 * 
 * @return 实际读取大小。
 */
UINT64 EFIAPI asv_read(IN asv_file* file,OUT VOID* dest,IN UINT64 size)
{
    if(asv_read_function==NULL)
    {
        panic(EFI_UNSUPPORTED,"The AOS System Volume has not been initialized.");
    }
    return asv_read_function(file,dest,size);
}

/**
 * 读取AOS系统卷文件指定位置起始的数据。指针会最后指向未读的第一个字节。
 * 
 * @param file   文件指针。
 * @param offset 起始偏移。
 * @param dest   目标数组。
 * @param size   期望读取大小。
 * 
 * @return 实际读取大小。
 */
UINT64 EFIAPI asv_read_at(IN asv_file* file,IN UINT64 offset,OUT VOID* dest,IN UINT64 size)
{
    if(asv_read_at_function==NULL)
    {
        panic(EFI_UNSUPPORTED,"The AOS System Volume has not been initialized.");
    }
    return asv_read_at_function(file,offset,dest,size);
}

/**
 * 重置AOS系统卷文件指针位置。
 * 
 * @param file 文件指针。
 * 
 * @return 无返回值。
 */
VOID EFIAPI asv_reset(IN asv_file* file)
{
    if(asv_reset_function==NULL)
    {
        panic(EFI_UNSUPPORTED,"The AOS System Volume has not been initialized.");
    }
    asv_reset_function(file);
}

/**
 * 从AOS系统卷文件当前指针位置跳过N个字节。
 * 
 * @param file 文件指针。
 * @param n    跳过字节数。
 * 
 * @return 无返回值。
 */
VOID EFIAPI asv_skip(IN asv_file* file,IN UINT64 n)
{
    if(asv_skip_function==NULL)
    {
        panic(EFI_UNSUPPORTED,"The AOS System Volume has not been initialized.");
    }
    asv_skip_function(file,n);
}

/**
 * 获取AOS系统卷文件大小。
 * 
 * @param file 文件指针。
 * 
 * @return 文件大小。
 */
UINT64 EFIAPI asv_get_size(IN asv_file* file)
{
    if(asv_get_size_function==NULL)
    {
        panic(EFI_UNSUPPORTED,"The AOS System Volume has not been initialized.");
    }
    return asv_get_size_function(file);
}

/**
 * 初始化AOS系统卷读写。
 * 
 * @param type   AOS系统卷类型。
 * @param info   设备识别信息。
 * @param params 启动参数。
 * 
 * @return 无返回值。
 */
VOID EFIAPI init_asv_io(IN asv_type type,IN VOID* info,IN OUT aos_boot_params* params)
{
    if(type>=ASV_MAX||(type>ASV_ESP&&info==NULL))
    {
        panic(EFI_INVALID_PARAMETER,"The input parameters do not support AOS System Volume"
            " initialization.");
    }

    if(type==ASV_ESP)
    {
        asv_device=esp;
        params->asv=params->esp;
    }
    else
    {
        /*按GPT分区GUID在系统内查找对应硬盘分区设备。*/
        UINTN count=0;
        EFI_HANDLE* handles=NULL;
        EFI_STATUS status=gBS->LocateHandle(ByProtocol,&gEfiDevicePathProtocolGuid,NULL,&count,handles);
        if(status==EFI_BUFFER_TOO_SMALL)
        {
            if(count==0)
            {
                panic(EFI_NOT_FOUND,"No device handles exposing the Device Path Protocol were found.");
            }
            handles=(EFI_HANDLE*)pool_alloc(count);
            if(handles==NULL)
            {
                panic(EFI_OUT_OF_RESOURCES,"Failed to allocate memory from bootstrap pool for device"
                    " handles.");
            }
            panic_if_error(gBS->LocateHandle(ByProtocol,&gEfiDevicePathProtocolGuid,NULL,&count,handles),
                "Failed to locate device handles by Device Path Protocol.");
            count/=sizeof(EFI_HANDLE);
        }
        else
        {
            panic(status,"Failed to locate device handles by Device Path Protocol.");
        }

        EFI_GUID* guid=(EFI_GUID*)info;
        asv_device=NULL;
        for(UINTN index=0;index<count;index++)
        {
            EFI_DEVICE_PATH_PROTOCOL* device_path=DevicePathFromHandle(handles[index]);
            if(device_path==NULL||IsDevicePathEnd(device_path))
            {
                continue;
            }

            /*取设备路径最后一个节点，判断是否为GPT硬盘分区节点。*/
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
                if(hd->MBRType==MBR_TYPE_EFI_PARTITION_TABLE_HEADER&&hd->SignatureType==SIGNATURE_TYPE_GUID&&
                    CompareGuid(guid,(EFI_GUID*)&hd->Signature))
                {
                    asv_device=handles[index];

                    UINTN asvdp_size=GetDevicePathSize(device_path);
                    VOID* asvdp=pool_alloc(asvdp_size);
                    if(asvdp==NULL)
                    {
                        panic(EFI_OUT_OF_RESOURCES,"Failed to allocate memory from bootstrap pool for"
                            " the AOS System Volume device path.");
                    }
                    CopyMem(asvdp,device_path,asvdp_size);
                    params->asv=(aos_efi_device_path*)asvdp;
                    break;
                }
            }
        }
        pool_free(handles);

        if(asv_device==NULL)
        {
            asv_device=esp;
            params->asv=params->esp;
        }
    }

    panic_if_error(gBS->OpenProtocol(asv_device,&gEfiBlockIoProtocolGuid,NULL,gImageHandle,NULL,
        EFI_OPEN_PROTOCOL_TEST_PROTOCOL),"The specified AOS System Volume device is not an available block"
        " device.");
    panic_if_error(gBS->OpenProtocol(asv_device,&gEfiSimpleFileSystemProtocolGuid,NULL,gImageHandle,NULL,
        EFI_OPEN_PROTOCOL_TEST_PROTOCOL),"No filesystem driver was found for the specified AOS System Volume "
        "device.");

    asv_mount_function=file_efi_file_mount;
    asv_unmount_function=file_efi_file_unmount;
    asv_open_function=file_efi_file_open;
    asv_close_function=file_efi_file_close;
    asv_read_function=file_efi_file_read;
    asv_read_at_function=file_efi_file_read_at;
    asv_reset_function=file_efi_file_reset;
    asv_skip_function=file_efi_file_skip;
    asv_get_size_function=file_efi_file_get_size;
    asv_root.handle.efi_file=NULL;
}