# AOS

一个简单的操作系统，由UEFI引导，平台x64。目前只在编写和UEFI接轨的部分，还未执行`ExitBootServices()`。**AOS**只是该项目暂定名字，待项目确定定型时,会新建仓库并在该文档中添加链接。  

该项目将一直遵循`MIT License`，直到定型的新仓库建立并停止提交该仓库。  

目录对应如下：
1. `edk2-stable202402`-EDK2开发工具，加上自己的项目。
1. `java`-AOS相关Java项目。
1. `kernel`-内核项目。

在目录`edk2-stable202402`下，目前与本项目相关包如下：
1. `ToolPkg`-工具包。编译出来的EFI文件都是`UEFI_APPLICATION`，在UEFI Shell中检查系统信息，或是测试小功能。现在组件`aos.uefi.tsl`作为AOS的`UEFI Bootstrap`编译包使用。

在目录`java`下，目前有以下项目：
1. `regex`-项目regex。内有aos.library.regex模块，为AOS Java流正则库。将用于编译器词法分析器实现。
1. `compiler`-项目compiler。内有aos.tools.compiler模块，为AOS Java自定义编译器。

在目录`kernel`下，目前有以下组件项目：

由于分类和实际使用的区别，组件与项目名（或文件名）有以下对应关系：
|组件|项目名|目标文件名|说明|
|:---:|:---:|:---:|:---:|
|`aos.uefi.tsl`|`${EDK2}/ToolPkg/Core/aosuefilts.inf`|`/EFI/BOOT/BOOTX64.EFI`|UEFI引导器。|
|`aos.bootstrap`|`${kernel}/boot/core`|`/aos/boot/boot.core.mod`|ESP分区内系统引导内核。|
