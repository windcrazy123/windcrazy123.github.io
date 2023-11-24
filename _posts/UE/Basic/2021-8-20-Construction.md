---
layout: post
title: "UE Actor的构造"
date: 2020-10-20 09:00:00 +0800 
categories: UE
tag: 基础
---
* content
{:toc #markdown-toc}

这是一篇关于UE中蓝图Construction Script和C++中Constructor、OnConstruction

<!-- more -->



官方文档参考：[**Actor Lifecycle**](https://docs.unrealengine.com/5.3/en-US/unreal-engine-actor-lifecycle/)    [**Construction Script**](https://docs.unrealengine.com/5.3/en-US/construction-script-in-unreal-engine/)

# Constructor

C++的构造函数是要比`OnConstruction`、`Construction Script`、C++Beginplay、蓝图Beginplay要早的



# Construction Script & OnConstruction

在蓝图编辑器中访问构造脚本

<img src="{{ '/styles/images/Basic/Construction/ConstructionScript.png' | prepend: site.baseurl }}" />

在 C++ 中，它由 `OnConstruction` 方法表示：

```c++
virtual void OnConstruction(const FTransform& Transform) {}
```

这个方便的工具对于定义如何构建 Actor 非常有用。一个典型的用例是当您希望能够设置 Actor 的属性并看到它自动地根据编辑器中的这些参数进行更新时。例如，我们可以使用它来构建一个以`边数`作为属性的多边形Actor，每次编辑`边数`时，我们都可以看到多边形在编辑器中重建和更新（而不是在运行时，但是在运行前，会运行一次构造脚本）。

但是，虽然它非常有用，但如果使用不当，也非常危险，甚至导致编辑器崩溃。

为了避免任何潜在的问题，我们需要：

- 知道何时调用构造脚本
- 了解它如何导致编辑器崩溃，使项目无法编辑，甚至一打开就崩溃
- 学习如何正确地使用它（构造脚本并不总是唯一的办法）

## 编辑器何时调用构造脚本

这是构造脚本最有趣的一点：它是在编辑器中调用的，而不是在运行时调用的。

并且在编辑器中，可以多次调用它：

- **当Actor产生（Spawn）时**
- **当Actor移动时**（*PostEditMove*事件）：在这种情况下，当此Object移动时它将被调用多次
- **当Actor的属性发生更改时**（*PostEditChangeProperty*事件）

正如我们所看到的，它可以被频繁调用。虽然它确实很有用（如果我更改任何属性，我可以直接看到构造脚本正在重建的对象），但也出现了一些缺点。

另外

```c++
/** Called after an actor has been moved in the editor */
	virtual void PostEditMove(bool bFinished);
```

```c++
/**
	 * Called when a property on this object has been modified externally
	 *
	 * @param PropertyThatChanged the property that was modified
	 */
	virtual void PostEditChangeProperty( struct FPropertyChangedEvent& PropertyChangedEvent);
```

重载时（参考`PaperGroupedSpriteComponent`和`VPRootActor`）一般添加#if WITH_EDITOR与#endif

```c++
	// UObject interface
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	// End of UObject interface
```

```c++
#if WITH_EDITOR
	virtual void PostEditMove(bool bFinished) override;
#endif
```



## 滥用的主要威胁

使用构造脚本时存在两个主要威胁。第一个很明显，第二个则比较棘手。

第一个是**对项目性能的影响**：即使只在编辑器中调用`Construction Script`，如果`Construction Script`太复杂，将无法再在编辑器中移动Actor。假设运行 `ConstructionScript` 需要一秒钟，如果尝试移动 Actor，则在移动 Actor 时，`Construction Script` 将被调用数十次，因此编辑器可能卡数十秒。那么对于美术同学来说，记住不要移动这个特定对象将是一件非常痛苦的事情，因为总是会对这个Actor摆放的位置不满意。

第二个威胁更加棘手。它涉及构造脚本的循环调用：**如果属性的更改导致Construction Script的循环调用，编辑器将崩溃**。原理如下：假设我们在一个 Actor 上有一个属性 A 和一个属性 B，并且`Construction Script`声明如果 A 改变，则 B 等于 A+1，如果 B 改变，则 A 等于等于B+1。理论上，这可能会由于无限循环而造成的崩溃。幸运的是，这种特定情况是由编辑器管理的：`Construction Script`无法调用自己（即使它更改了A/B属性）。但是，当您的演员具有*ChildActorComponent*并且这个Actor和这个*ChildActorComponent*之间存在循环关系时，编辑器将无能为力，它只会崩溃（根据*Isara*的经验，不是每次都会崩溃，这使得诊断变得更加困难）。因此，如果父级的构造脚本更改了子级的属性，并且子级的构造脚本更改了父级的属性，我们将进行循环调用（在两个不同的构造脚本之间）。

## 建议

构建脚本的使用建议如下：

- **如果没有必要，请避免使用它们**：它是一个自动化工具，仅当您有重复性任务需要自动化时才使用它
- **保持构建脚本简单快速**，它只是作为一个方便的工具而不是当成制作一个完全程序化的游戏
- **避免在 Construction Script 中更改另一个 actor（甚至是子 actor）的属性**，如果必须这样做，请记住，对 `Construction Script` 的循环调用可能会使编辑器崩溃





> 参考：[Be careful with the Construction Script](https://isaratech.com/ue4-be-careful-with-the-construction-script/)
>
> [构造函数和 BeginPlay() 中应该包含什么样的代码？](https://www.reddit.com/r/unrealengine/comments/107vid2/what_kind_of_code_should_be_in_a_constructor_vs/?onetap_auto=true)