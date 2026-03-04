/**
 * 模块所用随机数生成器实现。内部视情况最多三层次的多重混合。
 * @date 2026-03-02
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Register/Intel/Cpuid.h>

/**
 * 时间戳计数器是否可用。
 */
STATIC BOOLEAN tsc=FALSE;

/**
 * 随机数指令是否可用。
 */
STATIC BOOLEAN rdrand=FALSE;

/**
 * 对获取随机数指令进行检测。
 * 
 * @return 指令成功返回真，否则返回假。
 */
STATIC BOOLEAN EFIAPI random_rdrand_test()
{
    if(!rdrand)
    {
        return FALSE;
    }

    UINT64 previous=0;
    UINTN change_time=0;
    for(UINTN test=0;test<10;test++)
    {
        UINT64 value;
        UINTN try;
        for(try=0;try<10;try++)
        {
            if(AsmRdRand64(&value))
            {
                break;
            }
        }

        if(try>=10)
        {
            return FALSE;
        }

        if(test>0&&previous!=value)
        {
            change_time++;
        }
        previous=value;
    }

    return change_time>=7;
}

/**
 * xxHash64风格混合函数。
 * 
 * @param h 无符号64位值h。
 * @param h 无符号64位值k。
 * 
 * @return 混合后的无符号64位值。
 */
STATIC UINT64 EFIAPI random_xxhash64_mix(IN UINT64 h,IN UINT64 k)
{
    k=MultU64x64(k,0xBF58476D1CE4E5B9ULL);
    k^=k>>33;
    k=MultU64x64(k,0x94D049BB133111EBULL);
    k^=k>>29;
    k=MultU64x64(k,0xC4CEB9FE1A85EC53ULL);
    k^=k>>32;
    h^=k;
    h=MultU64x64(h,0x9E3779B97F4A7C15ULL)+0x85EBCA77C2B2AE63ULL;
    h^=h>>33;
    h=MultU64x64(h,0xBF58476D1CE4E5B9ULL);
    h^=h>>29;
    return h;
}

/**
 * MurmurHash3风格混合函数。
 * 
 * @param k    无符号64位值k。
 * @param seed 无符号64位值seed。
 * 
 * @return 混合后的无符号64位值。
 */
STATIC UINT64 EFIAPI random_murmur64_mix(IN UINT64 k,IN UINT64 seed)
{
    k^=seed;
    k=MultU64x64(k,0x9E3779B97F4A7C15ULL);
    k^=k>>33;
    k=MultU64x64(k,0xBF58476D1CE4E5B9ULL);
    k^=k>>33;
    return k;
}

/**
 * wyhash风格混合函数。
 * 
 * @param a 无符号64位值a。
 * @param b 无符号64位值b。
 * 
 * @return 混合后的无符号64位值。
 */
STATIC UINT64 EFIAPI random_wyhash_mix(IN UINT64 a,IN UINT64 b)
{
    UINT64 m1=MultU64x64(a^0xE7037ED1A0B428DBULL,b^0x8A96F1D043B03A13ULL);
    m1^=m1>>32;
    m1=MultU64x64(m1,0xBF58476D1CE4E5B9ULL);
    m1^=m1>>32;
    return m1;
}

/**
 * splitmix64风格混合函数。
 * 
 * @param x 无符号64位值x。
 * 
 * @return 混合后的无符号64位值。
 */
STATIC UINT64 EFIAPI random_splitmix64(IN UINT64 x)
{
    x+=0x9E3779B97F4A7C15ULL;
    x=MultU64x64(x^(x>>30),0xBF58476D1CE4E5B9ULL);
    x=MultU64x64(x^(x >> 27),0x94D049BB133111EBULL);
    return x^(x>>31);
}

/**
 * 收集与混合熵。
 * 
 * @param result 返回值数组。
 * 
 * @return 无返回值。
 */
