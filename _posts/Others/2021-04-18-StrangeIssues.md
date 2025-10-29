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

## 代码为6

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

## 代码为8

同上述编译环境

编译报错：

```ABAP
28>Expecting to find a type to be declared in a module rules named 'HoudiniEngine' in 'UE5Rules, Version=0.0.0.0, Culture=neutral, PublicKeyToken=null'.  This type must derive from the 'ModuleRules' type defined by UnrealBuildTool.
28>D:\VS\VS\MSBuild\Microsoft\VC\v170\Microsoft.MakeFile.Targets(44,5): error MSB3073: 命令“D:\UE\UE_5.4\Engine\Build\BatchFiles\Build.bat WemakeMarsMREditor Win64 DebugGame -Project="E:\Work\2025\MR_mars1\WemakeMarsMR.uproject" -WaitMutex -FromMsBuild -architecture=x64”已退出，代码为 8。
28>已完成生成项目“WemakeMarsMR.vcxproj”的操作 - 失败。
```

解决方法：`HoudiniEngine`插件不能放在UE5的Engine文件夹中,可以放在项目的插件中。

之前RiderLink也出现过这种问题

> 参考链接：[HoudiniEngine在UE5中创建C++工程时会报错](https://zhuanlan.zhihu.com/p/539734936)
>
> [UE5 Rider编译项目 RD报错](https://zhuanlan.zhihu.com/p/11329869625)

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

# 八、未找到合适的转换来编码或解码内容。

> <font color=red>LogWmfMedia: Error: An error occurred in the media session: 未找到合适的转换来编码或解码内容。</font>

遇到问题是由于UE导入的6k视频在MediaPlayer中无法播放，双击视频就会报上面的错误，原因是缺少解码器，经实验是缺少H265HEVC视频格式支持。

[来自设备制造商的 HEVC 视频扩展](https://apps.microsoft.com/detail/9n4wgh0z6vhq?hl=zh-cn&gl=CN)

> 不能下载的看这里
> 1，[store.rg-adguard.net/](store.rg-adguard.net/)这个网站选择productid，输入:`9n4wgh0z6vhq`，搜索后选择合适的版本下载即可安装。或者选择url，复制微软商店里的链接粘贴进去也能下载。
> 2，下载appxbundle后缀的安装包双击安装就可以了

> 参考：[Image and video codecs and import formats](https://dev.epicgames.com/community/learning/knowledge-base/yzP1/capturing-reality-image-and-video-codecs-and-import-formats)

# 九、使用5.2.1时启用OpenXR崩溃

> 问题详情与解决方案：[Crash when opening project on 5.2.1 with OpenXR plugin enabled](https://forums.unrealengine.com/t/crash-when-opening-project-on-5-2-1-with-openxr-plugin-enabled/1238892)

使用5.2.1时启动游戏崩溃并报错：`Assertion failed: ((Result) >= 0)[File:D:\build++UE5\Sync\Engine\Plugins\Runtime\OpenXR\Source\OpenXRHMD\Private\OpenXRCore.cpp] [Line: 24]`

使用 [OpenXR Explorer](https://github.com/maluoi/openxr-explorer)设置一下 `OpenXR runtime`，如图

![](D:\WindCrazyGithubio\windcrazy123.github.io\styles\images\Other\OpenXRExplorer.png)

# 十、Source package referenced an object in target package but the target package was marked NeverCook or is not cookable for the target platform.

```ABAP
UATHelper: Packaging (Android (ASTC)): LogCook: Error: Content is missing from cook. Source package referenced an object in target package but the target package was marked NeverCook or is not cookable for the target platform.
PackagingResults: Error: Content is missing from cook. Source package referenced an object in target package but the target package was marked NeverCook or is not cookable for the target platform.
UATHelper: Packaging (Android (ASTC)):     Source package: /Game/LV01_02/Art/SciFiIndustrialBase/Materials/MI_Blue_Decals
UATHelper: Packaging (Android (ASTC)):     Target package: /Bridge/MSPresets/M_MS_Glass_Material/Textures/White
UATHelper: Packaging (Android (ASTC)):     Referenced object: /Bridge/MSPresets/M_MS_Glass_Material/Textures/White.White
```

原因就是Bridge的`SupportedTargetPlatforms`没有"Android"这一项，现在加上就ok了

```xml
"SupportedTargetPlatforms": [
		"Win64",
		"Mac",
		"Linux",
		"Android"
	],
```

# 十一、屏幕时不时闪黑，UE右键菜单栏不显示

> 参考：[关闭OverlayTestMode，终于解决困扰了我好久游戏切换浏览器卡死掉驱动的问题](https://ngabbs.com/read.php?tid=44051581&rand=20)

注册表：

```ini
Windows Registry Editor Version 5.00

[HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\Dwm]
"OverlayTestMode"=dword:00000005
```

