/**
 * 内核UEFI运行时固件服务。x86平台的唯一配置。
 * @date 2026-04-19
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include <firmware/status.h>
#include <init/params.h>
#include <panic/callback.h>
#include <support/sync.h>

/**
 * UEFI运行时服务。
 */
static aos_efi_runtime_services* runtime=null;

/**
 * 运行时服务锁。
 */
static spinlock lock;

/**
 * 固件恐慌回调。这里主要是手动解锁运行时服务，保证恐慌时能正常使用运行时服务。
 * 
 * @return 无返回值。
 */
static void firmware_panic_callback(void)
{
    spinlock_unlock(&lock);
}

/**
 * 固件恐慌回调结点。
 */
static panic_callback_node firmware_node={null,null,firmware_panic_callback};

/**
 * 通过启动参数初始化固件模块。
 * 
 * @param params 启动参数。
 * 
 * @return 无返回值。
 */
void kernel_firmware_init(aos_boot_params *params)
{
    spinlock_init(&lock);
    runtime=params->system_table->runtime;
    register_panic_callback(&firmware_node);
}

/**
 * 主要用到的EFI状态。
 */
#define EFI_SUCCESS ((uintn)(0))
#define EFI_INVALID_PARAMETER ((uintn)(MAX_UINTN_BIT|(2)))
#define EFI_UNSUPPORTED ((uintn)(MAX_UINTN_BIT|(3)))
#define EFI_BUFFER_TOO_SMALL ((uintn)(MAX_UINTN_BIT|(5)))
#define EFI_DEVICE_ERROR ((uintn)(MAX_UINTN_BIT|(7)))
#define EFI_WRITE_PROTECTED ((uintn)(MAX_UINTN_BIT|(8)))
#define EFI_OUT_OF_RESOURCES ((uintn)(MAX_UINTN_BIT|(9)))
#define EFI_NOT_FOUND ((uintn)(MAX_UINTN_BIT|(14)))
#define EFI_SECURITY_VIOLATION ((uintn)(MAX_UINTN_BIT|(26)))

/**
 * 将固件状态转为EFI状态。
 * 
 * @param status 固件状态。
 * 
 * @return 对应EFI状态。
 */
static uintn firmware_firmware_to_efi_status(firmware_status status)
{
    switch(status)
    {
        case FIRMWARE_SUCCESS:
            return EFI_SUCCESS;
        case FIRMWARE_UNSUPPORTED:
        default:
            return EFI_UNSUPPORTED;
    }
}

/**
 * 将EFI状态转为固件状态。
 * 
 * @param status EFI状态。
 * 
 * @return 对应EFI状态。
 */
static firmware_status firmware_efi_to_firmware_status(uintn status)
{
    switch(status)
    {
        case EFI_SUCCESS:
            return FIRMWARE_SUCCESS;
        case EFI_UNSUPPORTED:
        default:
            return FIRMWARE_UNSUPPORTED;
    }
}

/**
 * 通过固件服务将系统冷重置。
 * 
 * @param status 重置状态码。
 * 
 * @return 支持冷重置不返回，不支持返回不支持。
 */
firmware_status firmware_cold_reset(firmware_status status)
{
    spinlock_lock(&lock);
    runtime->reset_system(AOS_EFI_RESET_COLD,firmware_firmware_to_efi_status(status),0,null);
    spinlock_unlock(&lock);
    return FIRMWARE_SUCCESS;
}

/**
 * 通过固件服务将系统热重置。
 * 
 * @param status 重置状态码。
 * 
 * @return 支持热重置不返回，不支持返回不支持。
 */
firmware_status firmware_warm_reset(firmware_status status)
{
    spinlock_lock(&lock);
    runtime->reset_system(AOS_EFI_RESET_WARM,firmware_firmware_to_efi_status(status),0,null);
    spinlock_unlock(&lock);
    return FIRMWARE_SUCCESS;
}

/**
 * 通过固件服务将系统关机。
 * 
 * @param status 重置状态码。
 * 
 * @return 支持关机不返回，不支持返回不支持。
 */
firmware_status firmware_shutdown(firmware_status status)
{
    spinlock_lock(&lock);
    runtime->reset_system(AOS_EFI_RESET_SHUTDOWN,firmware_firmware_to_efi_status(status),0,null);
    spinlock_unlock(&lock);
    return FIRMWARE_SUCCESS;
}