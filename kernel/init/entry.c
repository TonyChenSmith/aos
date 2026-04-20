/**
 * 内核模块入口。
 * @date 2026-01-26
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include <init/params.h>
#include <support/barrier.h>
#include <support/io.h>

#include <support/sync.h>
#include <panic/panic.h>

int test_atomic_operations(void)
{
    /* 1. 声明各种原子类型变量 */
    atomic_int8 a8;
    atomic_int16 a16;
    atomic_int32 a32;
    atomic_int64 a64;
    atomic_uint8 u8;
    atomic_uint16 u16;
    atomic_uint32 u32;
    atomic_uint64 u64;
    atomic_intn an;
    atomic_uintn un;
    atomic_bool ab;
    atomic_flag af;
    
    /* 2. 初始化原子变量 */
    atomic_init(&a8, 1);
    atomic_init(&a16, 2);
    atomic_init(&a32, 3);
    atomic_init(&a64, 4);
    atomic_init(&u8, 5);
    atomic_init(&u16, 6);
    atomic_init(&u32, 7);
    atomic_init(&u64, 8);
    atomic_init(&an, 9);
    atomic_init(&un, 10);
    atomic_init(&ab, true);
    atomic_init(&af,ATOMIC_FLAG_INIT);
    
    /* 用于存储操作结果的变量，防止优化 */
    int result = 0;
    
    /* 3. 测试atomic_store和atomic_load */
    atomic_store(&a32, 100);
    result += atomic_load(&a32);
    
    atomic_store(&u64, 200);
    result += atomic_load(&u64);
    
    /* 4. 测试显式内存顺序版本 */
    atomic_store_explicit(&a32, 300, MEMORY_ORDER_RELAXED);
    result += atomic_load_explicit(&a32, MEMORY_ORDER_ACQUIRE);
    
    atomic_store_explicit(&u64, 400, MEMORY_ORDER_RELEASE);
    result += atomic_load_explicit(&u64, MEMORY_ORDER_SEQ_CST);
    
    /* 5. 测试atomic_exchange */
    int old1 = atomic_exchange(&a32, 500);
    result += old1;
    
    int old2 = atomic_exchange_explicit(&a32, 600, MEMORY_ORDER_ACQ_REL);
    result += old2;
    
    /* 6. 测试atomic_compare_exchange_strong */
    int expected1 = 600;
    bool success1 = atomic_compare_exchange_strong(&a32, &expected1, 700);
    result += success1 ? 1 : 0;
    result += expected1;
    
    int expected2 = 700;
    bool success2 = atomic_compare_exchange_strong_explicit(&a32, &expected2, 800,
                                                           MEMORY_ORDER_SEQ_CST,
                                                           MEMORY_ORDER_RELAXED);
    result += success2 ? 2 : 0;
    result += expected2;
    
    /* 7. 测试atomic_compare_exchange_weak */
    int expected3 = 800;
    bool success3 = atomic_compare_exchange_weak(&a32, &expected3, 900);
    result += success3 ? 3 : 0;
    result += expected3;
    
    int expected4 = 900;
    bool success4 = atomic_compare_exchange_weak_explicit(&a32, &expected4, 1000,
                                                         MEMORY_ORDER_RELEASE,
                                                         MEMORY_ORDER_ACQUIRE);
    result += success4 ? 4 : 0;
    result += expected4;
    
    /* 8. 测试atomic_fetch_add */
    old1 = atomic_fetch_add(&a32, 10);
    result += old1;
    
    old2 = atomic_fetch_add_explicit(&a32, 20, MEMORY_ORDER_SEQ_CST);
    result += old2;
    
    /* 9. 测试atomic_fetch_sub */
    old1 = atomic_fetch_sub(&a32, 5);
    result += old1;
    
    old2 = atomic_fetch_sub_explicit(&a32, 5, MEMORY_ORDER_SEQ_CST);
    result += old2;
    
    /* 10. 测试atomic_fetch_or */
    uint32 old3 = atomic_fetch_or(&u32, 0xFF00FF00);
    result += old3;
    
    old3 = atomic_fetch_or_explicit(&u32, 0x00FF00FF, MEMORY_ORDER_SEQ_CST);
    result += old3;
    
    /* 11. 测试atomic_fetch_xor */
    old3 = atomic_fetch_xor(&u32, 0xAAAAAAAA);
    result += old3;
    
    old3 = atomic_fetch_xor_explicit(&u32, 0x55555555, MEMORY_ORDER_SEQ_CST);
    result += old3;
    
    /* 12. 测试atomic_fetch_and */
    old3 = atomic_fetch_and(&u32, 0xFFFF0000);
    result += old3;
    
    old3 = atomic_fetch_and_explicit(&u32, 0x0000FFFF, MEMORY_ORDER_SEQ_CST);
    result += old3;
    
    /* 13. 测试atomic_fetch_nand */
    old3 = atomic_fetch_nand(&u32, 0xFFFFFFFF);
    result += old3;
    
    old3 = atomic_fetch_nand_explicit(&u32, 0x00000000, MEMORY_ORDER_SEQ_CST);
    result += old3;
    
    /* 14. 测试atomic_fetch_max */
    old1 = atomic_fetch_max(&a32, 2000);
    result += old1;
    
    old1 = atomic_fetch_max_explicit(&a32, 3000, MEMORY_ORDER_SEQ_CST);
    result += old1;
    
    /* 15. 测试atomic_fetch_min */
    old1 = atomic_fetch_min(&a32, 500);
    result += old1;
    
    old1 = atomic_fetch_min_explicit(&a32, 250, MEMORY_ORDER_SEQ_CST);
    result += old1;
    
    /* 16. 测试原子标志操作 */
    bool flag_old = atomic_flag_test_and_set(&af);
    result += flag_old ? 10 : 0;
    
    flag_old = atomic_flag_test_and_set_explicit(&af, MEMORY_ORDER_ACQUIRE);
    result += flag_old ? 20 : 0;
    
    atomic_flag_clear(&af);
    atomic_flag_clear_explicit(&af, MEMORY_ORDER_RELEASE);
    
    /* 17. 测试内存栅栏 */
    atomic_thread_fence(MEMORY_ORDER_SEQ_CST);
    atomic_thread_fence(MEMORY_ORDER_ACQ_REL);
    atomic_signal_fence(MEMORY_ORDER_SEQ_CST);
    
    /* 18. 测试免锁检查 */
    bool lock_free1 = atomic_is_lock_free(sizeof(atomic_int32));
    result += lock_free1 ? 30 : 0;
    
    bool lock_free2 = atomic_is_lock_free(sizeof(atomic_int64));
    result += lock_free2 ? 40 : 0;
    
    /* 19. 测试kill_dependency */
    int value = 100;
    int no_dep = kill_dependency(value);
    result += no_dep;
    
    /* 20. 测试其他原子类型的基本操作 */
    atomic_store(&a8, 50);
    result += atomic_load(&a8);
    
    atomic_store(&a16, 60);
    result += atomic_load(&a16);
    
    atomic_store(&a64, 70);
    result += atomic_load(&a64);
    
    atomic_store(&u8, 80);
    result += atomic_load(&u8);
    
    atomic_store(&u16, 90);
    result += atomic_load(&u16);
    
    atomic_store(&an, 100);
    result += atomic_load(&an);
    
    atomic_store(&un, 110);
    result += atomic_load(&un);
    
    atomic_store(&ab, false);
    result += atomic_load(&ab) ? 1 : 0;
    
    /* 21. 测试所有内存顺序枚举值的使用 */
    memory_order orders[] = {
        MEMORY_ORDER_RELAXED,
        MEMORY_ORDER_CONSUME,
        MEMORY_ORDER_ACQUIRE,
        MEMORY_ORDER_RELEASE,
        MEMORY_ORDER_ACQ_REL,
        MEMORY_ORDER_SEQ_CST
    };
    
    /* 对每个内存顺序进行简单测试 */
    for (int i = 0; i < 6; i++) {
        atomic_store_explicit(&a32, i * 100, orders[i]);
        result += atomic_load_explicit(&a32, orders[i]);
    }
    
    return result;
}

