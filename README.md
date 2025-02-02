# AOS

## 目录
1. [简介](#简介)
2. [仓库结构](#仓库结构)
3. [项目说明](#项目说明)

## 简介
AOS是一个运行在x86-64架构处理器上，由UEFI引导的操作系统。AOS是该项
目当前代号，待项目定型时,会新建仓库并在该文档中添加链接。  
该项目当前遵循MIT协议开源。  

## 仓库结构
整个仓库主要目录结构如下图表示。图上项目目录将在下面篇幅中详细说明。
```plaintext
aos/                        - 仓库根目录。
 │
 ├─edk2-stable202402/       - EDK II开发套件。
 │  ├─AOS/                  - AOS包。
 │  └─ToolPkg/              - 工具项目包。
 │     ├─Application/       - 应用项目目录。
 │     │  ├─DisplayAdapter/ - 显示适配器项目。
 │     │  ├─Info/           - 信息项目。
 │     │  └─Memory/         - 内存信息项目。
 │     ├─Core/              - 核心项目目录。
 │     │  ├─aosuefitsl/     - AOS UEFI核模块项目。
 │     │  └─Test/           - 测试项目。
 │     └─Include/           - 头文件导出目录。
 │
 ├─java/                    - Java工具开发目录。
 │  ├─compiler/             - 编译器项目。
 │  ├─rbtree/               - 红黑树项目。
 │  └─regex/                - 流正则库项目。
 │
 ├─kernel/                  - 内核开发目录。
 │  ├─boot/                 - 启动核项目集。
 │  │  ├─core               - 启动核核心模块。
 │  │  ├─include            - 启动核公共头文件目录。
 │  │  ├─init               - 启动核初始化模块。
 │  │  ├─memory             - 启动核内存模块。
 │  │  ├─test               - 启动核测试代码目录。
 │  │  └─util               - 启动核工具模块。
 │  ├─cmake/                - 内核CMake配置。
 │  ├─include/              - 内核公共头文件目录。
 │  └─lib/                  - 内核编译用第三方库。
 │
 └─vm/                      - 测试虚拟机目录。
```
## 项目说明
在下面将会对每个子目录内含有的项目进行说明。其中每个项目当前状态将用
如下表图例说明。
|符号|说明|
|:---:|:---:|
|🟢|项目已完成。|
|🔵|项目测试中。|
|🟡|项目开发中。|
|🟠|项目已暂停。|
|🔴|项目已终止。|
|🟣|项目需重构。|
|⚪|项目待开始。|

1. **AOS-module**[🟢]  
UEFI核，模块名`aos.uefi`。作为AOS在UEFI阶段的引导启动程序，获取启动
核所需启动参数，直到转进启动核。
2. **TookPkg-DisplayAdapter**[🔴]  
显示适配器。实际上是一个空壳子，作为EDK II的一个模块是很好的开始。本
身并没有实现名字所称功能，所以定为终止。
3. **TookPkg-Info**[🟢]  
获取信息的程序。其获取了UEFI内存映射，检查了处理器分页功能并展示出来。
4. **TookPkg-Memory**[🟢]  
获取内存信息的程序。其获取了处理器当前页表详细内容并展示。
5. **TookPkg-aosuefitsl**[🔴]  
曾经使用的UEFI核，模块名`aos.uefi.tsl`。曾经在UEFI阶段获取PCI设备数
据等启动参数信息，在启动核参数改版后该项目停止使用。
6. **TookPkg-Test**[🟢]  
获取设备路径协议并显示的程序。其遍历了所有设备路径协议并将这些协议转化
成字符串输出。
7. **java-compiler**[🟠]  
一个设计支持LR(1)文法分析的编译器框架，用于支持自定义程序语言编译。由
于词法器有一定问题，加上没有合适的语法器实现思路，当前暂停开发。
8. **java-rbtree**[🟢]  
Java版本的红黑树结构。用于对红黑树算法的实验与调试检查。
9. **java-regex**[🟠]  
一个流正则库，用于支持词法器逐字符输入匹配。本身实现问题不大，但是由于
最终使用的是DFA，导致缺乏一些功能，当前同编译器项目一同暂停。
10. **kernel-boot-core**[🔴]  
曾经的启动核核心模块，模块名`aos.boot.core`。原先是设计为系统到启动核
后进入的第一个模块，管控系统在启动核阶段的一切核心事务。但在该模块落地
后不久就变成了娱乐模块，仅有一些对启动参数的检查与信息的输出函数。由于
后续设计对功能逐渐细分，认为不应该出现一个如此高度抽象的核心模块，该项
目就此定为终止。
11. **kernel-boot-init**[🟡]  
启动核初始化模块，模块名`aos.boot.init`。负责从UEFI核接管系统资源，构
造结束启动服务后的初始化进程。
12. **kernel-boot-memory**[🟡]  
启动核内存模块，模块名`aos.boot.memory`。负责管理启动核的内存资源，包
括物理内存图、分页表和虚拟内存图。
13. **kernel-boot-util**[🔵]  
启动核工具模块，模块名`aos.boot.util`。负责为启动核提供工具函数。该模
块在开发过程中将逐步添加需求，因此标记为测试状态。