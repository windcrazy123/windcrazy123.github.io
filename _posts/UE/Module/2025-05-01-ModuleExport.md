---
layout: post
title: "模块导出"
date: 2025-05-01 09:00:00 +0800 
categories: UE
tag: Module
---
* content
{:toc #markdown-toc}

记录一下虚幻引擎使用模块时遇到的模块导出问题

<!-- more -->

> 问题起源：[IImageWrapper::GetRawImage(struct FImage &)会出现链接失败的错误](https://forums.unrealengine.com/t/iimagewrapper-getrawimage-struct-fimage-unresolved/2015477)
>
> 然后有人发表[解决办法](https://github.com/EpicGames/UnrealEngine/pull/12920)：通过给 `IImageWrapper.h`文件中的`GetRawImage` 函数,添加 `virtual`关键字。
>
> 那么为什么呢

先观察一下文件结构

```
Plugins
	ImageWrapper
		Private
			ImageWrapperBase.cpp
			ImageWrapperBase.h
			ImageWrapperModule.cpp
		Public
			IImageWrapper.h
			IImageWrapperModule.h
Source
	MyProject
		MyActor.cpp
		MyActor.h
```

那么，我们可以简单仿照结构做一个简单的案例

```c++
#pragma once
class IImageWrapper{
public:
	virtual bool VirtualFunc();
	//bool NotVirtualFunc();
};
```

```c++
#pragma once
#include "IImageWrapper.h"

class FImageWrapperBase : public IImageWrapper{

};
```

```c++
#include "ImageWrapperBase.h"

bool IImageWrapper::VirtualFunc()
{
	return true;
}
```

```c++
#pragma once

#include "CoreMinimal.h"
#include "IImageWrapper.h"
#include "Modules/ModuleManager.h"

class IImageWrapperModule : public IModuleInterface
{
public:
	virtual void StartupModule() override{}
	virtual void ShutdownModule() override{}

	virtual TSharedPtr<IImageWrapper> CreatePP()=0;
};
```

```c++
#include "IImageWrapperModule.h"
#include "ImageWrapperBase.h"
#include "Modules/ModuleManager.h"

class FImageWrapperModule : public IImageWrapperModule
{
	virtual TSharedPtr<IImageWrapper> CreatePP() override {
		TSharedPtr<IImageWrapper> PP = MakeShared<FImageWrapperBase>();
		return PP;
	}
	virtual void StartupModule() override{}
	virtual void ShutdownModule() override{}
};

IMPLEMENT_MODULE(FImageWrapperModule, ImageWrapper);
```

```c++
void AMainActor::BeginPlay()
{
	Super::BeginPlay();
	
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));

	TSharedPtr<IImageWrapper> pp = ImageWrapperModule.CreatePP();
	pp->VirtualFunc();
}
```

对于`VirtualFunc`函数来说如果不是虚函数那么就会报错`LKN2019`，那么为什么成为虚函数不需要添加`IMAGEWRAPPER_API`就可见呢。

首先了解一下模块的结构

# 虚幻4模块源码结构解释

## Classes文件夹

Classes文件夹曾经是特殊的，因为在旧版本的[虚幻引擎](https://zhida.zhihu.com/search?content_id=100837968&content_type=Article&match_order=1&q=虚幻引擎&zhida_source=entity)中，所有UObject以及衍生类的声明，都必须写在Classes文件夹中。 但这一个限制已经不再存在，如果你对比并观察最近的几个虚幻版本，你就会发现，虚幻新加的功能模块，和新出的各种示例、教程等，也已经完全弃用了Classes文件夹。 所以，如果你还能看到带有Classes的文件夹结构，那估计是在旧版本虚幻下写的。在今后的代码中，你可以完全忘记有这么一回事。

## Public文件夹

模块源码下的Public文件夹，目前的作用就是，当该模块本身被其他模块依赖时，该模块的Public文件夹路径，会被其他模块自动添加为“include path”。打个比方，假如你有一个模块A，A/Public/下有code1.h和code2.h两个头文件；然后我有一个模块B，我在模块B的`PublicDependencyModuleNames`列表中加入模块A，这时候，我在模块B的代码中就可以直接include模块A的Public文件夹下的各种头文件了，比如`#include "code1.h"`。

## Private文件夹

Private文件夹并没有什么特殊的作用和意义。但已经作为惯例与Public文件夹成对使用了。下面会解释。

## Public/Private文件夹分离

为什么要使用Public/Private文件夹分离文件呢？使用这个方法分离文件的主要好处是封装。假如你写了一个需要给别人使用的模块，那么，你只需把使用这个模块所需要的Header文件放到Public目录下，把它的实现文件放到Private目录下，如果有其他功能的[头文件](https://zhida.zhihu.com/search?content_id=100837968&content_type=Article&match_order=5&q=头文件&zhida_source=entity)和实现文件且不需要使用者关心或者修改的，就可以都放到Private目录下，这样使用者要使用你的代码，只需要看看Public下有什么，并不需要深入去研究你的实现。

## 一些值得注意的地方

所以，虚幻的惯例是：

> 并不是Public下放头文件，Private下放实现文件，而是Public放需要暴露给别人的头文件,其他不需要暴露给其他人的头文件和实现文件放到Private目录下。
> Private文件夹并没什么特殊性，只是习惯性地被拿来与Public成对使用而已，并非强制的，你使用Pipixia放私有文件都可以。
> 游戏模块（非插件模块），一般来说也不会被其他模块依赖，因为游戏模块基本就是所有其他现有模块的“使用者”，所以你会看到网上某些游戏模块的源码结构根本不遵守Public/Private分离的惯例，对于游戏模块来说，这是可取的，但不推荐，因为你的游戏模块可能会有一个对应的编辑器模块，而这个编辑器模块一般是要依赖你的游戏模块的。



------

# 结论

那么我们就知道了在UE的UBT和UHT下导致模块的Public文件夹下的文件被include（即使没有导出）因此我们可以`#include "IImageWrapper.h"`（*Cast to the class. Extend the class继承后的子类仍然无法实例化 Use inline functions.*）但是没有导出是不会`link against .cpp文件`的因此函数是不可调用的，但是使用`virtual`后我们可以通过动态绑定机制通过虚表动态绑定到实现（可参考我写的[虚函数]()的文章）但前提是我们要拿到`IImageWrapper`类或子类的实例，**不然构造函数也没导出是没办法实例化的**，但是UE模块时导出的可以通过他拿到实例（工厂函数存在的话，显然`ImageWrapper`模块是存在的）然后实例通过虚指针找到函数。

# 参考链接

> 1，[虚幻4模块源码结构解释](https://zhuanlan.zhihu.com/p/57186370)
>
> 2，[UE4 Modules](https://docs.google.com/presentation/d/1rSFFQk7RxNAHevROfVvUNviUfIntLkO_HpdvzHLkNEs/edit?pli=1&slide=id.g6e0e4b3bcf_2_45#slide=id.g6e0e4b3bcf_2_45)
>
> 