extern void print_bytes(const char8* buf,uint64 size);
extern void sleep();

void line(const char8* str)
{
    uintn size=0;
    while(str[size]!=0)
    {
        size++;
    }
    print_bytes(str,size);
    print_bytes("\n",1);
}

/**
 * 重置所有AP。
 * 
 * @param params 启动参数。
 * 
 * @return 无返回值。
 */
static void init_reset_all_aps(aos_boot_params* params)
{
    if(params->state.apic==AOS_APIC_NO_APIC)
    {
        return;
    }
    else if(params->state.apic==AOS_APIC_XAPIC)
    {
        uint64 apic_base=read_msr(0x1B)&0xFFFFFFFFFFFFF000UL;
        uint32* icr_low=(uint32*)(apic_base+0x300);
        uint32* icr_high=(uint32*)(apic_base+0x310);

        *icr_high=0;
        uint32 command=(5<<8)|BIT14|(3<<18);
        *icr_low=command;
        memory_barrier();
    }
    else
    {
        write_msr(0x830,(5<<8)|BIT14|(3<<18));
    }
}

/**
 * 内核模块入口。
 * 
 * @param params 启动参数。
 * 
 * @return 不再返回。
 */
noreturn void aos_kernel_entry(aos_boot_params* params)
{
    init_reset_all_aps(params);
    while(true)
    {
        sleep();
        panic("fault");
    }
}