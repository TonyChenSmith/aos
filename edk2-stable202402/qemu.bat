@echo off
qemu-system-x86_64 -bios OVMF_S.fd -hda fat:rw:F:/GitHub/edk2-stable202402/disk -serial none -m 1G -device VGA,edid=on,xmax=1920,xres=1920,ymax=1080,yres=1080 -monitor vc:1920x1080
@rem -device VGA,edid=on,xmax=1920,xres=1920,ymax=1080,yres=1080  -serial stdio
@echo on