/**
 * 模块文件管理。
 * @date 2025-12-12
 * 
 * Copyright (c) 2025 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include "fsmi.h"

/**
 * ESP设备句柄。
 */
STATIC EFI_HANDLE esp=NULL;

/**
 * ESP根路径。
 */
STATIC EFI_FILE_HANDLE root=NULL;

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
 * 挂载ESP分区。
 * 
 * @return 无返回值。
 */
VOID EFIAPI mount()
{
    if(root!=NULL)
    {
        return;
    }

    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* fs;
    EFI_STATUS state=gBS->OpenProtocol(esp,&gEfiSimpleFileSystemProtocolGuid,(VOID**)&fs,gImageHandle,NULL,
        EFI_OPEN_PROTOCOL_GET_PROTOCOL);
    ASSERT(state==EFI_SUCCESS);
    state=fs->OpenVolume(fs,&root);
    if(EFI_ERROR(state))
    {
        DEBUG((DEBUG_ERROR,"[aos.uefi.fsm] "
            "Problem encountered while mounting the ESP. EFI status code:%lu.\n",state));
    }
}

/**
 * 卸载ESP分区。
 * 
 * @return 无返回值。
 */
VOID EFIAPI umount()
{
    if(root==NULL)
    {
        return;
    }

    root->Close(root);
    root=NULL;
}

/**
 * 打开ESP内文件或目录。
 * 
 * @param path  文件路径。
 * @param mode  打开模式。
 * @param attrs 打开属性。
 * 
 * @return 打开成功返回句柄，文件找不到和其它错误返回空。其它错误会打印调试输出。
 */
EFI_FILE_HANDLE EFIAPI ufopen(IN CHAR16* path,IN UINT64 mode,IN UINT64 attrs)
{
    ASSERT(path!=NULL&&*path!=0);

    if(root==NULL)
    {
        DEBUG((DEBUG_ERROR,"[aos.uefi.fsm] The ESP is not mounted.\n"));
        return NULL;
    }

    EFI_FILE_HANDLE result=NULL;
    EFI_STATUS state=root->Open(root,&result,path,mode,attrs);

    if(state==EFI_NOT_FOUND)
    {
        return NULL;
    }
    else if(EFI_ERROR(state))
    {
        DEBUG((DEBUG_ERROR,"[aos.uefi.fsm] Failed to open the file. EFI status code:%lu.\n",state));
        return NULL;
    }

    return result;
}

/**
 * 快速获取文件的大小。调用者有义务保证参数为文件而非目录。
 * 
 * @param file 句柄。
 * 
 * @return 文件大小。出现问题返回最大值。
 */
UINT64 EFIAPI ufsize(IN EFI_FILE_HANDLE file)
{
    ASSERT(file!=NULL);

    UINT64 current;
    EFI_STATUS state=file->GetPosition(file,&current);
    if(EFI_ERROR(state))
    {
        DEBUG((DEBUG_ERROR,"[aos.uefi.fsm] Failed to get the file position. EFI status code:%lu.\n",
            state));
        return MAX_UINT64;
    }

    UINT64 result=MAX_UINT64;
    state=file->SetPosition(file,MAX_UINT64);
    if(EFI_ERROR(state))
    {
        DEBUG((DEBUG_ERROR,"[aos.uefi.fsm] Failed to set the file position. EFI status code:%lu.\n",
            state));
        return MAX_UINT64;
    }

    state=file->GetPosition(file,&result);
    if(EFI_ERROR(state))
    {
        DEBUG((DEBUG_ERROR,"[aos.uefi.fsm] Failed to get the file position. EFI status code:%lu.\n",
            state));
        return MAX_UINT64;
    }

    state=file->SetPosition(file,current);
    if(EFI_ERROR(state))
    {
        DEBUG((DEBUG_ERROR,"[aos.uefi.fsm] Failed to set the file position. EFI status code:%lu.\n",
            state));
        return MAX_UINT64;
    }
    return result;
}