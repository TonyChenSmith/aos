/**
 * 模块签名验证。
 * @date 2026-09-30
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include <Library/BaseCryptLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>

#include "file.h"
#include "key.h"
#include "panic.h"
#include "version.h"

/**
 * 平年每月天数表。
 */
STATIC CONST UINT8 VERIFY_MONTH_DAYS[13]={0,31,28,31,30,31,30,31,31,30,
    31,30,31};

/**
 * 签名结点结构。
 */
#pragma pack(1)
typedef struct _verify_signature_node
{
    UINT64 version;                    /*版本BCD。*/
    UINT8  sha256[SHA256_DIGEST_SIZE]; /*SHA256摘要。*/
    UINT8  sha384[SHA384_DIGEST_SIZE]; /*SHA384摘要。*/
    UINT8  sha512[SHA512_DIGEST_SIZE]; /*SHA512摘要。*/
    UINT8  ecdsa[96];                  /*ECDSA P-384签名。*/
} verify_signature_node;
#pragma pack()

/**
 * 判断是否为公历闰年。
 * 
 * @param year 年份（十进制数值，如2026）。
 * 
 * @return 闰年返回真。
 */
STATIC BOOLEAN EFIAPI verify_is_leap_year(IN UINT64 year)
{
    return (BOOLEAN)(((year%4==0)&&(year%100!=0))||(year%400==0));
}

/**
 * 检查是否为有效的BCD日期。
 * 
 * @param date BCD日期。
 * 
 * @return 有效返回真。
 */
STATIC BOOLEAN EFIAPI verify_is_valid_date(IN UINT64 date)
{
    for(UINTN i=0;i<8;i++)
    {
        UINT8 byte=(UINT8)(date>>(i<<3));
        if((byte>>4)>9||(byte&0xF)>9)
        {
            return FALSE;
        }
    }

    UINT8 day=0;
    UINT8 month=0;
    UINT64 year=0;

    day=(date&0xF)+((date>>4)&0xF)*10;
    month=((date>>8)&0xF)+((date>>12)&0xF)*10;

    UINT64 y=date>>16;
    UINTN i=1;
    while(y>0)
    {
        year+=(y&0xF)*i;
        i*=10;
        y>>=4;
    }

    if(year==0)
    {
        /*公历0年未定义*/
        return FALSE;
    }
    if(month<1||month>12)
    {
        return FALSE;
    }
    UINT8 max_day=VERIFY_MONTH_DAYS[month];
    if(month==2&&verify_is_leap_year(year))
    {
        max_day=29;
    }
    if(day<1||day>max_day)
    {
        return FALSE;
    }

    return TRUE;
}

/**
 * 检查读取到的版本BCD。
 * 
 * @param uefi_version   模块“aos.uefi”版本BCD。
 * @param kernel_version 模块“aos.kernel”版本BCD。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI verify_check_version(IN UINT64 uefi_version,IN UINT64 kernel_version)
{
    if(!(verify_is_valid_date(uefi_version)&&verify_is_valid_date(kernel_version)))
    {
        panic(EFI_UNSUPPORTED,"The signature file format is invalid.");
    }

    if(uefi_version<AOS_UEFI_VERSION)
    {
        /*新版本引导器不能使用适用于旧版本引导器的签名进行验证*/
        panic(EFI_UNSUPPORTED,"This bootloader cannot use a signature intended for an older "
            "bootloader version for verification.");
    }

    if(uefi_version>AOS_UEFI_VERSION)
    {
        /*旧版本引导器不能使用适用于新版本引导器的签名进行验证*/
        panic(EFI_UNSUPPORTED,"This bootloader cannot use a signature intended for a newer "
            "bootloader version for verification.");
    }

    if(uefi_version>kernel_version)
    {
        /*引导器不能加载低于引导器版本的内核*/
        panic(EFI_UNSUPPORTED,"This bootloader cannot load a kernel older than its own version.");
    }
}

/**
 * 验证签名和内核。
 * 
 * @param signature 签名文件句柄。
 * @param kernel    内核文件句柄。
 * 
 * @return 无返回值。
 */
