---
layout: post
title: "UE Beginplay"
date: 2020-10-20 09:00:00 +0800 
categories: UE
tag: 基础
---
* content
{:toc #markdown-toc}

这是一篇关于UE中蓝图Beginplay和C++中Beginplay

<!-- more -->

官方文档参考：[**Actor Lifecycle**](https://docs.unrealengine.com/5.3/en-US/unreal-engine-actor-lifecycle/) 

# Beginplay

One C++ Constructor > Corresponding BP Constructors > Repeat / One C++ Begin Play > Corresponding BP Begin Play 



蓝图的BeginPlay是在AActor::BeginPlay()方法中被调用的

```c++
void AXXX::BeginPlay()
{
	UE_LOG(LogTest, Warning, TEXT("CPP 1 Begin"));
	Super::BeginPlay();
    UE_LOG(LogTest, Warning, TEXT("CPP 2 Begin"));
}
```

那么会看到

```ini
LogTest: Warning: CPP 1 Begin
LogBlueprintUserMessages: [BP_XXX_0] Blueprint Beginplay
LogTest: Warning: CPP 2 Begin
```

继承于Actor的对象在调用Super::BeginPlay以后会调到AActor::BeginPlay()

```c++
void AActor::BeginPlay()
{
	TRACE_OBJECT_EVENT(this, BeginPlay);

	ensureMsgf(ActorHasBegunPlay == EActorBeginPlayState::BeginningPlay, TEXT("BeginPlay was called on actor %s which was in state %d"), *GetPathName(), (int32)ActorHasBegunPlay);
	SetLifeSpan( InitialLifeSpan );
	RegisterAllActorTickFunctions(true, false); // Components are done below.

	TInlineComponentArray<UActorComponent*> Components;
	GetComponents(Components);

	for (UActorComponent* Component : Components)
	{
		// bHasBegunPlay will be true for the component if the component was renamed and moved to a new outer during initialization
		if (Component->IsRegistered() && !Component->HasBegunPlay())
		{
			Component->RegisterAllComponentTickFunctions(true);
			Component->BeginPlay();
			ensureMsgf(Component->HasBegunPlay(), TEXT("Failed to route BeginPlay (%s)"), *Component->GetFullName());
		}
		else
		{
			// When an Actor begins play we expect only the not bAutoRegister false components to not be registered
			//check(!Component->bAutoRegister);
		}
	}

	if (GetAutoDestroyWhenFinished())
	{
		if (UWorld* MyWorld = GetWorld())
		{
			if (UAutoDestroySubsystem* AutoDestroySys = MyWorld->GetSubsystem<UAutoDestroySubsystem>())
			{
				AutoDestroySys->RegisterActor(this);
			}			
		}
	}

	ReceiveBeginPlay();

	ActorHasBegunPlay = EActorBeginPlayState::HasBegunPlay;
}
```

其中倒数第二行调用到了**BlueprintImplementableEvent**的方法`ReceiveBeginPlay()`，这个就是蓝图里的Beginplay

```c++
protected:
	/** Event when play begins for this actor. */
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName = "BeginPlay"))
	void ReceiveBeginPlay();
```

另外在调用此函数前遍历了此Actor的组件，因此组件的Beginplay会更早

```c++
/** 
	 * Blueprint implementable event for when the component is beginning play, called before its owning actor's BeginPlay
	 * or when the component is dynamically created if the Actor has already BegunPlay. 
	 */
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName = "Begin Play"))
	void ReceiveBeginPlay();
```



> 参考：[C++ for Blueprints](https://www.youtube.com/watch?v=6485d5Zoc_k)
