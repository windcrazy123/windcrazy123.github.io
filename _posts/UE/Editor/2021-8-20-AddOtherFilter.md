---
layout: post
title: "在UE的内容浏览器中添加自定义的Filter"
date: 2021-08-20 09:00:00 +0800 
categories: UE
tag: Editor
---
* content
{:toc #markdown-toc}

这是一篇如何在UE的内容浏览器中添加自定义的Filter的文章

<!-- more -->

# 目标

在UE的内容浏览器中，可以使用**Filter**来过滤资源：

![](D:\WindCrazyGithubio\windcrazy123.github.io\styles\images\Editor\FilterMenu.png)



观察代码发现，一个Filter是通过`AllFrontendFilters`函数添加的，而调用`AllFrontendFilters`的地方不止是对内建的filter使用。比如在后面的位置，可以看到它遍历了`UContentBrowserFrontEndFilterExtension`对象并依据它来添加，注释还标注了 *Add any global user-defined frontend filters*：![](D:\WindCrazyGithubio\windcrazy123.github.io\styles\images\Editor\AllFrontendFilters.png)

# 实现

```c++
//MyFilter.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "FrontendFilterBase.h"

// 自定义Filter
class FMyFilter : public FFrontendFilter
{
public:
	FMyFilter(TSharedPtr<FFrontendFilterCategory> InCategory)
		: FFrontendFilter(InCategory)
	{
	}

	// FFrontendFilter implementation
	virtual FLinearColor GetColor() const override;
	virtual FString GetName() const override;
	virtual FText GetDisplayName() const override;
	virtual FText GetToolTipText() const override;
	// End of FFrontendFilter implementation

	// IFilter implementation
	virtual bool PassesFilter(FAssetFilterType InItem) const override;
	// End of IFilter implementation
};
```

```c++
//MyFilter.cpp
#include "MyFilter.h"

#define LOCTEXT_NAMESPACE "MyFilter"

// 颜色
FLinearColor FMyFilter::GetColor() const
{
	return FLinearColor::Yellow;
}

// 引擎内部的标识名称，不可重复
FString FMyFilter::GetName() const
{
	return TEXT("MyFilter");
}

// 显示名
FText FMyFilter::GetDisplayName() const
{
	return LOCTEXT("MyFilterDisplay", "MyFilter");
}

// 鼠标指针移到Filter列表上时显示的提示
FText FMyFilter::GetToolTipText() const
{
	return LOCTEXT("MyDisplayTooltip", "This is my Filter");
}

// Filter的主要逻辑
// 内容浏览器中显示的文件夹下的每个文件都经过这个判断。
bool FMyFilter::PassesFilter(FAssetFilterType InItem) const
{
	//当前逻辑：过滤掉开头为“temp”的资源
	return !InItem.AssetName.ToString().StartsWith("temp");
}

#undef LOCTEXT_NAMESPACE
```

```c++
//MyFilterExtention.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "ContentBrowserFrontEndFilterExtension.h"

#include "MyFilterExtention.generated.h"

UCLASS()
class UMyFilterExtention : public UContentBrowserFrontEndFilterExtension
{
public:
	GENERATED_BODY()

	// UContentBrowserFrontEndFilterExtension interface
	virtual void AddFrontEndFilterExtensions(TSharedPtr<class FFrontendFilterCategory> DefaultCategory, TArray< TSharedRef<class FFrontendFilter> >& InOutFilterList) const override;
	// End of UContentBrowserFrontEndFilterExtension interface
};
```

```c++
//MyFilterExtention.cpp
#include "MyFilterExtention.h"
#include "MyFilter.h"


void UMyFilterExtention::AddFrontEndFilterExtensions(TSharedPtr<class FFrontendFilterCategory> DefaultCategory, TArray< TSharedRef<class FFrontendFilter> >& InOutFilterList) const
{
	// 注册MyFilter
	InOutFilterList.Add(MakeShareable(new FMyFilter(DefaultCategory)));
}
```

主要关注的就是`FMyFilter::PassesFilter`函数，因为过滤的逻辑在此指定。
比如这里就是过滤掉开头为“temp”的资源：

```cpp
return !InItem.AssetName.ToString().StartsWith("temp");
```

# 参考链接

> 1，[在UE的内容浏览器中添加自定义的Filter](https://blog.csdn.net/u013412391/article/details/120929455)
>
> 