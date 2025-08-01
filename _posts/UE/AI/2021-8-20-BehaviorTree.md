---
layout: post
title: "UE 行为树基础"
date: 2021-8-20 09:00:00 +0800 
categories: UE
tag: AI
---
* content
{:toc #markdown-toc}

这是一篇关于UE行为树(*Behavior Tree*)的基础节点和一些使用方法等，大致分为Composite、Auxiliary、Task三个篇章。
可能较为深入但初学者也可以看懂的( •̀ ω •́ )✧

<!-- more -->

先贴一个[官方文档](https://docs.unrealengine.com/5.2/zh-CN/behavior-tree-node-reference-in-unreal-engine/)

## BTCompositeNode

```c++
class AIMODULE_API UBTCompositeNode : public UBTNode
```

就是合成节点，包括三种节点：Selector、Sequence和SimpleParallel

都是继承自UBTCompositeNode的合成节点，他们主要是按照自己的逻辑规则实现了父类的虚函数GetNextChildHandler

### Sequence节点

<img src="{{ '/styles/images/ai/Sequence.png' | prepend: site.baseurl }}" />

```c++
class AIMODULE_API UBTComposite_Sequence : public UBTCompositeNode
```

```c++
int32 UBTComposite_Sequence::GetNextChildHandler(FBehaviorTreeSearchData& SearchData, int32 PrevChild, EBTNodeResult::Type LastResult) const
{
	// failure = quit 如果LastResult == EBTNodeResult::Failed则返回到父节点
	int32 NextChildIdx = BTSpecialChild::ReturnToParent;

	if (PrevChild == BTSpecialChild::NotInitialized)
	{
		// newly activated: start from first
		NextChildIdx = 0;
	}
    // 如果LastResult == EBTNodeResult::Succeeded，则找下一个分支
	else if (LastResult == EBTNodeResult::Succeeded && (PrevChild + 1) < GetChildrenNum())
	{
		// success = choose next child
		NextChildIdx = PrevChild + 1;
	}

	return NextChildIdx;
}
```

**Sequence节点**：顺序节点，依次执行下级节点，若下级的所有节点都返回 Succeeded，则Sequence节点本身返回 Succeeded；若任何一个下级节点返回 Failed，则停止执行后续的下级节点，并且Sequence节点本身返回 Failed；如果 Sequence 节点下方没有任务节点，返回 Failed。

### Selector节点

<img src="{{ '/styles/images/ai/Selector.png' | prepend: site.baseurl }}" />

```c++
class AIMODULE_API UBTComposite_Selector: public UBTCompositeNode
```

```c++
int32 UBTComposite_Selector::GetNextChildHandler(FBehaviorTreeSearchData& SearchData, int32 PrevChild, EBTNodeResult::Type LastResult) const
{
	// success = quit 如果LastResult == EBTNodeResult::Succeeded，则返回到父节点
	int32 NextChildIdx = BTSpecialChild::ReturnToParent;

	if (PrevChild == BTSpecialChild::NotInitialized)
	{
		// newly activated: start from first
		NextChildIdx = 0;
	}
    // 如果LastResult == EBTNodeResult::Failed，则找下一个分支
	else if (LastResult == EBTNodeResult::Failed && (PrevChild + 1) < GetChildrenNum())
	{
		// failed = choose next child
		NextChildIdx = PrevChild + 1;
	}
    
	return NextChildIdx;
}
```

**Selector节点**：选择节点，从左到右依次选择执行下级节点，若有任何一个节点返回 Succeeded，则停止执行后续下级节点，并返回 Succeeded；若全部的下级节点都返回 Failed，则此 Selector返回 Failed；如果 Selector 节点下方没有任务节点，返回 Failed。

<img src="{{ '/styles/images/ai/Scope.png' | prepend: site.baseurl }}" />

**ApplyDecoratorScope选项**：Sequence 和 Selector 都有 `ApplyDecoratorScope` 选项，意思是开启装饰器的作用域，当勾选 `ApplyDecoratorScope` 则这个Composites下级节点的装饰器，若执行状态不在所处的作用域，则装饰器是无效的。

如下图：勾选上后Composites下方会有一行字“观察器的局部作用域”，之后运行开始后即使任务节点改变布尔值为true，也没有被abort，依旧将Sequence运行完才运行左侧节点

<img src="{{ '/styles/images/ai/ApplyDecoratorScope.png' | prepend: site.baseurl }}" />


### SimpleParallel节点

<img src="{{ '/styles/images/ai/SimpleParallel.png' | prepend: site.baseurl }}" />

```c++
class AIMODULE_API UBTComposite_SimpleParallel : public UBTCompositeNode
```

```c++
int32 UBTComposite_SimpleParallel::GetNextChildHandler(FBehaviorTreeSearchData& SearchData, int32 PrevChild, EBTNodeResult::Type LastResult) const
{
	FBTParallelMemory* MyMemory = GetNodeMemory<FBTParallelMemory>(SearchData);
	int32 NextChildIdx = BTSpecialChild::ReturnToParent;
	//第一次tick会执行主任务
	if (PrevChild == BTSpecialChild::NotInitialized)
	{
		NextChildIdx = EBTParallelChild::MainTask;
		MyMemory->MainTaskResult = EBTNodeResult::Failed;
		MyMemory->bRepeatMainTask = false;
	}
    //如果主任务还在执行中或者平行节点被标记为“Delayed”，则执行次要任务
	else if ((MyMemory->bMainTaskIsActive || MyMemory->bForceBackgroundTree) && !SearchData.OwnerComp.IsRestartPending())
	{
		// if main task is running - always pick background tree
		// unless search is from abort from background tree - return to parent (and break from search when node gets deactivated)
		NextChildIdx = EBTParallelChild::BackgroundTree;
		MyMemory->bForceBackgroundTree = false;
	}
	else if (MyMemory->bRepeatMainTask)
	{
		UE_VLOG(SearchData.OwnerComp.GetOwner(), LogBehaviorTree, Verbose, TEXT("Repeating main task of %s"), *UBehaviorTreeTypes::DescribeNodeHelper(this));
		NextChildIdx = EBTParallelChild::MainTask;
		MyMemory->bRepeatMainTask = false;
	}

	if ((PrevChild == NextChildIdx) && (MyMemory->LastSearchId == SearchData.SearchId))
	{
		// retrying the same branch again within the same search - possible infinite loop
		SearchData.bPostponeSearch = true;
	}

	MyMemory->LastSearchId = SearchData.SearchId;
	return NextChildIdx;
}
```

**SimpleParallel节点**：并行节点，左边紫色的部分，必须连接一个任务节点，可称这个任务为“主要任务”，右边灰色的部分可以连 Composites 节点或者任务节点，是与主要任务并行执行（同时执行）逻辑所在的位置。这也意味着你可以在**并行节点**的并行部分再放**并行节点**。

当主要任务返回 Succeeded，此SimpleParallel 节点返回 Succeeded；若主要任务返回 Failed，此 SimpleParallel 返回 Failed；SimpleParallel 的返回结果和并行部分的结果没有关系。

<img src="{{ '/styles/images/ai/FinishMode.png' | prepend: site.baseurl }}" />

并行节点有一个 `FinishMode` 选项。

- 立即（Immediate）
  - 主任务完成后，后台树的执行将立即中止。

- 推迟（Delayed）
  - 主任务完成后，允许后台树继续执行直至完成。

`BTComposite_SimpleParallel.h`

```c++
namespace EBTParallelMode
{
	// keep in sync with DescribeFinishMode

	enum Type
	{
		AbortBackground UMETA(DisplayName="Immediate" , ToolTip="When main task finishes, immediately abort background tree."),
		WaitForBackground UMETA(DisplayName="Delayed" , ToolTip="When main task finishes, wait for background tree to finish."),
	};
}
```

## BTTaskNode

任务节点的功能是实现操作，例如移动AI或调整黑板值。它们可以连接至 [装饰器（Decorators）](https://docs.unrealengine.com/5.2/zh-CN/unreal-engine-behavior-tree-node-reference-decorators)节点 或 [服务（Services）](https://docs.unrealengine.com/5.2/zh-CN/unreal-engine-behavior-tree-node-reference-services)节点，他们是行为树中的叶子节点。

```c++
/** 
 * Task are leaf nodes of behavior tree, which perform actual actions
 *
 * Because some of them can be instanced for specific AI, following virtual functions are not marked as const:
 *  - ExecuteTask
 *  - AbortTask
 *  - TickTask
 *  - OnMessage
 */
```

每个任务节点都有结束状态，任务节点的状态由 EBTNodeResult::Type决定：

<img src="{{ '/styles/images/ai/EBTNodeResult.png' | prepend: site.baseurl }}" />

FinishAbort用在ExecuteTask里是无效的, 只有在AbortTask里才可用.Task自身执行的结果只有成功或失败, 是否被打断不是自己能决定的.

<img src="{{ '/styles/images/ai/FinishAbort.png' | prepend: site.baseurl }}" />

蓝图节点优先执行判断 `BTTask_BlueprintBase.cpp`

```c++
if (AIOwner != nullptr && (ReceiveExecuteImplementations & FBTNodeBPImplementationHelper::AISpecific))
{
    ReceiveExecuteAI(AIOwner, AIOwner->GetPawn());
}
else if (ReceiveExecuteImplementations & FBTNodeBPImplementationHelper::Generic)
{
    ReceiveExecute(ActorOwner);
}
```

其中ReceiveExecuteImplementations 的注释为set if ReceiveExecute is implemented by blueprint（而且**UBTTask_BlueprintBase的注释为**Base class for blueprint based task nodes. Do NOT use it for creating native c++ classes!）

```c++
ReceiveExecuteImplementations = FBTNodeBPImplementationHelper::CheckEventImplementationVersion(TEXT("ReceiveExecute"), TEXT("ReceiveExecuteAI"), *this, *StopAtClass);
```

`BTFunctionLibrary.cpp`

```c++
int32 CheckEventImplementationVersion(FName GenericEventName, FName AIEventName, const UObject& Object, const UClass& StopAtClass)
{
    const bool bGeneric = BlueprintNodeHelpers::HasBlueprintFunction(GenericEventName, Object, StopAtClass);
    const bool bAI = BlueprintNodeHelpers::HasBlueprintFunction(AIEventName, Object, StopAtClass);

    return (bGeneric ? Generic : NoImplementation) | (bAI ? AISpecific : NoImplementation);
}
```

你会发现`FBTNodeBPImplementationHelper`的条件判断使用的是位运算这是由于他结构的设计

```c++
namespace FBTNodeBPImplementationHelper
{
    static const int32 NoImplementation = 0;
    static const int32 Generic = 1 << 0;
    static const int32 AISpecific = 1 << 1;
    static const int32 All = Generic | AISpecific;
}
```

>至于为什么设计两个版本，我先挖个坑

另外根据写法可以将之分为:同帧完成后即调用Finish和在此节点持续一段时间后才调用Finish, 两类

<img src="{{ '/styles/images/ai/OneFrame.png' | prepend: site.baseurl }}" />

这种属于第一种，C++写法可以参考UE节点`FinishWithResult`

<img src="{{ '/styles/images/ai/FinishWithResult.png' | prepend: site.baseurl }}" />

```c++
/**
 * Instantly finishes with given result
 */
UCLASS()
class AIMODULE_API UBTTask_FinishWithResult : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_FinishWithResult(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual FString GetStaticDescription() const override;
    
protected:
    /** allows adding random time to wait time */
    UPROPERTY(Category = Result, EditAnywhere)
    TEnumAsByte<EBTNodeResult::Type> Result;
};


UBTTask_FinishWithResult::UBTTask_FinishWithResult(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    NodeName = "FinishWithResult";
    Result = EBTNodeResult::Succeeded;
}

EBTNodeResult::Type UBTTask_FinishWithResult::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    return Result;
}

FString UBTTask_FinishWithResult::GetStaticDescription() const
{
    return FString::Printf(TEXT("%s %s"), *Super::GetStaticDescription()
       , *UBehaviorTreeTypes::DescribeNodeResult(Result.GetValue()));
}
```



<img src="{{ '/styles/images/ai/DelayFrame.png' | prepend: site.baseurl }}" />

这种属于第二种，C++写法可以参考UE节点`Wait`

<img src="{{ '/styles/images/ai/Wait.png' | prepend: site.baseurl }}" />

```c++
/**
 * Wait task node.
 * Wait for the specified time when executed.
 */
UCLASS()
class AIMODULE_API UBTTask_Wait : public UBTTaskNode
{
    GENERATED_UCLASS_BODY()

    /** wait time in seconds */
    UPROPERTY(Category = Wait, EditAnywhere, meta = (ClampMin = "0.0", UIMin = "0.0"))
    float WaitTime;

    /** allows adding random time to wait time */
    UPROPERTY(Category = Wait, EditAnywhere, meta = (UIMin = 0, ClampMin = 0))
    float RandomDeviation;

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual uint16 GetInstanceMemorySize() const override;
    virtual void DescribeRuntimeValues(const UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTDescriptionVerbosity::Type Verbosity, TArray<FString>& Values) const override;
    virtual FString GetStaticDescription() const override;

#if WITH_EDITOR
    virtual FName GetNodeIconName() const override;
#endif // WITH_EDITOR

protected:

    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
```

其中最关键的三个函数

```c++
UBTTask_Wait::UBTTask_Wait(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    NodeName = "Wait";
    WaitTime = 5.0f;
    //自动设置bNotifyTick = true
    INIT_TASK_NODE_NOTIFY_FLAGS();
}

EBTNodeResult::Type UBTTask_Wait::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    FBTWaitTaskMemory* MyMemory = CastInstanceNodeMemory<FBTWaitTaskMemory>(NodeMemory);
    MyMemory->RemainingWaitTime = FMath::FRandRange(FMath::Max(0.0f, WaitTime - RandomDeviation), (WaitTime + RandomDeviation));
    
    return EBTNodeResult::InProgress;
}

void UBTTask_Wait::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    FBTWaitTaskMemory* MyMemory = CastInstanceNodeMemory<FBTWaitTaskMemory>(NodeMemory);
    MyMemory->RemainingWaitTime -= DeltaSeconds;

    if (MyMemory->RemainingWaitTime <= 0.0f)
    {
       // continue execution from this node
       FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
    }
}
```

其中官方注释：

>Ticks this task this function should be considered as const (don't modify state of object) if node is not instanced! 
>
>**bNotifyTick must be set to true for this function to be called. Calling INIT_TASK_NODE_NOTIFY_FLAGS in the constructor of the task will set this flag automatically**

所以现在好多教程是这样写的：

```c++
// 持续一段时间的任务

UBTTask_MyTask::UBTTask_MyTask()
{
    bNotifyTick = true; // 执行TickTask()的必要设置
}

void UBTTask_MyTask::TickTask(UBehaviorTreeComponent& OwnerComp, 
    uint8* NodeMemory, float DeltaSeconds)
{
    // check 一段时间后结束
    if(/*满足条件*/) FinishLatentTask(*BehaviorComp, EBTNodeResult::Succeeded);
}
```

## BTAuxiliaryNode

辅助节点，包括Decorator和Service两个节点，他既可附着在合成节点上，也可以附着在任务节点上，无法独立创建此节点

```c++
class AIMODULE_API UBTAuxiliaryNode : public UBTNode
```

### Decorator节点

```c++
class AIMODULE_API UBTDecorator : public UBTAuxiliaryNode
```

装饰器节点可以连接到[合成（Composite）](https://docs.unrealengine.com/5.2/zh-CN/unreal-engine-behavior-tree-node-reference-composites)或[任务（Task）](https://docs.unrealengine.com/5.2/zh-CN/unreal-engine-behavior-tree-node-reference-tasks)节点，主要有两个作用：

- 控制节点是否可以执行，即作为条件判断
- 中止正在执行的分支，并跳转到其他分支

另外装饰器有一个`ObserverAborts`选项

<img src="{{ '/styles/images/ai/ObserverAborts.png' | prepend: site.baseurl }}" />

```c++
UENUM()
namespace EBTFlowAbortMode
{
	enum Type
	{
		None				UMETA(DisplayName="Nothing"),
		LowerPriority		UMETA(DisplayName="Lower Priority"),
		Self				UMETA(DisplayName="Self"),
		Both				UMETA(DisplayName="Both"),
	};
}
```

- None：不会中止流程，仅作条件判断，如果当前分支正在执行，会等待执行完毕
- LowerPriority：装饰器结果从true到false，若当前节点或优先级高的节点或优先级低的节点（装饰器右边节点）正在执行，都无中止；装饰器结果从false到true，若当前节点或优先级高的节点正在执行，则无中止，若优先级低的节点（装饰器右边节点）正在执行，则中止并从这个装饰器节点开始执行而不会执行高优先级节点除非高优先级节点中也有同样条件的装饰器中止。
- Self：若当前节点分支正在执行时，装饰器结果从true到false，中止装饰器所在的节点及其分支
- Both：装饰器结果从true到false，若当前节点正在执行，则中止当前节点，并从装饰器附着节点的父级查找可执行节点，若父级是Selector则查找低优先级子节点，若优先级高的节点正在执行，则无中止；装饰器结果从false到true，若优先级高的节点正在执行，则无中止，若优先级低的节点（装饰器右边节点）正在执行，则中止并于当前装饰器附着节点开始。

官方注释`BehaviorTreeComponent.cpp`

```c++
// search range depends on decorator's FlowAbortMode:
//
// - LowerPri: try entering branch = search only nodes under decorator
//
// - Self: leave execution = from node under decorator to end of tree
//
// - Both: check if active node is within inner child nodes and choose Self or LowerPri
```

不过FlowAbortMode在不同的情况下有不同选项，具体是：

- 当附着的节点的父节点为Selector时候，有四个选项：None、LowerPriority、Self和Both，这与Selector设计目的不冲突，它允许装饰器根据自己的逻辑选一个分支
- 当附着的节点的父节点为Sequence时，只有两个选项None和Self，因为Sequence设计必须为按照顺序执行，不允许装饰器随意选择分支

```c++
bool UBTComposite_Sequence::CanAbortLowerPriority() const
 {
   // don't allow aborting lower priorities, as it breaks sequence order and doesn't makes sense
    return false;
 }
```

- 当附着的节点为Simple Parallel时，其只有None选项，因为Simple Parallel为并发节点，不能终止任务分支执行



还有Flow Control

<img src="{{ '/styles/images/ai/FlowControl.png' | prepend: site.baseurl }}" />

`On Result Change`当检测的值越界时进行重新评估该条件产生的结果

`On Value Change`当检测的值发生变化时进行重新评估该条件产生的结果

`Key Query`

- Is Set：初始化或为true

- 有些如int/float等有==/!=/>/>=等并且可以填写与之比较的Key Value

<img src="{{ '/styles/images/ai/KeyQuery.png' | prepend: site.baseurl }}" />


>再挖个坑：装饰器写法可以参考UE的BlackboardBasedCondition

### Service节点

```c++
class AIMODULE_API UBTService : public UBTAuxiliaryNode
```

服务节点的主要作用就是在后台更新数据（ 比如修改黑板变量以供装饰器用），但并不会返回结果，也不能直接影响行为树的执行流程，通常它可以配合装饰器节点来实现复杂的功能。（有时使用SimpleParallel也可以达到相同效果）

不过要注意的是Service节点附着在Composite节点和Task节点上时执行顺序不同

<img src="{{ '/styles/images/ai/Service1.png' | prepend: site.baseurl }}" />
<img src="{{ '/styles/images/ai/Service2.png' | prepend: site.baseurl }}" />

具体可以参考BehaviorTreeManager里InitializeNodeHelper方法，大致逻辑就是(代码逻辑我再挖个坑)：

1. 为当前节点生成ExecutionIndex，然后ExecutionIndex自增1
2. 如果当前节点为合成节点，则为它的所有Service节点生成ExecutionIndex，ExecutionIndex依次自增，这里就是合成节点的ExecutionIndex会比它的Service节点小的原因
3. 如果当前节点为合成节点，开始判断它的孩子节点，如果为ChildTask节点（即任务节点），则先生成ChildTask的所有Service节点的ExecutionIndex，ExecutionIndex依次自增，这就是任务节点的ExecutionIndex会比它的Service节点的大的原因
4. 递归调用InitializeNodeHelper

<img src="{{ '/styles/images/ai/Interval.png' | prepend: site.baseurl }}" />

`Interval`和`Random Deviation`就是附着的节点在执行时每10-0到10+0秒（上述图片的例子）此Service节点的Tick函数调用一次

## ROOT节点

<img src="{{ '/styles/images/ai/ROOT.png' | prepend: site.baseurl }}" />

最后，蓝图中有一个`ROOT`节点，它虽然是行为树的根，但它并不是逻辑节点，运行时的行为树中并没有该节点，这一点从UBehaiorTree类的结构就能看出来，他是用来持有黑板数据的。










## 参考

[行为树的常见问题以及您应该避免的事情](https://dev.epicgames.com/community/learning/tutorials/L9vK/unreal-engine-common-issues-with-behavior-trees-and-things-you-should-competely-avoid)

[游戏开发误区：玩家想要智能人工智能](https://askagamedev.tumblr.com/post/76972636953/game-development-myths-players-want-smart)

[解决AI Move to和Nav Proxy不工作的问题20种方法](https://forums.unrealengine.com/t/fixed-ai-move-to-nav-proxy-not-firing-20-things-to-try/270575)

使用AI导航时取消勾选被导航的pawn上面的`Can Ever Affect Navigation`，并且pawn所附带的所有带有此布尔值的actor都要取消。不然，如果执行`Show Navigation`命令，你会看到角色周围的地面无法导航，并且随着角色在世界中移动，这种情况会不断更新。

[行为树子树](https://zhuanlan.zhihu.com/p/282059321)

[官方快速入门](https://docs.unrealengine.com/5.3/zh-CN/behavior-tree-in-unreal-engine---quick-start-guide/)
