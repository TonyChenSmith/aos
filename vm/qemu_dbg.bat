@echo off
qemu-system-x86_64 -machine q35 -m 4G -cpu Skylake-Client-v4,+pdpe1gb -smp 4,sockets=1,cores=2 -pflash "OVMF.fd" -hda ./disk/hda.vhd -debugcon file:debug.log -global isa-debugcon.iobase=0x402 -device VGA,edid=on,xmax=1920,xres=1920,ymax=1080,yres=1080 -monitor stdio -s -S
@echo on