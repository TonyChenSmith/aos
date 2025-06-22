@rem
@rem Copyright (c) 2024-2025 Tony Chen Smith
@rem
@rem SPDX-License-Identifier: MIT
@rem
@echo off
qemu-system-x86_64 -machine q35 -cpu Skylake-Client-v4,+pdpe1gb -smp 4,sockets=1,cores=2 -m 4G -drive if=pflash,format=raw,unit=0,readonly=on,file=OVMF_CODE.fd -drive if=pflash,format=raw,unit=1,file=OVMF_VARS.fd,readonly=off -hda hda.vhd -debugcon file:debug.log -global isa-debugcon.iobase=0x402 -device VGA,edid=on,xmax=1920,xres=1920,ymax=1080,yres=1080 -monitor stdio
@rem ,+la57, -device intel-iommu
@echo on