STATIC VOID EFIAPI random_collect_mix_entropy(OUT UINT64* result)
{
    UINT64 state0=0;
    UINT64 state1=0;
    UINT64 accumulator=0;
    
    EFI_TIME time;
    gST->RuntimeServices->GetTime(&time, NULL);
    UINT64 time_parts[]={(((UINT64)time.Nanosecond)<<32)|time.Year,(((UINT64)time.Month)<<48)|
        (((UINT64)time.Day)<<32)|(time.Hour<<16)|time.Minute,(((UINT64)time.Second)<<32)|time.TimeZone,
        time.Daylight};
    
    for(UINTN i=0;i<4;i++)
    {
        accumulator=random_splitmix64(accumulator^time_parts[i]);
        state0^=random_murmur64_mix(time_parts[i],accumulator);
        state1=random_xxhash64_mix(state1,time_parts[i]^accumulator);
    }
    
    UINT64 monotonic;
    gBS->GetNextMonotonicCount(&monotonic);
    
    state0=random_splitmix64(state0^monotonic);
    state1=random_splitmix64(state1^monotonic);
    UINT64 mix_val=random_wyhash_mix(state0,state1);
    state0=random_murmur64_mix(state0,mix_val);
    state1=random_murmur64_mix(state1,~mix_val);
    
    if(tsc)
    {
        UINT64 tsc_value=AsmReadTsc();
        
        for(UINTN rot=1;rot<=61;rot+=20)
        {
            state0^=LRotU64(tsc_value,rot);
            state1^=LRotU64(~tsc_value,64-rot);
        }
    }
    
    if(random_rdrand_test())
    {
        UINT64 rand;
        UINT64 mixed;
        UINTN rand_success_count=0;
        
        for(UINTN i=0;i<3;i++) {
            if(AsmRdRand64(&rand))
            {
                rand_success_count++;
                
                switch(i)
                {
                    case 0:
                        state0=random_xxhash64_mix(state0,rand);
                        break;
                    case 1:
                        state1=random_xxhash64_mix(state1,rand);
                        break;
                    case 2: 
                        mixed=random_wyhash_mix(state0,state1);
                        state0^=LRotU64(rand,19);
                        state1^=LRotU64(mixed,37);
                        break;
                }
            }
        }

        if(rand_success_count==0)
        {
            state0=random_splitmix64(state0^state1);
            state1=random_splitmix64(state1^state0);
        }
    }
    
    UINT64 addr_entropy=(UINT64)&state0;
    state0^=random_murmur64_mix(addr_entropy,state1);
    state1^=random_murmur64_mix(~addr_entropy,state0);
    
    state0=random_xxhash64_mix(state0,state1);
    state1=random_xxhash64_mix(state1,state0);
    UINT64 final_mix=random_splitmix64(state0^state1);
    state0^=final_mix;
    state1^=random_splitmix64(final_mix);
    result[0]=random_wyhash_mix(state0,state1);
    result[1]=random_wyhash_mix(state1,state0);
    
    if(result[0]==result[1])
    {
        result[0]=random_splitmix64(result[0]);
        result[1]=random_splitmix64(result[1]);
    }
}

/**
 * Generates a 128-bit random number.
 * 
 * if Rand is NULL, then ASSERT().
 * 
 * @param Rand Buffer pointer to store the 128-bit random value.
 * 
 * @retval TRUE  Random number generated successfully.
 * @retval FALSE Failed to generate the random number.
 * 
 */
BOOLEAN EFIAPI GetRandomNumber128(OUT UINT64* Rand)
{
    ASSERT(Rand!=NULL);

    STATIC UINT64 reseed_counter=0;
    random_collect_mix_entropy(Rand);

    reseed_counter++;
    Rand[0]^=random_murmur64_mix(reseed_counter,Rand[1]);
    Rand[1]^=random_murmur64_mix(~reseed_counter,Rand[0]);
    Rand[0]=random_splitmix64(Rand[0]);
    Rand[1]=random_splitmix64(Rand[1]);

    return TRUE;
}

/**
 * Generates a 64-bit random number.
 * 
 * if Rand is NULL, then ASSERT().
 * 
 * @param Rand Buffer pointer to store the 64-bit random value.
 * 
 * @retval TRUE  Random number generated successfully.
 * @retval FALSE Failed to generate the random number.
 * 
 */
