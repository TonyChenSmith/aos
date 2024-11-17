@echo off
qemu-system-x86_64 -machine q35 -m 1G -cpu Skylake-Client-v4,+pdpe1gb -smp 4,sockets=1,cores=2 -pflash "OVMF.fd" -hda ./disk/hda.vhd -debugcon file:debug.log -global isa-debugcon.iobase=0x402 -device VGA,edid=on,xmax=1920,xres=1920,ymax=1080,yres=1080 -monitor stdio
@rem ,+la57,  -device VGA,edid=on,xmax=1920,xres=1920,ymax=1080,yres=1080 -serial stdio -cpu Skylake-Client-v2 vc:1920x1080 ,vgamem_mb=256 -serial none -debugcon file:debug.log -global isa-debugcon.iobase=0x402
@echo on