---
layout: post
title: "自定义Object/Trace Channel"
date: 2024-08-28 09:00:00 +0800 
categories: UE
tag: Practice
---
* content
{:toc #markdown-toc}

这是一篇关于让自定义的碰撞通道在C++和蓝图中都能使用的文章

<!-- more -->

# 项目设置添加自定义碰撞通道

这一步很关键，不然蓝图无法使用

1. 打开UE编辑器，点击菜单栏的 **Edit（编辑） -> Project Settings（项目设置）**。
2. 在左侧列表中找到 **Engine -> Collision（碰撞）**。
3. 你会看到Trace Channels和Object Channels的添加区域。
   - **Object Channel（对象通道）**：赋予给物体本身时（如StaticMesh、Character等），决定“我是什么”，赋予物体通道时则决定和此通道的交互反应
   - **Trace Channel（追踪通道）**：用于射线检测（如LineTrace、SphereTrace等），决定“我要检测什么”，赋予物体通道时则决定“会被什么检测到”
4. 点击New Trace Channel或New Object Channel：
   - 输入通道名称（例如：`MyCustomObject` 和 `MyCustomTrace`）。
   - Default Response（默认响应）建议设为 `Ignore` 或 `Block`，这决定了未特别设置的物体对该通道的默认反应

此时蓝图就已经完全可用了，但是C++中还是看不到你设置的这些东西

# 在C++中使用自定义通道

在项目设置中添加通道后，UE会自动在 `DefaultEngine.ini` 中生成配置项，并在启动时将这些通道注册到 `ECollisionChannel` 枚举中。因此，你可以在C++中直接使用它们。

1. 打开你项目目录下的 `Config/DefaultEngine.ini`。
2. 搜索 `[/Script/Engine.CollisionProfile]`。
3. 你会看到类似这样的内容：

```
[/Script/Engine.CollisionProfile]
+DefaultChannelResponses=(Channel=ECC_GameTraceChannel1,DefaultResponse=ECR_Block,bTraceType=True,bStaticObject=False,Name="MyCustomTrace")
+DefaultChannelResponses=(Channel=ECC_GameTraceChannel2,DefaultResponse=ECR_Ignore,bTraceType=False,bStaticObject=False,Name="MyCustomObject")
```

- 从这里可以清晰地看到：
  - `MyCustomTrace` 对应的是 `ECC_GameTraceChannel1`。
  - `MyCustomObject` 对应的是 `ECC_GameTraceChannel2`。
- `bTraceType=True` 表示这是 Trace Channel，`False` 表示这是 Object Channel。

我们确实在C++中看不到这些东西，但是就如刚才说的，已经注册过了，我们就可以使用`ECC_GameTraceChannel1`和`ECC_GameTraceChannel2`，但是不清晰直观，所以我们会在一个头文件里这么写，只要你能认识分清即可：

```cpp
#define OBJECT_HAND ECC_GameTraceChannel1
#define TRACE_HAND ECC_GameTraceChannel2
```

不过在经行如射线检测时参数要求的并不是`ECollisionChannel`枚举类型，所以`UEngineTypes`提供了转换函数，方便我们进行类型转换：

```cpp
/** Convert a trace type to a collision channel. */
static ECollisionChannel ConvertToCollisionChannel(ETraceTypeQuery TraceType);

/** Convert an object type to a collision channel. */
static ECollisionChannel ConvertToCollisionChannel(EObjectTypeQuery ObjectType);

/** Convert a collision channel to an object type. Note: performs a search of object types. */
static EObjectTypeQuery ConvertToObjectType(ECollisionChannel CollisionChannel);

/** Convert a collision channel to a trace type. Note: performs a search of trace types. */
static ETraceTypeQuery ConvertToTraceType(ECollisionChannel CollisionChannel);
```

> 参考：[UE4[C++]自定义Object/Trace Channel](https://zhuanlan.zhihu.com/p/59507553)