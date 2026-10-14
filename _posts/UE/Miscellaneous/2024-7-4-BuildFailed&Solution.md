---
layout: post
title: "编译问题及解决办法"
date: 2024-7-4 09:00:00 +0800 
categories: UE
tag: Misc
---
* content
{:toc #markdown-toc}
~~在新电脑配置Android打包环境后打包可能出现各种各样的错误，本篇将由于Gradle而出现的错误，进行分享~~。将我遇到的一些编译失败的问题和解决办法进行分享

<!-- more -->

# 一、无法在禁用异常的情况下使用“try”

> 参考：[**Cannot use ‘try’ with exceptions disabled**](https://forums.unrealengine.com/t/cannot-use-try-with-exceptions-disabled/326861)

Unreal 不鼓励[使用 c++ 的异常系统](https://answers.unrealengine.com/questions/51798/how-can-i-enable-unwind-semantics-for-c-style-exce.html)。这一设计决策背后有几个原因。使用异常会使代码变得非常复杂，尤其是在像 UE 这样复杂的系统中。它还可能导致内存泄漏。然而，这些问题可以说是由于语言设计本身不佳造成的，在其他语言中可能不会如此频繁地出现，即使在 C++ 开发人员中，异常的使用也被认为是一个公开的讨论。
Epic团队建议提前检查，而不是事后从故障中恢复。
你可以使用[断言宏](https://docs.unrealengine.com/latest/INT/Programming/Assertions/index.html)报告意外行为（不可恢复和可恢复）。您还可以使用“checked”函数（如 `CastChecked`），如果出现错误，该函数会自动崩溃并显示诊断消息。大多数情况下，这是关于传递空指针或不支持的配置 - 只有重大错误才会导致这种情况发生。
但是，如果你非得使用异常，请查看第一个链接并尝试那里的建议，不知道是否适用于较新的引擎版本。

解决办法：在你项目文件夹下的Source里有一个`{ProjectName}.Build.cs`文件，在构造函数中添加一行代码`bEnableExceptions=true`：

```c++
// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class DoubaoTest : ModuleRules
{
	public DoubaoTest(ReadOnlyTargetRules Target) : base(Target)
	{
        //UEBuildConfiguration.bForceEnableExceptions = true;//如果下面没有试试这个，这个引擎版本低
		bEnableExceptions = true;//使用引擎5.1.1

		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });
	}
}

```

你可以在[源码](https://github.com/EpicGames/UnrealEngine/blob/release/Engine/Source/Programs/UnrealBuildTool/Configuration/ModuleRules.cs)中找到，在886行

```c#
/// <summary>
/// Enable exception handling
/// </summary>
public bool bEnableExceptions { get; set; }
```

# 三、Error NU1903

完整报错有很多行，我就选其中一行展示：

```cpp
Gauntlet.Automation.csproj: Error NU1903 : Warning As Error: Package 'Magick.NET-Q16-HDRI-AnyCPU' 14.7.0 has a known high severity vulnerability, https://github.com/advisories/GHSA-72hf-fj62-w6j4
```

这个在Epic社区也有人讨论：[Vulnerability of Magik.net](https://forums.unrealengine.com/t/vulnerability-of-magik-net/2650266)

我是参考下面这个办法解决的

I solved the issue by disabling the warnings inside the `PackageReference` tags in `AutomationTool.csproj`, `AutomationUtils.Automation.csproj`, and `Gauntlet.Automation.csproj`; i.e., respectively, I used:

```xml
<PackageReference Include="Magick.NET-Q16-HDRI-AnyCPU" Version="14.7.0">
  <NoWarn>$(NoWarn);NU1901;NU1902;NU1903</NoWarn>
</PackageReference>
<PackageReference Include="Magick.NET-Q16-HDRI-AnyCPU" Version="14.7.0" PrivateAssets="all">
  <NoWarn>$(NoWarn);NU1901;NU1902;NU1903</NoWarn>
</PackageReference>
```

and

```xml
<PackageReference Include="Magick.NET-Q16-HDRI-AnyCPU" Version="14.7.0" PrivateAssets="all">
  <NoWarn>$(NoWarn);NU1901;NU1902;NU1903</NoWarn>
</PackageReference>
```

Obviously, that’s not ideal because we are now only ignoring the vulnerability, but it does the trick if the goal is to continue using that package version.

# 四、ossl_typ.h(144): Error C2365 : “UI”: 重定义；以前的定义是“命名空间”

EncryptTool.cpp:

```cpp
#include "EncryptTool.h"

#include "openssl/hmac.h"
//。。。
```
EncryptTool.h：
```cpp
class EncryptTool
{
public:
	bool static CalculateHMACSHA256(const TArrayView<const uint8>& Message, const TArrayView<const uint8>& Key, TArray<uint8>& OutHmac);
};
```
JwtSecurityHelper.cpp:
```cpp
#include "EncryptTool.h"
EncryptTool::CalculateHMACSHA256(Message, Key, OutHmac);
```
报错：
```ABAP
[1/5] Compile [x64] EncryptTool.cpp
11>ossl_typ.h(144): Error C2365 : “UI”: 重定义；以前的定义是“命名空间”
11>ObjectMacros.h(886): Reference C2365 : 参见“UI”的声明
```

我试过在EncryptTool.cpp里使用

```cpp
#ifdef UI
#undef UI
#endif
#include "openssl/hmac.h"
```

或者在Build.cs中添加

```csharp
PublicDefinitions.Add("OPENSSL_NO_UI=1");
```

但是并没有什么用

## 解决方案

EncryptTool.cpp:

```cpp
#define UI UI_ST
THIRD_PARTY_INCLUDES_START
#include "openssl/hmac.h"
THIRD_PARTY_INCLUDES_END
#undef UI
```

## 参考

> [OpenSSL compile errorOpenSSL 编译错误](https://forums.unrealengine.com/t/openssl-compile-error/402878)

# Flushing shader jobs, remaining jobs 19357

打包遇到了 `UATHelper: Packaging (Windows): LogCookCommandlet: Display: Flushing shader jobs, remaining jobs 13675` 这样的 log，这个任务时间太长了

UE5 Cooker 在两种典型场景会触发 Flushing shader jobs：

1. 内存不足：`Detected max mem exceeded - forcing shader compilation flush`（日志往上翻，大概率能看到这句），内存不够，必须停下 Cook，等 shader 编译 worker 全部做完再继续资源 Cook
2. Cook 收尾阶段，主动等待所有 shader 任务完成（正常流程，但 jobs 数量上万就不正常）

## 快速排查步骤

### ① 先看完整日志头部，确认触发原因

搜索这一行： `Detected max mem exceeded - forcing shader compilation flush`

- ✅ 有这行：

  内存触发强制 flush

  。Cook 内存耗尽，暂停资源处理，等 shader 任务跑完。

  - 解决：打包前关闭其他软件；增加物理内存；或者调 Cook 内存参数。

> Tried to look for it on Google instead of my notes, couldn’t find it fast enough. Here’s for anyone else who needs it. Config/DefaultEditor.ini
>
> `[CookSettings]`
> `MemoryMinFreePhysical=512`
> `MemoryMinFreeVirtual=512`
> `MemoryMaxUsedPhysical=16384`

- ❌ 没有：单纯 shader 变体太多，DDC 缓存失效。

### ② 检查 DDC 缓存（优先做）

1. 关闭编辑器，删除项目 `Saved/DDC` 文件夹（旧坏缓存）
2. 打开编辑器，**先在编辑器内预编译目标平台 shader**：窗口 → 着色器预编译，选 Windows 平台，等编辑器把 shader 编译完、缓存写入 DDC
3. 再打包。打包会复用编辑器 DDC，不会重新生成上万任务

> 注意：DDC 是平台隔离的，编辑器预览 shader ≠ Windows 打包平台 shader，必须选 WindowsClient 目标平台预编译。

### ③ 减少 shader 变体数量（根治上万 jobs）

1. 清理废弃材质、材质实例，删掉不用的材质参数开关（每一个静态开关都会翻倍 shader permutation）
2. 项目设置 → 渲染：关掉不需要的功能（Lumen、光追、毛发、体积雾等，不需要就关闭）
3. 材质：尽可能合并材质，减少`StaticSwitch`，减少质量等级数量
4. 用 PSO 缓存：打包前录制 PSO，把运行时才需要的管线提前烘焙，减少 Cook 阶段 shader 数量Epic Games

## 结论

我清理了废弃的材质，加上了`CookSettings`，再次打包不再出现这种log
