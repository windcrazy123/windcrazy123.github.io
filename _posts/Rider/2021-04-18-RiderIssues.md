---
layout: post
title: "使用Rider的一些问题"
date: 2021-04-18 09:00:00 +0800 
categories: Rider
tag: Misc
---
* content
{:toc #markdown-toc}

使用Rider时遇到的一些问题，并给出一些解决方法。

<!-- more -->

# 一、UE配置Source Code中无法关联Rider

![](/styles/images/Rider/UESouceCodeRider.png)

解决办法：在UE安装路径下的RiderLocations.txt文件中修改或增加Rider最新路径即可。

```
Epic Games\UE_4.27\Engine\Plugins\Developer\RiderSourceCodeAccess\Resources\RiderLocations.txt
```

> 参考：https://www.cnblogs.com/chevin/p/16083202.html
>
> 官方传送门：[No Rider IDE option in UE 4.27.1](https://rider-support.jetbrains.com/hc/en-us/community/posts/4410229042066-No-Rider-IDE-option-in-UE-4-27-1)

# 二、'RiderLink' failed to load because module ‘RD’ couldn't be loaded

> 参考：[17.10 不支持UE5](https://developercommunity.visualstudio.com/t/1710-not-supporting-the-UE5/10668890?sort=active)

我的编译环境：

```
Visual Studio Community 2022 17.10.3
Unreal Engine 5.1.1/Unreal Engine 5.3.2
JetBrains Rider 2024.1.4 RD-241.18034.76
```

在安装Rider并打开UE项目后会让你下载RiderLink plugin

![](/styles/images/Rider/InstallRiderLinkWarning.png)

点击Install plugin in Engine开始下载并编译，开始的时候出现

```
Building UnrealEditor...
Using Visual Studio 2022 14.40.33811 toolchain (C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.40.33807) and Windows 10.0.22621.0 SDK (C:\Program Files (x86)\Windows Kits\10).
```

在编译中途出现

```
Detected compiler newer than Visual Studio 2022, please update min version checking in WindowsPlatformCompilerSetup.h
```

在最后出现(有时会编译不完直接失败，但是报错信息我没有收集)

```
BUILD SUCCESSFUL
AutomationTool executed for 0h 3m 55s
AutomationTool exiting with ExitCode=0 (Success)
RiderLink plugin installed
```

虽然看起来没有问题，但是在点击运行项目后会卡在75%加载插件的一步然后出现崩溃报错。

![](/styles/images/Rider/RiderLinkError.png)

![](/styles/images/Rider/UECrashasRiderLink.png)

## 解决问题

问题出现在

```
Detected compiler newer than Visual Studio 2022, please update min version checking in WindowsPlatformCompilerSetup.h
```

既然说编译器版本高，那么我们就降低编译器版本。

1. 转到 Visual Studio 安装程序 -> 修改 -> 单个组件->添加“MSVC v143 - VS 2022 c++ x64/x86 build tools (v14.38-17.8)”![](/styles/images/Rider/VSInstallMSVC.png)
2. 可以通过转到 VS 安装路径并查看文件夹是否存在来确认它是否安装了正确的文件夹（*C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Tools\MSVC\14.38.33130*）
3. 将“C:\Users\My\AppData\Roaming\Unreal Engine\UnrealBuildTool\BuildConfiguration.xml”更改为

```xml
<?xml version="1.0" encoding="utf-8" ?>
<Configuration xmlns="https://www.unrealengine.com/BuildConfiguration">
	<WindowsPlatform>
		<CompilerVersion>14.38.33130</CompilerVersion>
		<!-- <ToolchainVersion>14.38.33130</ToolchainVersion> -->
	</WindowsPlatform>
</Configuration>
```

将Rider和UE关闭，把安装的RiderLink删除(安装到引擎：*C:\Program Files\Epic Games\UE_5.1\Engine\Plugins\Marketplace\Developer*)

重启Rider会提示你再次安装(如果没有提示，Rider->File->Settings->Languages & Frameworks->Unreal Engine->Install RiderLink in Engine)，那么此时安装编译开时始会出现

```
Building UnrealEditor...
Using Visual Studio 2022 14.38.33139 toolchain (C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.38.33130) and Windows 10.0.22621.0 SDK (C:\Program Files (x86)\Windows Kits\10).
```

并且在编译过程中不在出现编译器版本高的信息，并且可以正常运行UE。(如果不成功将BuildConfiguration.xml中的ToolchainVersion子标签的注释去掉，不过去掉之后每次编译都会给出警告，而且去掉注释也不一定成功，因为我加注释和去掉注释都试过全部成功了，如果你不成功，看看是不是有其他报错信息)。

警告信息：

```
Warning：  BuildConfiguration.xml(5): [] The element 'WindowsPlatform' in namespace 'https://www.unrealengine.com/BuildConfiguration' has invalid child element 'ToolchainVersion' in namespace 'https://www.unrealengine.com/BuildConfiguration'. List of possible elements expected: 'bCompilerTrace, MaxNestedPathLength, MaxRootPathLength, bStrictConformanceMode, StaticAnalyzer, StaticAnalyzerOutputType, bStrictEnumTypesConformance, bClangTimeTrace, AdditionalLinkerOptions, bStrictPreprocessorConformance, Compiler, WindowsSdkVersion, bShowIncludes, PCHMemoryAllocationFactor, bStrictInlineConformance, bUpdatedCPPMacro' in namespace 'https://www.unrealengine.com/BuildConfiguration'.
```

# 三、UE 编译时 PCH 虚拟内存不足的错误

在多核 CPU 上编译 UE 时，可能比较频繁的遇到以下错误，尤其是编译的数量较多的时候：

```ABAP
0>c1xx: Error C3859 : 未能创建 PCH 的虚拟内存
	c1xx: note: 系统返回代码 1455: 页面文件太小，无法完成操作。
	c1xx: note: 请访问 https://aka.ms/pch-help 了解更多详情
0>c1xx: Error C1076 : 编译器限制: 达到内部堆限制
  Microsoft.MakeFile.targets(51, 5): [MSB3073] 命令“D:\UE_4.26\Engine\Build\BatchFiles\Rebuild.bat MyProjectEditor Win64 Development -Project="D:\UEProject\MultiGame\Multi_Test\MyProject.uproject" -WaitMutex -FromMsBuild”已退出，代码为 -1。
```

默认情况下，UE 会拉起系统最大线程数量的编译进程。就比如我电脑是16个

![](/styles/images/Rider/16processes.png)

UE 里每个编译任务的 `/zm` 值为 1000：[VCToolChain.cs?q=%2Fzm#L354](https://cs.github.com/EpicGames/UnrealEngine/blob/d11782b9046e9d0b130309591e4efc57f4b8b037/Engine/Source/Programs/UnrealBuildTool/Platform/Windows/VCToolChain.cs?q=/zm#L354)
表示每个 cl 进程会分配 750M 的虚拟内存：[/Zm (Specify precompiled header memory allocation limit)](https://docs.microsoft.com/en-us/cpp/build/reference/zm-specify-precompiled-header-memory-allocation-limit?view=msvc-160)

| Value of *`factor`* | Memory allocation limit |
| :------------------ | :---------------------- |
| 10                  | 7.5 MB                  |
| 100                 | 75 MB                   |
| 200                 | 150 MB                  |
| 1000                | 750 MB                  |
| 2000                | 1500 MB                 |

如果ue使用的虚拟内存大于系统的虚拟内存，就会导致分配失败
*TaskCount ∗ PCHMemoryAllocationFactor > SystemVirtualMemery*
那么就有两种办法可以解决：一个是减少`TaskCount`，另一个就是减少`PCHMemoryAllocationFactor`。这两种办法都可以通过编辑`BuildConfiguration.xml`进行更改

1. 减少TaskCount：[本地执行器和并行执行器](https://docs.unrealengine.com/4.27/en-US/ProductionPipelines/BuildTools/UnrealBuildTool/BuildConfiguration/#localexecutor)

   ```xml
   <LocalExecutor>
   	<MaxProcessorCount>8</MaxProcessorCount>
   </LocalExecutor>
   <ParallelExecutor>
   	<MaxProcessorCount>8</MaxProcessorCount>
   </ParallelExecutor>
   ```

2. 减少PCHMemoryAllocationFactor：[WindowsPlatform->PCHMemoryAllocationFactor](https://docs.unrealengine.com/4.27/en-US/ProductionPipelines/BuildTools/UnrealBuildTool/BuildConfiguration/#windowsplatform)

   ```xml
   <WindowsPlatform>
   	<PCHMemoryAllocationFactor>200</PCHMemoryAllocationFactor>
   </WindowsPlatform>
   ```

### BuildConfiguration.xml

除添加到 `Config/UnrealBuildTool` 文件夹中已生成UE4项目外，虚幻编译工具还会从以下位置（Windows系统）的XML配置文件读取设置：

- Engine/Saved/UnrealBuildTool/BuildConfiguration.xml
- User Folder/AppData/Roaming/Unreal Engine/UnrealBuildTool/BuildConfiguration.xml
- My Documents/Unreal Engine/UnrealBuildTool/BuildConfiguration.xml

如果是Linux和Mac，则会从以下路径读取：

- /Users//.config//Unreal Engine/UnrealBuildTool/BuildConfiguration.xml
- /Users//Unreal Engine/UnrealBuildTool/BuildConfiguration.xml

### Finished

```xml
<?xml version="1.0" encoding="utf-8" ?>
<Configuration xmlns="https://www.unrealengine.com/BuildConfiguration">
    <WindowsPlatform>
        <PCHMemoryAllocationFactor>200</PCHMemoryAllocationFactor>
    </WindowsPlatform>
    <LocalExecutor>
        <MaxProcessorCount>8</MaxProcessorCount>
    </LocalExecutor>
    <ParallelExecutor>
        <MaxProcessorCount>8</MaxProcessorCount>
    </ParallelExecutor>
</Configuration>
```

8*150MB = 1200MB（虽然我改为16也没有超过我系统的虚拟内存大小但还是报错了，**所以此内容可能有误**，但是我如此改完后确实不报错了）

> 注：虚拟内存查看：我的电脑->属性->高级系统设置->高级->性能->设置->高级->虚拟内存
>
> 并且需要注意的是：如果 `/zm` 值设置的太小，可能无法满足 UE 合并翻译单元的要求，导致编译错误，所以，最好还是修改系统虚拟内存大小或者控制并行的任务数量。
>
> 参考：https://ue5wiki.com/wiki/5cc4f8a/
