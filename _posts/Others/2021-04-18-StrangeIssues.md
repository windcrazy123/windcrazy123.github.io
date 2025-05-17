---
layout: post
title: "疑难杂症"
date: 2021-04-18 09:00:00 +0800 
categories: Other
tag: Issues
---
* content
{:toc #markdown-toc}

这是一些奇奇怪怪的问题

<!-- more -->

# 一、Assertion failed: PIEInstanceID != INDEX_NONE

> 问题重现和参考：[Crash on loading level with Landscape by 2 PIE instances](https://issues.unrealengine.com/issue/UE-156921)
> [Assertion failed: PIEInstanceID != INDEX_NONE [File:D:\build++UE5\Sync\Engine\Source\Runtime\CoreUObject\Private\UObject\LazyObjectPtr.cpp]](https://forums.unrealengine.com/t/assertion-failed-pieinstanceid-index-none/563742)

如果你想在多人游戏中添加`Landscape`，你的map需要是有`World Partition`的。我试过了，可以！你可以参考官方文档：[World Partition](https://dev.epicgames.com/documentation/en-us/unreal-engine/world-partition-in-unreal-engine?application_version=5.0)

# 二、Smart Link Proxy有时不工作（未解决）

> 问题解决办法参考：[Ai Move To, Nav Proxy Not Firing. 20+ things to try](https://forums.unrealengine.com/t/fixed-ai-move-to-nav-proxy-not-firing-20-things-to-try/270575)
>

都试过了，而且上面还有没提到的我也试过，就是不知道为什么有时候不工作

# 三、Microsoft.MakeFile.targets(44, 5): [MSB3073]

> 问题参考与解决办法：[how can i solve Microsoft.MakeFile.targets(44, 5): MSB3073](https://forums.unrealengine.com/t/how-can-i-solve-microsoft-makefile-targets-44-5-msb3073/915780)

使用编译环境：
```
Visual Studio Community 2022 17.10.3
Unreal Engine 5.3.2
JetBrains Rider 2024.1.4 RD-241.18034.76
Using Visual Studio 2022 14.38.33139 toolchain
```

编译的时候报错：
```ABAP
10>Microsoft.MakeFile.Targets(44,5): Error MSB3073 : 命令“D:\UE_5.3\Engine\Build\BatchFiles\Build.bat NetworkTeachingEditor Win64 Development -Project="C:\Users\My\Documents\Unreal Projects\NetworkTeaching\NetworkTeaching.uproject" -WaitMutex -FromMsBuild”已退出，代码为 6。
```

我把以下文件/文件夹删除后，右键.uproject文件然后点击Generate Visual Studio project files，重新编译就没有这个错误了，这个对我来说是有用的

```
.idea
.vs
Binaries
DerivedDataCache
Intermediate
Saved
.vsconfig
NetworkTeaching.sln
```

# 四、BuildGraph.AutoMation报错

> 问题参考与解决办法：[Error compiling the Automation Tool after updating Visual Studio today (Unreal 5.3.2)](https://forums.unrealengine.com/t/error-compiling-the-automation-tool-after-updating-visual-studio-today-unreal-5-3-2/1393088)

使用编译环境：
```
Visual Studio Community 2022 17.10.3
Unreal Engine 5.3.2
JetBrains Rider 2024.1.4 RD-241.18034.76
Using Visual Studio 2022 14.38.33139 toolchain
```

编译的时候报错：
```ABAP
  BgScriptReader.cs(1642, 38): [CS8604] “void HashSet<string>.UnionWith(IEnumerable<string> other)”中的形参“other”可能传入 null 引用实参。
```

修改源码：BgScriptReader.cs在第1642行左右源码没有`if (users != null)`这句加上就可以了。

```c#
if (_graph.NameToReport.TryGetValue(reportName, out report))
{
    if (users != null)
    {
        report.NotifyUsers.UnionWith(users);
    }
}
```

# 五、Failed to generate project model definition files

```
Failed to generate project model definition files for 'Configuration: Development Editor, Platform: Win64'
```

此消息表明 Rider for Unreal Engine 无法从 UnrealBuildTool 获取项目属性，大概率是.Target.cs和Editor.Target.cs有问题，可先从他俩身上找问题

# 六、报错No corresponding file with the 'Precompiled Header' property set to 'Create' (/Yc) is found

不知道具体什么原因导致的，但是将Binaries，Intermediate，Saved包括Plugins文件夹内所有的临时文件夹删除后重新生成后就没有这个报错警告了

# 七、预计算可视性体积没用Precomputed Visibility Volume NOT WORK

> 引擎版本5.3.2

找到问题可能是引擎功能丢失：[                     UE5.3~5.4 Precomputed Visibility Volume 功能丢了，希望反馈下                   ](https://forums.unrealengine.com/t/ue5-3-5-4-precomputed-visibility-volume/1589792)

还有引擎代码提交[https://github.com/EpicGames/UnrealEngine/pull/11304](https://github.com/EpicGames/UnrealEngine/pull/11304)

