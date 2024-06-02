# AOS

一个简单的操作系统，由UEFI引导，平台x64。目前只在编写和UEFI接轨的部分，还未执行`ExitBootServices()`。**AOS**只是该项目暂定名字，待项目确定定型时,会新建仓库并在该文档中添加链接。  

该项目将一直遵循`MIT License`，直到定型的新仓库建立并停止提交该仓库。  

目录对应如下：
1. `edk2-stable202402`-EDK2开发工具，加上自己的项目。

在目录`edk2-stable202402`下，与本项目相关包如下：
1. `ToolPkg`-工具包。编译出来的EFI文件都是`UEFI_APPLICATION`，在UEFI Shell中检查系统信息，或是测试小功能。现在作为AOS的`UEFI Bootloader`编译包使用。