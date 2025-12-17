/**
 * 模块内核程序文件加载。
 * @date 2025-12-09
 *
 * Copyright (c) 2025 Tony Chen Smith
 *
 * SPDX-License-Identifier: MIT
 */
#include "loaderi.h"

/**
 * 检查ELF文件头是否符合预期样式。由于前面有哈希与签名保障，这里的检查主要是防呆。
 *
 * @param header 文件头基址。
 * @param size   读取数据长度。安全防呆校验。
 *
 * @return 检验成功返回真。
 */
STATIC BOOLEAN EFIAPI loader_elf_check(IN loader_elf64_ehdr* header,IN UINTN size)
{
    if(header==NULL||size<sizeof(loader_elf64_ehdr))
    {
        return FALSE;
    }

    if(!(header->e_ident[LOADER_EI_MAG0]==LOADER_ELFMAG0&&header->e_ident[LOADER_EI_MAG1]==LOADER_ELFMAG1&&
        header->e_ident[LOADER_EI_MAG2]==LOADER_ELFMAG2&&header->e_ident[LOADER_EI_MAG3]==LOADER_ELFMAG3&&
        header->e_ident[LOADER_EI_CLASS]==LOADER_ELFCLASS64&&header->e_ident[LOADER_EI_DATA]==LOADER_ELFDATA2LSB&&
        header->e_ident[LOADER_EI_OSABI]==LOADER_ELFOSABI_SYSV))
    {
        return FALSE;
    }

    if(!(header->e_type==LOADER_ET_DYN&&header->e_machine==LOADER_EM_AMD64&&header->e_version==LOADER_EV_CURRENT&&
        header->e_ehsize==sizeof(loader_elf64_ehdr)&&header->e_phnum>0&&header->e_phentsize==sizeof(loader_elf64_phdr)))
    {
        return FALSE;
    }

    return TRUE;
}

/**
 * 将内核文件内容映射到内存区域。
 *
 * @param kernel 内核文件句柄。
 * @param params 启动参数。
 *
 * @return 正常完成读写返回成功，任何文件或内存分配问题返回失败。
 */
