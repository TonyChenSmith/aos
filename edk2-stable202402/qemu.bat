@echo off
qemu-system-x86_64 -machine q35 -cpu Skylake-Client-v4,+la57,+pdpe1gb -bios OVMF.fd -hda ./disk/hda.vhd -serial none -m 1G -device VGA,edid=on,xmax=1920,xres=1920,ymax=1080,yres=1080 -monitor stdio
@rem -device VGA,edid=on,xmax=1920,xres=1920,ymax=1080,yres=1080 -serial stdio -cpu Skylake-Client-v2 vc:1920x1080 ,vgamem_mb=256
@echo on