VOID EFIAPI verify_kernel(IN asv_file* signature,IN asv_file* kernel)
{
    if(signature==NULL||kernel==NULL)
    {
        panic(EFI_INVALID_PARAMETER,"The signature file or the kernel file is null.");
    }

    UINTN signature_size=asv_get_size(signature);
    UINTN kernel_size=asv_get_size(kernel);
    if(signature_size!=2*sizeof(verify_signature_node))
    {
        panic(EFI_INVALID_PARAMETER,"The signature file size is invalid.");
    }
    if(kernel_size==0)
    {
        panic(EFI_INVALID_PARAMETER,"The kernel file is empty.");
    }

    verify_signature_node* nodes=AllocatePool(2*sizeof(verify_signature_node));
    if(nodes==NULL)
    {
        panic(EFI_OUT_OF_RESOURCES,"Not enough memory to verify the signature.");
    }

    signature_size=asv_read_at(signature,0,nodes,2*sizeof(verify_signature_node));
    if(signature_size!=2*sizeof(verify_signature_node))
    {
        panic(EFI_UNSUPPORTED,"Failed to read the signature file.");
    }

    verify_check_version(nodes[0].version,nodes[1].version);

    UINT8* message_sha256=AllocatePool(SHA256_DIGEST_SIZE);
    UINT8* message_sha384=AllocatePool(SHA384_DIGEST_SIZE);
    UINT8* message_sha512=AllocatePool(SHA512_DIGEST_SIZE);
    UINT8* group_sha512=AllocatePool(SHA512_DIGEST_SIZE);
    VOID* sha256_context=AllocatePool(Sha256GetContextSize());
    VOID* sha384_context=AllocatePool(Sha384GetContextSize());
    VOID* sha512_context=AllocatePool(Sha512GetContextSize());
    VOID* group_sha512_context=AllocatePool(Sha512GetContextSize());
    VOID* file_buffer=AllocatePool(EFI_PAGE_SIZE);
    if(message_sha256==NULL||message_sha384==NULL||message_sha512==NULL||group_sha512==NULL||sha256_context==NULL||
        sha384_context==NULL||sha512_context==NULL||group_sha512_context==NULL||file_buffer==NULL)
    {
        panic(EFI_OUT_OF_RESOURCES,"Not enough memory to verify the signature.");
    }

    ZeroMem(sha256_context,Sha256GetContextSize());
    ZeroMem(sha384_context,Sha384GetContextSize());
    ZeroMem(sha512_context,Sha512GetContextSize());
    ZeroMem(group_sha512_context,Sha512GetContextSize());
    if(!(Sha256Init(sha256_context)&&Sha384Init(sha384_context)&&
        Sha512Init(sha512_context)&&Sha512Init(group_sha512_context)))
    {
        panic(EFI_DEVICE_ERROR,"Failed to initialize the hash computation.");
    }

    if(!Sha512Update(group_sha512_context,&nodes[0],sizeof(UINT64)+SHA256_DIGEST_SIZE+
        SHA384_DIGEST_SIZE+SHA512_DIGEST_SIZE))
    {
        panic(EFI_DEVICE_ERROR,"Failed to compute the hash.");
    }

    if(!Sha256Update(sha256_context,&nodes[1],sizeof(verify_signature_node))||
        !Sha384Update(sha384_context,&nodes[1],sizeof(verify_signature_node))||
        !Sha512Update(sha512_context,&nodes[1],sizeof(verify_signature_node))||
        !Sha512Update(group_sha512_context,&nodes[1],sizeof(verify_signature_node)))
    {
        panic(EFI_DEVICE_ERROR,"Failed to compute the hash.");
    }

    if(!Sha256Final(sha256_context,message_sha256)||
        !Sha384Final(sha384_context,message_sha384)||
        !Sha512Final(sha512_context,message_sha512)||
        !Sha512Final(group_sha512_context,group_sha512))
    {
        panic(EFI_DEVICE_ERROR,"Failed to compute the hash.");
    }

    if(CompareMem(message_sha256,nodes[0].sha256,SHA256_DIGEST_SIZE)!=0||
        CompareMem(message_sha384,nodes[0].sha384,SHA384_DIGEST_SIZE)!=0||
        CompareMem(message_sha512,nodes[0].sha512,SHA512_DIGEST_SIZE)!=0)
    {
        panic(EFI_UNSUPPORTED,"The hash of the signature file does not match.");
    }

    VOID* key=NULL;
    if(!EcGetPublicKeyFromX509(AOS_UEFI_PUBLIC_KEY,AOS_UEFI_PUBLIC_KEY_SIZE,&key))
    {
        panic(EFI_DEVICE_ERROR,"Failed to get the public key.");
    }
    if(!EcDsaVerify(key,CRYPTO_NID_SHA512,group_sha512,SHA512_DIGEST_SIZE,
        nodes[0].ecdsa,sizeof(nodes[0].ecdsa)))
    {
        panic(EFI_UNSUPPORTED,"Failed to verify the signature file.");
    }
    EcFree(key);

    ZeroMem(sha256_context,Sha256GetContextSize());
    ZeroMem(sha384_context,Sha384GetContextSize());
    ZeroMem(sha512_context,Sha512GetContextSize());
    ZeroMem(group_sha512_context,Sha512GetContextSize());
    if(!(Sha256Init(sha256_context)&&Sha384Init(sha384_context)&&
        Sha512Init(sha512_context)&&Sha512Init(group_sha512_context)))
    {
        panic(EFI_DEVICE_ERROR,"Failed to initialize the hash computation.");
    }

    if(!Sha512Update(group_sha512_context,&nodes[1],sizeof(UINT64)+SHA256_DIGEST_SIZE+
        SHA384_DIGEST_SIZE+SHA512_DIGEST_SIZE))
    {
        panic(EFI_DEVICE_ERROR,"Failed to compute the hash.");
    }

    asv_reset(kernel);
    for(UINT64 offset=0;offset<kernel_size;)
    {
        UINT64 chunk=kernel_size-offset;
        if(chunk>EFI_PAGE_SIZE)
        {
            chunk=EFI_PAGE_SIZE;
        }
        UINT64 read_size=asv_read(kernel,file_buffer,chunk);
        if(read_size!=chunk)
        {
            panic(EFI_DEVICE_ERROR,"Failed to read the kernel file.");
        }
        if(!Sha256Update(sha256_context,file_buffer,read_size)||
            !Sha384Update(sha384_context,file_buffer,read_size)||
            !Sha512Update(sha512_context,file_buffer,read_size)||
            !Sha512Update(group_sha512_context,file_buffer,read_size))
        {
            panic(EFI_DEVICE_ERROR,"Failed to compute the hash.");
        }
        offset+=read_size;
    }

    if(!Sha256Final(sha256_context,message_sha256)||
        !Sha384Final(sha384_context,message_sha384)||
        !Sha512Final(sha512_context,message_sha512)||
        !Sha512Final(group_sha512_context,group_sha512))
    {
        panic(EFI_DEVICE_ERROR,"Failed to compute the hash.");
    }

    if(CompareMem(message_sha256,nodes[1].sha256,SHA256_DIGEST_SIZE)!=0||
        CompareMem(message_sha384,nodes[1].sha384,SHA384_DIGEST_SIZE)!=0||
        CompareMem(message_sha512,nodes[1].sha512,SHA512_DIGEST_SIZE)!=0)
    {
        panic(EFI_UNSUPPORTED,"The hash of the kernel file does not match.");
    }

    key=NULL;
    if(!EcGetPublicKeyFromX509(AOS_KERNEL_PUBLIC_KEY,AOS_KERNEL_PUBLIC_KEY_SIZE,&key))
    {
        panic(EFI_DEVICE_ERROR,"Failed to get the public key.");
    }
    if(!EcDsaVerify(key,CRYPTO_NID_SHA512,group_sha512,SHA512_DIGEST_SIZE,
        nodes[1].ecdsa,sizeof(nodes[1].ecdsa)))
    {
        panic(EFI_UNSUPPORTED,"Failed to verify the kernel file.");
    }
    EcFree(key);

    FreePool(nodes);
    FreePool(message_sha256);
    FreePool(message_sha384);
    FreePool(message_sha512);
    FreePool(group_sha512);
    FreePool(sha256_context);
    FreePool(sha384_context);
    FreePool(sha512_context);
    FreePool(group_sha512_context);
    FreePool(file_buffer);
}