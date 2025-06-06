/** @file
*
*  Copyright (c) 2023, Ampere Computing LLC. All rights reserved.<BR>
*  Copyright (c) 2013-2017, ARM Limited. All rights reserved.
*
*  SPDX-License-Identifier: BSD-2-Clause-Patent
*
*  @par Reference(s):
*  - Generic Watchdog specification in Arm Base System Architecture 1.0C:
*    https://developer.arm.com/documentation/den0094/c/
**/

#ifndef GENERIC_WATCHDOG_H_
#define GENERIC_WATCHDOG_H_

// Refresh Frame:
#define GENERIC_WDOG_REFRESH_REG  ((UINTN)FixedPcdGet64 (PcdGenericWatchdogRefreshBase) + 0x000)

// Control Frame:
#define GENERIC_WDOG_CONTROL_STATUS_REG      ((UINTN)FixedPcdGet64 (PcdGenericWatchdogControlBase) + 0x000)
#define GENERIC_WDOG_OFFSET_REG_LOW          ((UINTN)FixedPcdGet64 (PcdGenericWatchdogControlBase) + 0x008)
#define GENERIC_WDOG_OFFSET_REG_HIGH         ((UINTN)FixedPcdGet64 (PcdGenericWatchdogControlBase) + 0x00C)
#define GENERIC_WDOG_COMPARE_VALUE_REG_LOW   ((UINTN)FixedPcdGet64 (PcdGenericWatchdogControlBase) + 0x010)
#define GENERIC_WDOG_COMPARE_VALUE_REG_HIGH  ((UINTN)FixedPcdGet64 (PcdGenericWatchdogControlBase) + 0x014)
#define GENERIC_WDOG_IID_REG                 ((UINTN)FixedPcdGet64 (PcdGenericWatchdogControlBase) + 0xFCC)

// Values of bit 0 of the Control/Status Register
#define GENERIC_WDOG_ENABLED   1
#define GENERIC_WDOG_DISABLED  0

#define GENERIC_WDOG_IID_ARCH_REV_SHIFT  16
#define GENERIC_WDOG_IID_ARCH_REV_MASK   0xF

#endif // GENERIC_WATCHDOG_H_
