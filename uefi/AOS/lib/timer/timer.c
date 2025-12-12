/**
 * 模块所用时间库实现。内部使用TSC实现。
 * @date 2025-12-09
 * 
 * Copyright (c) 2025 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/TimerLib.h>
#include <Library/UefiBootServicesTableLib.h>

/**
 * TSC时钟。
 */
STATIC UINT64 clock=1000000000;

/**
 * 每纳秒计数。
 */
STATIC UINT64 count=1;

/**
 * Stalls the CPU for at least the given number of microseconds.
 * 
 * Stalls the CPU for the number of microseconds specified by MicroSeconds.
 * 
 * @param MicroSeconds  The minimum number of microseconds to delay.
 * 
 * @return The value of MicroSeconds inputted.
 */
UINTN EFIAPI MicroSecondDelay(IN UINTN MicroSeconds)
{
    if(MicroSeconds!=0)
    {
        gBS->Stall(MicroSeconds);
    }
    return MicroSeconds;
}

/**
 * Stalls the CPU for at least the given number of nanoseconds.
 * 
 * Stalls the CPU for the number of nanoseconds specified by NanoSeconds.
 * 
 * @param NanoSeconds The minimum number of nanoseconds to delay.
 * 
 * @return The value of NanoSeconds inputted.
 */
UINTN EFIAPI NanoSecondDelay(IN UINTN NanoSeconds)
{
    if(NanoSeconds==0)
    {
        return 0;
    }
    UINTN result=NanoSeconds;
    if(NanoSeconds>1000)
    {
        gBS->Stall(NanoSeconds/1000);
        NanoSeconds=NanoSeconds%1000;
    }
    UINT64 wait=count*NanoSeconds;
    if(wait!=0)
    {
        UINT64 start=AsmReadTsc();
        while(AsmReadTsc()-start<wait)
        {
            ;
        }
    }
    return result;
}

/**
 * Retrieves the current value of a 64-bit free running performance counter.
 * 
 * The counter can either count up by 1 or count down by 1. If the physical
 * performance counter counts by a larger increment, then the counter values
 * must be translated. The properties of the counter can be retrieved from
 * GetPerformanceCounterProperties().
 * 
 * @return The current value of the free running performance counter.
 */
UINT64 EFIAPI GetPerformanceCounter(VOID)
{
    return AsmReadTsc();
}

/**
 * Retrieves the 64-bit frequency in Hz and the range of performance counter
 * values.
 * 
 * If StartValue is not NULL, then the value that the performance counter starts
 * with immediately after is it rolls over is returned in StartValue. If
 * EndValue is not NULL, then the value that the performance counter end with
 * immediately before it rolls over is returned in EndValue. The 64-bit
 * frequency of the performance counter in Hz is always returned. If StartValue
 * is less than EndValue, then the performance counter counts up. If StartValue
 * is greater than EndValue, then the performance counter counts down. For
 * example, a 64-bit free running counter that counts up would have a StartValue
 * of 0 and an EndValue of 0xFFFFFFFFFFFFFFFF. A 24-bit free running counter
 * that counts down would have a StartValue of 0xFFFFFF and an EndValue of 0.
 * 
 * @param StartValue The value the performance counter starts with when it
 *                   rolls over.
 * @param EndValue   The value that the performance counter ends with before
 *                   it rolls over.
 * 
 * @return The frequency in Hz.
 */
UINT64 EFIAPI GetPerformanceCounterProperties(OUT UINT64* StartValue OPTIONAL,OUT UINT64* EndValue OPTIONAL)
{
    if(StartValue!= NULL) {
        *StartValue=0;
    }
    if(EndValue!=NULL) {
        *EndValue=MAX_UINT64;
    }
    return clock;
}

/**
 * Converts elapsed ticks of performance counter to time in nanoseconds.
 * 
 * This function converts the elapsed ticks of running performance counter to
 * time value in unit of nanoseconds.
 * 
 * @param Ticks The number of elapsed ticks of running performance counter.
 * 
 * @return The elapsed time in nanoseconds.
 */
UINT64 EFIAPI GetTimeInNanoSecond(IN UINT64 Ticks)
{
    if(clock==0)
    {
        return 0;
    }

    UINT64 result;
    UINT64 remainder;

    result=DivU64x64Remainder(Ticks,count,&remainder);
    if(remainder>=(count>>1))
    {
        result++;
    }

    return result;
}

/**
 * 库入口，获取TSC的频率信息。
 * 
 * @return 一定成功。
 */
EFI_STATUS EFIAPI aos_uefi_timer_entry(VOID)
{
    clock=AsmReadTsc();
    gBS->Stall(50);
    clock=AsmReadTsc()-clock;
    count=clock/50000;
    count=MAX(count,1);
    clock=count*1000000000;
    return EFI_SUCCESS;
}