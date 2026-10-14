# Niagara Particle kill on collision

> 参考：[Niagara Particle kill on collision](https://forums.unrealengine.com/t/niagara-particle-kill-on-collision/447828)

使用UE5.4时发现Niagara没有了`kill on collision`类似的节点，在论坛上有两种方法都可使用，个人觉得`Collision`中`Advanced Aging Rate`更加方便快捷

# PICOXRInputPlus

## Component的OnRegister调用

1. Component的OnRegister会先被此组件的父组件UActorComponent::PostEditChangeProperty中delete ReregisterContext;的析构函数调起的，
2. 在delete完后紧接着MyOwner->RerunConstructionScripts();又执行了ExecuteConstruction最终是由SCS调用的ExecuteScriptOnActor中调用的USimpleConstructionScript::RegisterInstancedComponent(NewSceneComp);又再次调起的UActorComponent::RegisterComponentWithWorld而调起的OnRegister
3. 之后引擎检测到有修改，调用了APawn::PostEditChangeProperty而AActor的此函数中有RerunConstructionScripts();和ReregisterAllComponents();调用，上一个函数是第二次的OnRegister，后面的函数是调用了IncrementalRegisterComponents(0);来让所有的组件执行Component->RegisterComponentWithWorld(World, Context);后调用的ExecuteRegisterEvents(Context);才调起了OnRegister
4. 此时已经是四次调用了，仅仅是改了一个值，但是还没完
5. 如果你这个蓝图视口Viewport是打开的状态下，引擎还会调用SSCSEditorViewport::DeferredUpdatePreview来更新预览场景

```cpp
// Create or update the Blueprint actor instance in the preview scene
BlueprintEditorPtr.Pin()->UpdatePreviewActor(Blueprint, !bIsPreviewActorValid);
```
6. 而此函数也会调用PreviewActor->ReregisterAllComponents();和PreviewActor->RerunConstructionScripts();因此又有两次调用
7. 在Register的流程中可以看到PostEditChangeProperty是要早于组件的注册的，因此PostEditChangeProperty函数中会有一些功能用不了，比如GetAttachParent函数会返回空，不过又因为OnRegister函数不只是在Editor中会有很多次调用而且也会在Runtime中运行，因此最好是不要放过重的逻辑尤其是用来处理PostEditChangeProperty相关的逻辑，如果需要用到GetAttachParent之类的东西，可以创建一些EditorData变量，在OnRegister赋值后在PostEditChangeProperty使用也是可以的。如

```cpp
void UGrabberComponent::OnRegister()
{
	Super::OnRegister();
#if WITH_EDITORONLY_DATA
	if (!MyParentComponent)
	{
		MyParentComponent = Cast<USkinnedMeshComponent>(GetAttachParent());
	}
#endif
}
```

# 补充

最好是自己调试一下可以更加清晰，你还可以看到有SCS_Node的调用，因为我们写的类是Component的，然后手动在Actor蓝图添加，这样会有一个SCS在里面。所以问题来了，如果说我们打开Actor蓝图编辑器的情况下，热重载了我们的组件，很有可能出现`SCS Node component template PokeInteractor has been recreated because it was unexpectedly not owned by the class`这个log，蓝图SCS组件模版发生变化会导致Outer对象改变，从而导致引擎新建了一份模版，这样会导致我们在组件上的变量的修改重置，所以最好关闭编辑器进行编译。

> 参考：[UE4源码剖析——Actor蓝图之CDO与SCS](https://zhuanlan.zhihu.com/p/681252301)
>
> [浅析UE4 Actor&Actor生命周期](https://www.cnblogs.com/CatSevenMillion/p/16653763.html)
>
> [Nesting actor components isn't exactly straightforward.](https://kisspread.github.io/notes/Basic/painful_ue.html#nesting-actor-components-isnt-exactly-straightforward)

## Template Mismatch during attachment.

```cpp
Template Mismatch during attachment. Attaching instanced component to template component. Parent 'Grabbable_GEN_VARIABLE' (Owner 'None') Self 'InteractableCollider' (Owner 'BP_Pot_C_1').
```

现在代码结构为：

```cpp
UInteractableComponent::UInteractableComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	InteractableCollider = CreateDefaultSubobject<USphereComponent>(TEXT("InteractableCollider"));
	InteractableCollider->SetSphereRadius(CollisionRadius);
	InteractableCollider->SetupAttachment(this);
	
	InteractableCollider->SetGenerateOverlapEvents(true);
	InteractableCollider->SetCollisionResponseToAllChannels(ECR_Overlap);
	InteractableCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractableCollider->SetCollisionObjectType(ECC_WorldDynamic);

	InteractableCollider->SetVisibility(false);
	InteractableCollider->SetHiddenInGame(true);
}
//。。。。。。
class UGrabbableComponent : public UInteractableComponent
```

将UGrabbableComponent组件添加到Actor蓝图 `A` 中，后由`A`创建子蓝图 `B`，将B中UGrabbableComponent组件一些属性进行修改，放入关卡后出现如此报错，并且打包也是这个报错

这个主要问题在于CDO和实例之间的问题，同时也是一些博主说组件中不能添加组件应该在Actor中添加组件的原因（构造函数中CreateDefaultSubObject）

![](D:\windcrazy123.github.io\styles\images\Basic\Construction\Place​Actor​To​Comp​nent​Register​.png)

我认为关键点就在于USCS_Node::ExecuteNodeOnActor调用的AActor::CreateComponentFromTemplate所产生的NewActorComp过程中变量赋值的问题，在调试中发现子蓝图B的`InteractableCollider`在创建出来后他的AttachParent又被设置回和CDO内容相同的东西了，明明在`UInteractableComponent`的构造函数中已经设置好变量，所以之前在PostEditor函数中设置的碰撞球半径不生效可能是这个原因，但是蓝图A就是正常的，这个由于时间原因还需要后续调试或学习。

> 参考：[UE4 FUObjectHashTables FUObjectArray](https://zhuanlan.zhihu.com/p/683756790)
>

## AI内容

### 获取Component中Children

下方代码是UActorComponent::RegisterComponentWithWorld中获取Component中Children的`GetObjectsWithOuter`的函数片段
```C++
int32 StartNum = Results.Num();
FUObjectHashTables& ThreadHash = FUObjectHashTables::Get();
FHashTableLock HashLock(ThreadHash);
FHashBucket* Inners = ThreadHash.ObjectOuterMap.Find(Outer);
if (Inners)
{
    for (FHashBucketIterator It(*Inners); It; ++It)
    {
        UObject *Object = static_cast<UObject *>(*It);
        if (!Object->HasAnyFlags(ExclusionFlags) && !Object->HasAnyInternalFlags(ExclusionInternalFlags))
        {
            if (UE::GC::GIsIncrementalReachabilityPending)
            {
                UE::GC::MarkAsReachable(Object);
            }
            Results.Add(Object);
        }
    }
    int32 MaxResults = GUObjectArray.GetObjectArrayNum();
    while (StartNum != Results.Num() && bIncludeNestedObjects)
    {
        int32 RangeStart = StartNum;
        int32 RangeEnd = Results.Num();
        StartNum = RangeEnd;
        for (int32 Index = RangeStart; Index < RangeEnd; Index++)
        {
            FHashBucket* InnerInners = ThreadHash.ObjectOuterMap.Find(Results[Index]);
            if (InnerInners)
            {
                for (FHashBucketIterator It(*InnerInners); It; ++It)
                {
                    UObject *Object = static_cast<UObject *>(*It);
                    if (!Object->HasAnyFlags(ExclusionFlags) && !Object->HasAnyInternalFlags(ExclusionInternalFlags))
                    {
                        if (UE::GC::GIsIncrementalReachabilityPending)
                        {
                            UE::GC::MarkAsReachable(Object);
                        }
                        Results.Add(Object);
                    }
                }
            }
        }
        check(Results.Num() <= MaxResults); // otherwise we have a cycle in the outer chain, which should not be possible
    }
}
```

这段代码是虚幻引擎（Unreal Engine）底层核心代码的一部分，主要用于**获取指定 Outer 对象所包含的所有子对象，并根据参数决定是否递归获取嵌套的子对象**。

简单回答你的问题：**是的，它的核心目的就是获取 Outer 引用的所有 object，但不仅如此，它还包含了排除特定标记对象、支持增量垃圾回收标记、以及递归获取嵌套对象的功能。**

下面我为你详细拆解这段代码的逻辑：

#### 1. 基础准备与直接子对象获取
```cpp
int32 StartNum = Results.Num();
FUObjectHashTables& ThreadHash = FUObjectHashTables::Get();
FHashTableLock HashLock(ThreadHash);
FHashBucket* Inners = ThreadHash.ObjectOuterMap.Find(Outer);
```
*   `Results` 是传入的结果数组，`StartNum` 记录了在执行此函数前数组中原有的元素数量。
*   `FUObjectHashTables` 是 UE 用来加速 UObject 查找的哈希表结构（包含 Outer 映射、接口映射等）。`FHashTableLock` 是 RAII 锁，确保在多线程环境下对哈希表的访问是线程安全的。
*   `ObjectOuterMap` 是一个核心哈希表，它维护了 **Outer -> Inners** 的映射关系。`ThreadHash.ObjectOuterMap.Find(Outer)` 就是查找直接属于该 Outer 的所有 UObject。

#### 2. 遍历直接子对象并过滤
```cpp
if (Inners)
{
    for (FHashBucketIterator It(*Inners); It; ++It)
    {
        UObject *Object = static_cast<UObject *>(*It);
        if (!Object->HasAnyFlags(ExclusionFlags) && !Object->HasAnyInternalFlags(ExclusionInternalFlags))
        {
            if (UE::GC::GIsIncrementalReachabilityPending)
            {
                UE::GC::MarkAsReachable(Object);
            }
            Results.Add(Object);
        }
    }
```
*   如果找到了子对象（`Inners` 不为空），则遍历这个哈希桶中的所有 UObject。
*   **过滤机制**：如果对象拥有 `ExclusionFlags`（如 `RF_BeginDestroyed`）或 `ExclusionInternalFlags`，则跳过，不加入结果集。
*   **增量 GC 支持**：如果当前开启了增量垃圾回收且正在扫描阶段，需要调用 `MarkAsReachable` 防止这些正在被访问的对象被 GC 当作垃圾回收掉。
*   将符合条件的对象加入 `Results` 数组。

#### 3. 递归获取嵌套子对象
```cpp
    int32 MaxResults = GUObjectArray.GetObjectArrayNum();
    while (StartNum != Results.Num() && bIncludeNestedObjects)
    {
        int32 RangeStart = StartNum;
        int32 RangeEnd = Results.Num();
        StartNum = RangeEnd;
        for (int32 Index = RangeStart; Index < RangeEnd; Index++)
        {
            FHashBucket* InnerInners = ThreadHash.ObjectOuterMap.Find(Results[Index]);
            // ... (与上面类似的遍历、过滤、添加逻辑)
        }
        check(Results.Num() <= MaxResults); // otherwise we have a cycle in the outer chain, which should not be possible
    }
}
```
*   这部分是代码的精髓，实现了一个**广度优先搜索（BFS）**的逻辑。
*   `bIncludeNestedObjects` 控制是否需要递归获取。如果为 false，这个 while 循环根本不会执行。
*   **BFS 逻辑解析**：
    *   `StartNum != Results.Num()`：如果结果数组没有增长，说明没有新的子对象被找到，搜索结束。
    *   `RangeStart` 和 `RangeEnd` 圈定了**本轮新加入**的对象范围。
    *   `for (int32 Index = RangeStart; Index < RangeEnd; Index++)`：只针对本轮新加入的对象，去 `ObjectOuterMap` 中查找它们的子对象。
    *   找到的新子对象会继续追加到 `Results` 末尾，下一轮 while 循环会继续处理这些新对象，以此类推，直到所有嵌套层级的对象都被找完。
*   **死循环保护**：`check(Results.Num() <= MaxResults);`。`MaxResults` 是当前引擎中所有 UObject 的总数。如果 `Results` 的数量超过了它，说明 Outer 链中出现了**循环引用**（A的Outer是B，B的Outer是A）。在 UE 的架构中，UObject 的 Outer 链必须是严格树状的，不允许出现环，因此这里用 `check` 触发断言报错。

#### 总结
这段代码的功能可以概括为：
1. **查找** 指定 `Outer` 的所有直接子对象。
2. **过滤** 掉带有排除标记的对象。
3. **标记** 存活状态以配合增量垃圾回收。
4. **递归查找**（如果 `bIncludeNestedObjects` 为 true）所有嵌套的子对象（即子对象的子对象），直到最底层。
5. **保证安全性**，通过锁保证线程安全，通过断言检查防止 Outer 循环引用导致的死循环。

### UE的Attach链的构成

在OnRegister中有一行代码AttachToComponent(GetAttachParent(), FAttachmentTransformRules::KeepRelativeTransform, GetAttachSocketName())这是什么意思，明明使用GetAttachParent还Attach什么啊，UE的Attach链是如何构成的什么时候构成的

#### 一、先拆解这行代码：到底在做什么

先明确调用主体和参数的含义：

```
// 调用主体：某个 USceneComponent 子类实例（比如你的 InteractableCollider）
AttachToComponent(
    GetAttachParent(),                  // 父对象：自己当前的场景父组件
    FAttachmentTransformRules::KeepRelativeTransform, // 保持相对变换
    GetAttachSocketName()               // 保持原来的插槽名
);
```

**直观理解：把自己重新挂到「已经是自己父节点」的组件上。** 看起来完全是多此一举，但这是UE开发中一个非常经典的**「附着刷新操作」**——它不改变父节点，只做「重新绑定+刷新参数+修复状态」。

------

#### 二、核心疑问：明明已经有父节点了，为什么还要重新Attach？

##### 1. 先澄清两个关键事实

###### （1）SetupAttachment 只管「模板定父级」，不管「附着参数」

构造函数里的 `SetupAttachment(this)` 只做一件事：

> 在CDO的SCS节点里，记录「这个子组件的父组件叫什么名字」。

它**不能指定附着规则、不能绑定插槽、不能设置变换偏移**。SCS实例化时自动建立的Attach，用的是引擎默认参数。

###### （2）SCS自动建立的Attach链是「半成品」

Actor实例化时，SCS会按照模板记录的父节点名，批量建立Attach关系，但这个初始Attach有局限性：

- 无法绑定骨骼Socket（骨骼此时可能还未初始化）
- 只能用默认的变换规则
- 热重载、蓝图重建、编辑器预览时容易出现层级不同步

**这行代码的本质：在OnRegister阶段，对已经存在的父子关系，重新执行一次完整的Attach流程，用来修复状态、补全参数。**

##### 2. 这行代码的4个真实作用

###### ① 修复骨骼Socket绑定延迟

如果父组件是骨骼网格体（SkeletalMeshComponent）：

- SCS实例化建立Attach时，骨骼资源可能还没加载完成，Socket还不存在

- 此时指定Socket绑定会失效，组件会挂在父节点原点，而不是Socket位置

- OnRegister阶段骨骼已经初始化完成，重新Attach可以正确绑定到Socket上

  > `GetAttachSocketName()` 就是取出SCS模板里记录的插槽名，重新绑定一次。

###### ② 强制刷新Transform层级，修复偏移不同步

- 蓝图里修改了组件的相对位置、旋转，但SCS实例化时没有正确应用
- 或者组件被Detach过、层级被外部代码改动过
- 重新执行 `AttachToComponent` + `KeepRelativeTransform`，会强制按照当前相对变换重新计算世界变换，修复位置错位。

###### ③ 修复热重载/蓝图编译后的层级错乱

编辑器环境下，蓝图编译、C++热重载后，组件的场景Attach链经常出现「逻辑上有父节点，但实际场景Transform链断了」的情况。 重新Attach一次可以强制重建场景Transform节点的父子链接。

###### ④ 补全SCS没做的附着逻辑

SCS自动Attach只做最基础的父子绑定，不会触发：

- Overlap事件的重新计算
- 物理碰撞的附着更新
- 渲染场景的Transform节点刷新 重新Attach会走完整的附着流程，把这些状态全部刷新。

> 一句话总结：**父节点确实没变，但附着的状态、参数、底层链接可能是坏的，这行代码是做一次「刷新修复」。** 类似你已经连着WiFi，点一下「重新连接」用来刷新IP和信道。

------

#### 三、UE的Attach链：是什么、怎么构成、什么时候构成

##### 1. Attach链的本质

Attach链是**USceneComponent特有的场景变换层级**（UActorComponent没有Transform，不存在Attach链），本质是一棵双向树：

- 每个SceneComponent有一个 `AttachParent`（父组件指针）
- 每个SceneComponent有一个 `AttachChildren`（子组件数组）
- 父组件的Transform变化，会级联影响所有子组件的世界变换
- 这棵树同时映射到渲染场景、物理场景的Transform层级

##### 2. Attach链构建的完整时间线

结合CDO/SCS，分5个阶段，每个阶段能做什么不能做什么非常清晰：

###### 阶段1：CDO模板构造期（C++构造函数执行）

- **操作**：`CreateDefaultSubobject` + `SetupAttachment`
- **做了什么**：只在SCS节点里记录「父组件的名字」，是**逻辑父子关系**
- **实际状态**：没有真实的Attach链，CDO上调用 `GetAttachParent()` 永远返回 `nullptr`
- **禁止**：调用 `AttachToComponent`（模板阶段操作实例API，会触发Template Mismatch）

###### 阶段2：SCS实例展开期（Actor Spawn → InitializeComponents）

- **触发时机**：把蓝图拖进关卡、SpawnActor时，组件初始化阶段

- 做了什么

  ：

  1. 按SCS节点树克隆所有组件实例
  2. 从根组件开始，按层级遍历，**自动逐个调用 AttachToComponent**
  3. 建立起完整的物理Attach链，`AttachParent` 指针被正式赋值

- 特点

  ：

  - 这是Attach链**第一次真正形成**
  - 用的是默认变换规则，不处理复杂Socket绑定
  - 这个阶段是模板驱动的，会严格校验Template Mismatch

###### 阶段3：OnRegister 阶段

- **触发时机**：组件注册到渲染场景和物理场景之前

- **Attach链状态**：已经存在，但可能参数不全、状态不完整

- 常见操作

  ：

  - 重新Attach刷新状态（就是你问的这行代码）
  - 修改附着父节点（比如你之前挂到Actor根）
  - 设置碰撞、重叠事件

- **特点**：操作的全是实例对象，不经过SCS模板校验，不会触发Template Mismatch

###### 阶段4：PostInitializeComponents

- **触发时机**：所有组件全部初始化完成
- **Attach链状态**：完全稳定，所有父子关系、Socket绑定都已生效
- **适合做**：依赖Attach链的逻辑，比如获取子组件位置、计算碰撞范围

###### 阶段5：运行时（BeginPlay之后）

- 可以随时调用 `AttachToComponent` / `DetachFromComponent` 动态修改Attach链
- 每次修改都会触发Transform级联更新、重叠事件、物理同步

------

#### 四、回到你的问题：为什么OnRegister里Attach能解决模板报错

现在把整个逻辑串起来：

1. **构造函数里SetupAttachment**：把父子关系写进SCS模板 → 子蓝图继承合并SCS时，校验发现「Scene组件挂在ActorComponent上」结构非法 → Template Mismatch

2. 删掉SetupAttachment，改到OnRegister里Attach

   ：

   - SCS模板里没有父子关系记录 → 蓝图继承合并时，没有非法结构，校验通过
   - 实例化后，OnRegister阶段手动执行Attach → 直接操作实例组件，建立运行时Attach链
   - 整个过程不经过SCS模板校验，所以不会报错

> 核心区别：
>
> - SCS自动建Attach：**模板驱动**，受模板规则约束，会报Template Mismatch
> - OnRegister手动Attach：**运行时驱动**，直接改实例，模板校验管不到

------

#### 五、补充：容易混淆的两个父节点概念

很多人搞不清，这里明确区分：

| 概念                 | 存在阶段     | 含义                                       | 获取方式                      |
| -------------------- | ------------ | ------------------------------------------ | ----------------------------- |
| **SCS逻辑父节点**    | CDO模板阶段  | 模板里记录的父组件名字，只用于实例化时建链 | SCS节点数据，运行时不直接暴露 |
| **场景Attach父节点** | 实例运行阶段 | 实际Transform链的父组件，真正影响位置      | `GetAttachParent()`           |

- 正常情况下，实例化后两者是一致的
- 你在OnRegister里改Attach，只会改后者，不会影响CDO模板里的前者
- 这也是为什么OnRegister改Attach不会影响蓝图继承的原因

------

#### 总结

1. `AttachToComponent(GetAttachParent(), ...)` 不是换父节点，是**刷新附着状态**，用来修复Socket、Transform不同步、热重载等问题。
2. Attach链分「模板逻辑链」和「实例物理链」：前者在构造函数由SetupAttachment定义，后者在实例化时由SCS自动构建。
3. 构造函数/SetupAttachment触发模板校验，OnRegister/AttachToComponent是运行时操作，不触发模板校验——这就是你改到OnRegister就解决报错的根本原因。

需要我再讲一下 `GetAttachParent()`、`GetAttachmentRoot()`、`GetOwner()->GetRootComponent()` 三者的区别和适用场景吗？