STATIC EFI_STATUS EFIAPI loader_map(IN EFI_FILE_HANDLE kernel,IN OUT aos_boot_params* params)
{
    EFI_STATUS status=kernel->SetPosition(kernel,0);
    if(EFI_ERROR(status))
    {
        /*内核文件重定位失败*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.loader] Kernel file relocation failed.\n"));
        return status;
    }

    loader_elf64_ehdr* header=(loader_elf64_ehdr*)AllocatePool(sizeof(loader_elf64_ehdr));
    if(header==NULL)
    {
        /*缓冲区申请失败*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.loader] Buffer allocation failed.\n"));
        return EFI_OUT_OF_RESOURCES;
    }
    UINTN size=sizeof(loader_elf64_ehdr);
    status=kernel->Read(kernel,&size,header);
    if(EFI_ERROR(status))
    {
        /*在读取内核文件时出现问题*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.loader] An issue occurred while reading the kernel file.\n"));
        return status;
    }
    if(!loader_elf_check(header,size))
    {
        /*内核文件格式不正确*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.loader] Kernel file format is incorrect.\n"));
        return EFI_UNSUPPORTED;
    }

    UINTN count=header->e_phnum;
    loader_elf64_phdr* phdrs=(loader_elf64_phdr*)AllocatePool(count*sizeof(loader_elf64_phdr));
    if(phdrs==NULL)
    {
        /*缓冲区申请失败*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.loader] Buffer allocation failed.\n"));
        return EFI_OUT_OF_RESOURCES;
    }
    status=kernel->SetPosition(kernel,header->e_phoff);
    if(EFI_ERROR(status))
    {
        /*内核文件重定位失败*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.loader] Kernel file relocation failed.\n"));
        return status;
    }
    size=count*sizeof(loader_elf64_phdr);
    status=kernel->Read(kernel,&size,phdrs);
    if(EFI_ERROR(status))
    {
        /*在读取内核文件时出现问题*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.loader] An issue occurred while reading the kernel file.\n"));
        return status;
    }
    UINTN load_count=0,max=0,range;
    UINTN index=0;
    for(;index<count;index++)
    {
        if(phdrs[index].p_type!=LOADER_PT_LOAD)
        {
            continue;
        }
        load_count++;
        range=phdrs[index].p_vaddr+phdrs[index].p_memsz;
        if(range>max)
        {
            max=range;
        }
    }
    range=EFI_PAGES_TO_SIZE(EFI_SIZE_TO_PAGES(max));

    /*正常来说，单个内核不应该要求占2GB内存空间。出现这种问题在调试时必须断言*/
    ASSERT(range>0&&range<SIZE_2GB);

    UINTN base=-SIZE_512GB;
    UINTN offset=0;

    #ifdef AOS_NDEBUG
    UINT32 value1=LOADER_ADDR_MAGIC,value2=AOS_UEFI_VESION_0_0_1,value3=AOS_UEFI_VESION_0_0_2;
    offset=(random32(&value1,&value2,&value3)&0x3FFFF)<<21;
    UINTN try=0;
    while(SIZE_512GB-offset<range+SIZE_4GB)
    {
        offset=(random32(&value1,&value2,&value3)&0x3FFFF)<<21;
        try++;
        if(try>5)
        {
            break;
        }
    }
    if(!CONFIG_RANDOMIZE_BASE||try>5)
    {
        offset=0;
    }
    #else
    offset=SIZE_4GB;
    #endif /*AOS_NDEBUG*/

    base+=offset;
    params->kinfo.kbase=base;

    params->kinfo.load=load_count;
    params->kinfo.entry=header->e_entry+base;
    params->kinfo.start=umalloc(load_count*sizeof(UINTN));
    params->kinfo.size=umalloc(load_count*sizeof(UINTN));
    params->kinfo.flags=umalloc(load_count*sizeof(UINT64));
    if(params->kinfo.size==NULL||params->kinfo.start==NULL||params->kinfo.flags==NULL)
    {
        /*数组空间申请失败*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.loader] Array space allocation failed.\n"));
        return EFI_OUT_OF_RESOURCES;
    }
    UINTN loadi=0;
    for(index=0;index<count;index++)
    {
        if(phdrs[index].p_type!=LOADER_PT_LOAD)
        {
            continue;
        }
        params->kinfo.start[loadi]=phdrs[index].p_vaddr+base;
        params->kinfo.size[loadi]=phdrs[index].p_memsz;
        params->kinfo.flags[loadi]=AOS_BOOT_VMA_READ|AOS_BOOT_VMA_TYPE_WB;

        if(phdrs[index].p_flags&LOADER_PF_W)
        {
            params->kinfo.flags[loadi]|=AOS_BOOT_VMA_WRITE;
        }

        if(phdrs[index].p_flags&LOADER_PF_X)
        {
            params->kinfo.flags[loadi]|=AOS_BOOT_VMA_EXECUTE;
        }

        UINTN pages=EFI_SIZE_TO_PAGES(phdrs[index].p_vaddr+phdrs[index].p_memsz)-(phdrs[index].p_vaddr>>EFI_PAGE_SHIFT);
        ASSERT(pages>0);
        EFI_PHYSICAL_ADDRESS block=SIZE_4GB;
        status=gBS->AllocatePages(AllocateMaxAddress,EfiLoaderData,pages,&block);
        if(EFI_ERROR(status))
        {
            /*申请页面失败。*/
            DEBUG((DEBUG_ERROR,"[aos.uefi.loader] Failed to allocate pages.\n"));
            return status;
        }

        if(params->kinfo.flags[loadi]&AOS_BOOT_VMA_EXECUTE)
        {
            SetMem((VOID*)block,EFI_PAGES_TO_SIZE(pages),0xCC);
        }
        else
        {
            SetMem((VOID*)block,EFI_PAGES_TO_SIZE(pages),0);
        }

        size=phdrs[index].p_filesz;
        if(size>0)
        {
            status=kernel->SetPosition(kernel,phdrs[index].p_offset);
            if(EFI_ERROR(status))
            {
                /*内核文件重定位失败*/
                DEBUG((DEBUG_ERROR,"[aos.uefi.loader] Kernel file relocation failed.\n"));
                return status;
            }
            status=kernel->Read(kernel,&size,(VOID*)(block+(phdrs[index].p_vaddr&0xFFF)));
            if(EFI_ERROR(status)||size!=phdrs[index].p_filesz)
            {
                /*在读取内核文件时出现问题*/
                DEBUG((DEBUG_ERROR,"[aos.uefi.loader] "
                    "An issue occurred while reading the kernel file.\n"));
                return status;
            }
        }
        
        status=add_kernel_vma((params->kinfo.start[loadi]>>EFI_PAGE_SHIFT)<<EFI_PAGE_SHIFT,
            block,pages,params->kinfo.flags[loadi]);
        if(EFI_ERROR(status))
        {
            /*添加内核线性区失败*/
            DEBUG((DEBUG_ERROR,"[aos.uefi.loader] Failed to add kernel VMA.\n"));
            return status;
        }
        
        loadi++;
    }
    FreePool(header);
    FreePool(phdrs);

    EFI_PHYSICAL_ADDRESS stack=SIZE_4GB;
    status=gBS->AllocatePages(AllocateMaxAddress,EfiLoaderData,CONFIG_KERNEL_STACK,&stack);
    if(EFI_ERROR(status))
    {
        /*申请页面失败*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.loader] Failed to allocate pages.\n"));
        return status;
    }

    params->kinfo.spages=CONFIG_KERNEL_STACK;

    /*内核区域会将最后512GB切割成两块。设计时打算在较大的一块内进行随机化。记住随机地址包含两边的标准2MB页*/
    UINTN a=EFI_SIZE_TO_PAGES(offset);
    UINTN b=EFI_SIZE_TO_PAGES(SIZE_512GB-offset-range-SIZE_4GB);
    UINTN choice=a;
    UINTN choice_base=-SIZE_512GB;
    if(b>a)
    {
        choice=b;
        choice_base+=offset+range;
    }
    if(choice<=CONFIG_KERNEL_STACK+EFI_SIZE_TO_PAGES(SIZE_4MB))
    {
        /*映射栈失败。按道理允许申请的内存资源完全不支持这样做，以防万一我内核写太大的防呆措施*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.loader] Failed to map stack.\n"));
        return status;
    }
    choice-=CONFIG_KERNEL_STACK+EFI_SIZE_TO_PAGES(SIZE_4MB);
    params->kinfo.sbase=choice_base+SIZE_2MB;

    #ifdef AOS_NDEBUG
    if(CONFIG_RANDOMIZE_BASE)
    {
        params->kinfo.sbase+=EFI_PAGES_TO_SIZE(random32(&value1,&value2,&value3)%choice);
    }
    #endif /*AOS_NDEBUG*/

    status=add_kernel_vma(params->kinfo.sbase,stack,CONFIG_KERNEL_STACK,
        AOS_BOOT_VMA_READ|AOS_BOOT_VMA_WRITE|AOS_BOOT_VMA_TYPE_WB);
    if(EFI_ERROR(status))
    {
        /*添加内核线性区失败*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.loader] Failed to add kernel VMA.\n"));
        return status;
    }

    return EFI_SUCCESS;
}

/**
 * 对内核签名进行验证。
 *
 * @param sig    签名文件句柄。
 * @param kernel 内核文件句柄。
 *
 * @return 正常返回成功。出错或验证失败都返回非成功的状态。
 */
STATIC EFI_STATUS loader_verify(IN EFI_FILE_HANDLE sig,IN EFI_FILE_HANDLE kernel)
{
    EFI_STATUS status=sig->SetPosition(sig,0);
    if(EFI_ERROR(status))
    {
        /*内核文件重定位失败*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.loader] Kernel signature file relocation failed.\n"));
        return status;
    }
    status=kernel->SetPosition(kernel,0);
    if(EFI_ERROR(status))
    {
        /*内核文件重定位失败*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.loader] Kernel file relocation failed.\n"));
        return status;
    }

    UINT64 size=ufsize(sig);
    if(size==MAX_UINT64||size<2*sizeof(loader_signature_node)||size%sizeof(loader_signature_node)!=0)
    {
        /*内核签名文件大小不合法*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.loader] The kernel signature file size is invalid.\n"));
        return status;
    }

    UINT8* sha256=AllocatePool(SHA256_DIGEST_SIZE);
    UINT8* sha512=AllocatePool(SHA512_DIGEST_SIZE);
    loader_signature_node* node=AllocatePool(sizeof(loader_signature_node));
    VOID* buffer=AllocatePages(1);
    if(buffer==NULL)
    {
        /*缓冲区申请失败*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.loader] Buffer allocation failed.\n"));
        return EFI_OUT_OF_RESOURCES;
    }

    UINTN bsize=sizeof(loader_signature_node);
    status=FileHandleRead(sig,&bsize,node);
    if(EFI_ERROR(status)||bsize!=sizeof(loader_signature_node))
    {
        /*内核签名文件读取失败*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.loader] Failed to read the kernel signature file.\n"));
        return EFI_UNSUPPORTED;
    }

    VOID* sha512c=AllocatePool(Sha512GetContextSize());
    VOID* sha256c=AllocatePool(Sha256GetContextSize());
    VOID* rsac=RsaNew();
    if(sha256c==NULL||sha512c==NULL||rsac==NULL)
    {
        /*参数空间申请失败*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.loader] Failed to allocate parameter space.\n"));
        return EFI_OUT_OF_RESOURCES;
    }
    if(!Sha512Init(sha512c))
    {
        /*SHA-512计算初始化失败*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.loader] Failed to initialize SHA-512 computation.\n"));
        return EFI_UNSUPPORTED;
    }
    if(!Sha256Init(sha256c))
    {
        /*SHA-256计算初始化失败*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.loader] Failed to initialize SHA-256 computation.\n"));
        return EFI_UNSUPPORTED;
    }
    if(!RsaSetKey(rsac,RsaKeyN,LOADER_SIGNATURE_RSA_KEY_N,
        sizeof(LOADER_SIGNATURE_RSA_KEY_N)))
    {
        /*RSA计算初始化失败*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.loader] Failed to initialize RSA computation.\n"));
        return EFI_UNSUPPORTED;
    }
    if(!RsaSetKey(rsac,RsaKeyE,LOADER_SIGNATURE_RSA_KEY_E,
        sizeof(LOADER_SIGNATURE_RSA_KEY_E)))
    {
        /*RSA计算初始化失败*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.loader] Failed to initialize RSA computation.\n"));
        return EFI_UNSUPPORTED;
    }

    UINT64 index=sizeof(loader_signature_node);
    while(index<size)
    {
        bsize=EFI_PAGE_SIZE;
        status=FileHandleRead(sig,&bsize,buffer);
        if(EFI_ERROR(status))
        {
            /*内核签名文件读取失败*/
            DEBUG((DEBUG_ERROR,"[aos.uefi.loader] Failed to read the kernel signature file.\n"));
            return EFI_UNSUPPORTED;
        }
        if(!Sha512Update(sha512c,buffer,bsize))
        {
            /*计算SHA-512失败*/
            DEBUG((DEBUG_ERROR,"[aos.uefi.loader] Failed to compute SHA-512.\n"));
            return EFI_UNSUPPORTED;
        }
        if(!Sha256Update(sha256c,buffer,bsize))
        {
            /*计算SHA-256失败*/
            DEBUG((DEBUG_ERROR,"[aos.uefi.loader] Failed to compute SHA-256.\n"));
            return EFI_UNSUPPORTED;
        }
        index+=bsize;
    }
    if(!Sha512Final(sha512c,sha512))
    {
        /*计算SHA-512失败*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.loader] Failed to compute SHA-512.\n"));
        return EFI_UNSUPPORTED;
    }
    if(!Sha256Final(sha256c,sha256))
    {
        /*计算SHA-256失败*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.loader] Failed to compute SHA-256.\n"));
        return EFI_UNSUPPORTED;
    }

    DEBUG_CODE_BEGIN();
    DEBUG((DEBUG_INFO,"[aos.uefi.loader] Signature SHA-512:"));
    for(UINTN i=0;i<SHA512_DIGEST_SIZE;i++)
    {
        DEBUG((DEBUG_INFO,"%02X",sha512[i]));
    }
    DEBUG((DEBUG_INFO,"\n"));
    DEBUG((DEBUG_INFO,"[aos.uefi.loader] Signature SHA-256:"));
    for(UINTN i=0;i<SHA256_DIGEST_SIZE;i++)
    {
        DEBUG((DEBUG_INFO,"%02X",sha256[i]));
    }
    DEBUG((DEBUG_INFO,"\n"));
    DEBUG_CODE_END();

    if(CompareMem(sha256,node->sha256,SHA256_DIGEST_SIZE))
    {
        /*内核签名文件SHA-256摘要校验失败，退出系统*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.loader] "
            "Kernel signature file SHA-256 verification failed; the system is exiting.\n"));
        return EFI_UNSUPPORTED;
    }
    if(!RsaPssVerify(rsac,sha512,SHA512_DIGEST_SIZE,node->rsa4096,512,
        SHA512_DIGEST_SIZE,SHA512_DIGEST_SIZE))
    {
        /*内核签名文件RSA-PSS校验失败，退出系统*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.loader] "
            "Kernel signature file RSA-PSS verification failed; the system is exiting.\n"));
        return EFI_UNSUPPORTED;
    }

    RsaFree(rsac);
    rsac=RsaNew();
    if(!Sha512Init(sha512c))
    {
        /*SHA-512计算初始化失败*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.loader] Failed to initialize SHA-512 computation.\n"));
        return EFI_UNSUPPORTED;
    }
    if(!Sha256Init(sha256c))
    {
        /*SHA-256计算初始化失败*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.loader] Failed to initialize SHA-256 computation.\n"));
        return EFI_UNSUPPORTED;
    }
    if(!RsaSetKey(rsac,RsaKeyN,LOADER_KERNEL_RSA_KEY_N,
        sizeof(LOADER_KERNEL_RSA_KEY_N)))
    {
        /*RSA计算初始化失败*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.loader] Failed to initialize RSA computation.\n"));
        return EFI_UNSUPPORTED;
    }
    if(!RsaSetKey(rsac,RsaKeyE,LOADER_KERNEL_RSA_KEY_E,
        sizeof(LOADER_KERNEL_RSA_KEY_E)))
    {
        /*RSA计算初始化失败*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.loader] Failed to initialize RSA computation.\n"));
        return EFI_UNSUPPORTED;
    }

    UINTN entry=size/sizeof(loader_signature_node)-1;
    ASSERT(entry>=1);
    status=sig->SetPosition(sig,sizeof(loader_signature_node));
    if(EFI_ERROR(status))
    {
        /*内核文件重定位失败*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.loader] Kernel signature file relocation failed.\n"));
        return status;
    }
    size=ufsize(kernel);
    index=0;
    while(index<size)
    {
        bsize=EFI_PAGE_SIZE;
        status=FileHandleRead(kernel,&bsize,buffer);
        if(EFI_ERROR(status))
        {
            /*内核签名文件读取失败*/
            DEBUG((DEBUG_ERROR,"[aos.uefi.loader] Failed to read the kernel file.\n"));
            return EFI_UNSUPPORTED;
        }
        if(!Sha512Update(sha512c,buffer,bsize))
        {
            /*计算SHA-512失败*/
            DEBUG((DEBUG_ERROR,"[aos.uefi.loader] Failed to compute SHA-512.\n"));
            return EFI_UNSUPPORTED;
        }
        if(!Sha256Update(sha256c,buffer,bsize))
        {
            /*计算SHA-256失败*/
            DEBUG((DEBUG_ERROR,"[aos.uefi.loader] Failed to compute SHA-256.\n"));
            return EFI_UNSUPPORTED;
        }
        index+=bsize;
    }
    if(!Sha512Final(sha512c,sha512))
    {
        /*计算SHA-512失败*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.loader] Failed to compute SHA-512.\n"));
        return EFI_UNSUPPORTED;
    }
    if(!Sha256Final(sha256c,sha256))
    {
        /*计算SHA-256失败*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.loader] Failed to compute SHA-256.\n"));
        return EFI_UNSUPPORTED;
    }

    DEBUG_CODE_BEGIN();
    DEBUG((DEBUG_INFO,"[aos.uefi.loader] Kernel SHA-512:"));
    for(UINTN i=0;i<SHA512_DIGEST_SIZE;i++)
    {
        DEBUG((DEBUG_INFO,"%02X",sha512[i]));
    }
    DEBUG((DEBUG_INFO,"\n"));
    DEBUG((DEBUG_INFO,"[aos.uefi.loader] Kernel SHA-256:"));
    for(UINTN i=0;i<SHA256_DIGEST_SIZE;i++)
    {
        DEBUG((DEBUG_INFO,"%02X",sha256[i]));
    }
    DEBUG((DEBUG_INFO,"\n"));
    DEBUG_CODE_END();

    index=0;
    while(index<entry)
    {
        bsize=sizeof(loader_signature_node);
        status=FileHandleRead(sig,&bsize,node);

        if(EFI_ERROR(status)||bsize!=sizeof(loader_signature_node))
        {
            /*内核签名文件读取失败*/
            DEBUG((DEBUG_ERROR,"[aos.uefi.loader] Failed to read the kernel signature file.\n"));
            return EFI_UNSUPPORTED;
        }
        if(!CompareMem(sha256,node->sha256,SHA256_DIGEST_SIZE))
        {
            if(!RsaPssVerify(rsac,sha512,SHA512_DIGEST_SIZE,node->rsa4096,
                512,SHA512_DIGEST_SIZE,SHA512_DIGEST_SIZE))
            {
                /*内核签名文件RSA-PSS校验失败，退出系统*/
                DEBUG((DEBUG_ERROR,"[aos.uefi.loader] "
                    "Kernel signature file RSA-PSS verification failed; the system is exiting.\n"));
                return EFI_UNSUPPORTED;
            }
            else
            {
                RsaFree(rsac);
                FreePool(node);
                FreePool(sha512);
                FreePool(sha256);
                FreePool(sha512c);
                FreePool(sha256c);
                FreePages(buffer,1);
                return EFI_SUCCESS;
            }
        }
        index++;
    }
    /*内核签名文件SHA-256摘要校验失败，退出系统*/
    DEBUG((DEBUG_ERROR,"[aos.uefi.loader] "
        "Kernel signature file SHA-256 verification failed; the system is exiting.\n"));
    return EFI_UNSUPPORTED;
}

/**
 * 将内核文件加载到目标区域。
 *
 * @param params 启动参数。
 *
 * @return 正常返回成功。
 */
EFI_STATUS EFIAPI load_kernel(IN OUT aos_boot_params* params)
{
    mount();

    EFI_FILE_HANDLE sig=ufopen(LOADER_SIG_PATH,EFI_FILE_MODE_READ,0);
    if(sig==NULL)
    {
        /*未发现或无法打开内核签名文件*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.loader] "
            "The kernel signature file was not found or could not be opened.\n"));
        return EFI_NOT_FOUND;
    }

    EFI_FILE_HANDLE kernel=ufopen(LOADER_KERNEL_PATH,EFI_FILE_MODE_READ,0);
    if(kernel==NULL)
    {
        /*未发现或无法打开内核文件*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.loader] "
            "The kernel file was not found or could not be opened.\n"));
        return EFI_NOT_FOUND;
    }

    EFI_STATUS status;

    /*校验*/
    status=loader_verify(sig,kernel);
    if(EFI_ERROR(status))
    {
        /*验证内核签名失败*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.loader] Failed to verify the kernel signature.\n"));
        return status;
    }
    sig->Close(sig);

    status=loader_map(kernel,params);
    if(EFI_ERROR(status))
    {
        /*映射内核失败*/
        DEBUG((DEBUG_ERROR,"[aos.uefi.loader] Failed to map the kernel.\n"));
        return status;
    }
    kernel->Close(kernel);

    umount();

    return EFI_SUCCESS;
}