BOOLEAN EFIAPI GetRandomNumber64(OUT UINT64* Rand)
{
    ASSERT(Rand!=NULL);

    UINT64 buffer[2];
    GetRandomNumber128(buffer);

    *Rand=buffer[0]^buffer[1];
    *Rand=MultU64x64(*Rand,0x9E3779B97F4A7C15ULL);
    *Rand^=*Rand>>32;
    *Rand=MultU64x64(*Rand,0xC4CEB9FE1A85EC53ULL);
    *Rand^=*Rand>>32;

    return TRUE;
}

/**
 * Generates a 32-bit random number.
 * 
 * if Rand is NULL, then ASSERT().
 * 
 * @param Rand Buffer pointer to store the 32-bit random value.
 * 
 * @retval TRUE  Random number generated successfully.
 * @retval FALSE Failed to generate the random number.
 * 
 */
BOOLEAN EFIAPI GetRandomNumber32(OUT UINT32* Rand)
{
    ASSERT(Rand!=NULL);

    UINT64 buffer[2];
    GetRandomNumber128(buffer);
    
    UINT64 x=buffer[0]^buffer[1];
    *Rand=(UINT32)(x&0xFFFFFFFF);
    *Rand=*Rand*0x9E3779B9UL+(x>>32);
    *Rand^=*Rand>>16;
    *Rand*=0x85EBCA77UL;
    *Rand^=*Rand>>13;
    
    return TRUE;
}

/**
 * Generates a 16-bit random number.
 * 
 * if Rand is NULL, then ASSERT().
 *
 * @param Rand Buffer pointer to store the 16-bit random value.
 * 
 * @retval TRUE  Random number generated successfully.
 * @retval FALSE Failed to generate the random number.
 * 
 */
BOOLEAN EFIAPI GetRandomNumber16(OUT UINT16* Rand)
{
    ASSERT(Rand!=NULL);

    UINT64 buffer[2];
    GetRandomNumber128(buffer);

    UINT64 x=buffer[0]^buffer[1];
    UINT32 mixed=(UINT32)(x&0xFFFFFFFF);
    mixed=mixed*0x9E3779B9UL+(x>>32);
    mixed^=mixed>>16;
    mixed*=0x85EBCA77UL;
    mixed^=mixed>>13;

    *Rand=(UINT16)((mixed>>16)^mixed);
    
    return TRUE;
}

/**
 * Get a GUID identifying the RNG algorithm implementation.
 * 
 * @param RngGuid If success, contains the GUID identifying
 *                the RNG algorithm implementation.
 * 
 * @retval EFI_SUCCESS           Success.
 * @retval EFI_UNSUPPORTED       Not supported.
 * @retval EFI_INVALID_PARAMETER Invalid parameter.
 */
EFI_STATUS EFIAPI GetRngGuid(OUT GUID* RngGuid)
{
    /*实现多种，且仅仅保证可用，所以不支持获取算法细节的GUID*/
    return EFI_UNSUPPORTED;
}

/**
 * 初始化随机数生成器。
 * 
 * @return 一定成功。
 */
EFI_STATUS EFIAPI aos_uefi_random_entry(VOID)
{
    UINT32 eax,ebx,ecx,edx;
    AsmCpuid(CPUID_SIGNATURE,&eax,&ebx,&ecx,&edx);
    if(eax>=CPUID_VERSION_INFO)
    {
        AsmCpuid(CPUID_VERSION_INFO,&eax, &ebx, &ecx, &edx);
        CPUID_VERSION_INFO_EDX ml1edx={.Uint32=edx};
        CPUID_VERSION_INFO_ECX ml1ecx={.Uint32=ecx};
        if(ml1edx.Bits.TSC)
        {
            tsc=TRUE;
        }
        if(ml1ecx.Bits.RDRAND)
        {
            rdrand=TRUE;
        }
    }
    return EFI_SUCCESS;
}