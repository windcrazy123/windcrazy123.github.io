---
layout: post
title: "遏制基于渲染的GPU崩溃"
date: 2024-1-16 09:00:00 +0800 
categories: UE
tag: Misc
---
* content
{:toc #markdown-toc}

在处理含有大量图形的项目时，你有可能会遇到GPU崩溃。下面方法能够很好地遏制基于渲染的GPU崩溃。

<!-- more -->
参考：[虚幻引擎：如何修复GPU驱动程序崩溃](https://mp.weixin.qq.com/s/e5l9XtfwEFWgwhHi1b2idg)
[Jeffrey Zhuang：什么是TDR Issue 及修改方法](https://zhuanlan.zhihu.com/p/38141415)

# 遇到的问题

详情：[Assertion failed: CurrentGPUVirtualAddress != 0](https://forums.unrealengine.com/t/5-0-ea-i-have-constant-crashes-average-every-5-mins-i-tried-almost-every-solution/236511) 

可能还会出现这个提示：
<img src="{{ '/styles/images/Miscellaneous/遏制基于渲染的GPU崩溃/GPUCrash.png' | prepend: site.baseurl }}" />

简要：[File:D:\build\++UE5\Sync\Engine\Source\Runtime\D3D12RHI\Private\D3D12DescriptorCache.cpp] [Line: 729]

# 初次解决办法

怀疑：RHI报错可能是DX12在UE5不稳定

解决办法：Edit->Project Setting->Platforms-Windows->Targeted RHIs->Default RHI->DirectX11

> 反正改过之后没再崩溃了

# 再次解决(虚幻引擎公众号提供)

## **发生GPU崩溃的原因**

为防止应用程序因使用过多内存而锁死，Windows实施了保护措施。如果一个应用程序的渲染时间超过几秒，Windows就会杀死GPU驱动程序，导致应用程序崩溃。在虚幻引擎这样的应用程序中，无法知道渲染进程的耗时，因此无法在应用程序层面避免崩溃。

## **如何解决此崩溃**

在开发项目期间，遇到GPU崩溃的情况并不罕见。但是，有一种方法可在开发过程中避免这种类型的崩溃，就是编辑Windows注册表项，让系统有更多时间运行渲染进程。在本指南中，你将创建两个新的注册表项：TdrDelay和TdrDdiDelay 。

- TdrDelay用于设置超时阈值。即负责处理和存储（VRAM）的GPU调度程序发出抢占请求时，GPU将此请求延迟的秒数。
- TdrDdiDelay用于设置操作系统（OS）允许线程离开驱动程序的时长。该时长耗尽之后，将发生超时延迟故障。

> 要进一步了解注册表项，请查阅Microsoft关于Tdr注册表项的官方文档：
>
> https://learn.microsoft.com/en-us/windows-hardware/drivers/display/tdr-registry-keys

```ABAP
WARNING: 在Windows操作系统上更改注册表项，可能会产生意外的结果，并需要彻底重新安装Windows。尽管在本教程中添加或编辑注册表项应该不会导致这些结果，但我们推荐你在备份系统之后再继续操作。若因修改系统注册表给系统造成损害，Epic Games概不负责。[我也不负责:)]
```

你需要将两个注册表项添加到显卡驱动。执行以下步骤来添加注册表项。

1. 在Windows操作系统搜索栏中输入"run"。打开运行（Run）应用程序。(或使用win+R键)

2. 在搜索字段中，输入"regedit"。点击确定（OK）打开注册表编辑工具。

3. 在注册表编辑工具左侧导航栏中找到GraphicsDrivers分段。此项的位置是Computer\HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\GraphicsDrivers。
    <img src="{{ '/styles/images/Miscellaneous/遏制基于渲染的GPU崩溃/regedit0.png' | prepend: site.baseurl }}" />
>注册表项需要添加到GraphicsDrivers文件夹，而不是其子文件夹。请务必选择正确的文件夹。

4. 你需要的注册表项称为TdrDelay。如果该注册表项已存在，请双击进行编辑。如果尚未存在，请右键点击右侧的窗格，并选择新建（New）> DWORD (32 位)值（DWORD (32-bit) Value）。<img src="{{ '/styles/images/Miscellaneous/遏制基于渲染的GPU崩溃/regedit1.png' | prepend: site.baseurl }}" />
5. 将基数（Base）设置为十进制（Decimal）。将TdrDelay的值（Value）设置为60。点击确定（OK）完成。<img src="{{ '/styles/images/Miscellaneous/遏制基于渲染的GPU崩溃/regedit2.png' | prepend: site.baseurl }}" />
6. 你需要称为TdrDdiDelay的第二个注册表项。如果该注册表已存在，请双击进行编辑。如果尚未存在，请右键点击右侧的窗格，并选择新建（New） > DWORD (32 位)值（DWORD (32-bit) Value）进行创建。
7. 将基数（Base）设置为十进制（Decimal）。将TdrDdiDelay的值（Value）设置为60。点击确定（OK）完成。
8. 你的注册表现在应该包括TdrDelay和TdrDdiDelay。<img src="{{ '/styles/images/Miscellaneous/遏制基于渲染的GPU崩溃/regedit3.png' | prepend: site.baseurl }}" />
9. 关闭注册表编辑器。
10. 重启计算机，使这些更改生效。

## **结果**

添加这些注册表项之后，Windows现在将等待60秒，再确定应用程序的渲染进程是否耗时太久。如果你仍遇到类似的GPU崩溃，请将注册表项TdrDelay和TdrDdiDelay中的值（Value）从60更改为120秒。

虽然这种方法能够很好地遏制基于渲染的GPU崩溃，但并不能解决所有崩溃。如果你尝试同时处理太多数据，无论你将超时延迟设置得多长，GPU都可能会超时。该解决方案只是给你的显卡稍微多提供了一点时间。

# 什么是TDR

TDR (Timeout Detection and Recovery) 超时侦测与恢复。

Windows Vista 之后的版本会检查显卡不响应的情况，并通过重置显卡，保证操作系统继续正常运行。
操作系统在一定时间内没有收到显卡的响应，便会触发重置。
响应超时的默认时间是2秒
TDR 特性推出以前，遇到显卡未响应只能通过重启解决。而如今一旦遇到显卡问题你将会看到这样的提示

> Display driver stopped responding and has recovered
> 显卡驱动程序已停止响应，并且已恢复

这个特性对于一般用户而言是非常有用。
但对于开发、调试GPU并行计算，
使用某些使用GPU运算的软件，例如Substance Painter，
默认的TDR太短了。



TdrDelay是允许GPU不处理调度器抢占请求的最大时间

TdrDdiDelay是运行线程停留在显卡驱动调用的最大时间