---
layout: post
title: "Audio-Wwise"
date: 2026-05-01 09:00:00 +0800 
categories: UE
tag: Audio
---
* content
{:toc #markdown-toc}

这是一篇关于UE音频的一些知识

<!-- more -->

> 链接：[《明日方舟：终末地》：大型游戏项目的音频系统架构](https://www.audiokinetic.com/zh/community/blog/arknights-endfield/#wwise-unity-integration)

先挖个坑，以后可能会研究

## 以 Wwise Unity Integration 为基石

Wwise Unity Integration 是 Audiokinetic 提供的一套强大且易用的音频开发工具。它覆盖从音频引擎的初始化、各种基本音频功能的使用，到音频对象的管理、空间音频系统的搭建等方方面面。具备十分强大的通用性。且配备了强大的 Wwise Picker 工具，方便设计师在 Unity 引擎中和 Wwise 工程交互。

对于大部分小型至中型项目来说，Wwise 所提供的原生整合完全可以满足开发需求。《终末地》也从这里开始。

然而，作为一款大型商业项目，我们很快就遇到了通过原生整合所提供的通用功能无法解决的需求。比如“集成工业(Automated Industry Complex, AIC)”系统。玩家可以在这套系统中，布置各种的生产建筑，使用传送带、管道连接形成流水线，实现游戏中产品的自动化大批量生产。一块集成工业区域中可能部署了数百个建筑。面对如此大量的对象，如果不加以管理，全部同时发声，无论是效果上还是性能上都是无法接受的。

![01-AIC](https://1940263.fs1.hubspotusercontent-na1.net/hubfs/1940263/Arknights%20Endfield/01-AIC.png)

又比如，为了给玩家足够沉浸的体验，我们为每个角色的不同鞋子踩在不同表面上的声音、不同角色身上挂件的声音都做了十分精细的设计，目前总计使用了接近 100,000 个样本。得益于 Wwise 在音频领域多年的深耕，面对巨量的样本，Wwise 无论是在编辑时还是运行时，仍然能快速、稳定地工作。不过，在游戏运行的过程中，如何合理地进行资源的加载和卸载也同样十分重要。

下面我将从最基本的游戏对象管理开始，向大家展示我们音频系统的架构，并介绍我们解决一个又一个问题的思路。

## 游戏对象的管理

在 Unity 的传统框架中，使用带有各种不同 Component 的 Unity GameObject 来实现各种不同的功能。原生的 Wwise Unity 集成依赖于 Unity 所提供的 GameObject 对象。

我们要在一个 Unity GameObject 上播放声音，则至少需要在它身上添加一个 AkGameObj 组件。AkGameObj 的作用（在 Unreal 中为 AkComponent）主要是向 Wwise 注册一个 Wwise GameObject，并同步位置等信息。游戏引擎与音频引擎交互时，需要通过 Wwise GameObject 的 ID 来进行。在 Unity 中，该 ID 则由 AkGameObj 通过 GetHashCode() 方法来获得。

如果你是一个 Unity 开发者，你应该听说过 Unity 提供的 DOTS 技术栈及 ECS 开发框架。《终末地》也使用了类似的方法来处理海量的实体。同时，我们还保留了传统的 GameObject，在演出、特效等模块使用。对于这些不依赖于原生 GameObject 的对象或实体，又该如何处理呢？

### ID 分配器

由于 AkGameObj 通过 Unity GameObject 的哈希值来作为 Wwise GameObject 的 ID。在单纯使用 Unity GameObject 这个体系时，出现哈希碰撞的概率非常小。然而如果我们需要混合多个系统使用，那么我们就不得不考虑多个系统之间的 ID 碰撞等问题了。为了解决这个问题，我们实现了一个统一的分配器（Dispatcher），每个系统在需要使用到 ID 时都从该分配器获取一个 ID，由分配器来保证 ID 的唯一性。而分配器的核心实现，只是简单的自增机制。

分配器的基本实现的伪代码如下：

```
public static class AudioIdDispatcher
{
    private static ulong s_currentId;
    public static ulong GetId()
    {
        return s_currentId++;
    }
} 
```

Wwise GameObject 的 ID 是 64 位的整数，这意味着我们可以使用的数量达到了 2^64 个。 假设我们每秒钟分配 100 个 ID，游戏也需要连续运行 58 亿年才会把 ID 消耗完，我们完全不需要担心 ID 溢出的问题。 当然，这些 ID 中还存在一些 Wwise 作为保留作用的 ID 需要避免（详情见 Wwise SDK 的[文档](https://www.audiokinetic.com/en/public-library/2025.1.7_9143/?source=SDK&id=namespace_a_k_1_1_sound_engine_a895ed0f83a0dea8fc284491c0ee0152c.html#a895ed0f83a0dea8fc284491c0ee0152c)）

### AudioAdapter

为保证所有音频代码使用统一的 ID分配器，并隔离底层 Wwise 实现，我们引入了一个抽象的中间层 AudioAdapter。我们利用 C# 的 Assembly 机制，在 Wwise Unity Integration 的基础上，新增了一个 Audio 的 Assembly。项目中除了该 Assembly，其他的代码一律无法访问 Wwise Unity Integration 原本提供的代码。Wwise Unity Integration 的接口提供了 GameObject 相关的功能，而我们又通过一个中间层隐藏了这些实现。在我们的这个 Adapter 所提供的所有接口中，都只能通过 ulong 类型的 ID 来操作 Wwise GameObject。Adapter 的这一设计不仅解决了ID冲突，也在后续资产管理、性能监控、调试等方面带来了意想不到的便利。

大部分游戏中可能还会拥有一个 AudioManager 类，我们也不例外。但是 AudioManager 类在我们的项目中，更多的是管理 GamePlay 相关的音频逻辑，各个与 GamePlay 耦合的子模块。比如音乐的切换、跟随场景进行流加载的音频对象、环境声系统等。在后续 Gameplay 的部分将会进一步展开。

### 音频对象

让每个用到音频系统的调用方都手动通过 ID 分配器来管理音频 ID，无疑是一种负担。我们需要提供一些更简便的方法来给其他系统使用。

基于“组合优于继承”的思想，我们的思路是向其他系统提供经过封装的 AudioObject（这里指的不是 Object-Base Audio 中的 AudioObject）。

“组合优于继承（[Composition over inheritance](https://en.wikipedia.org/wiki/Composition_over_inheritance)）”的思想并不是说不使用继承，而是将本身耦合程度不高的系统，通过组合而非继承的方式来实现。在我们的例子里，具体而言就是音频系统和其他系统。

AudioObject 本身通过继承来提供各种带有不同功能的 AudioObject。比如一个非常常见的需求：临时发声体，在某个位置播放一个声音，等待声音播放结束之后就销毁自己的发声体。AudioObject 的不同子类封装了各类常见的音频行为模式。调用方根据自身的需求，选择某种 AudioObject 组合到自己的系统中，并持有对 AudioObject 的引用，然后通过使用 AudioObject 提供的方法来与音频系统交互。

举一个具体的例子，如果我们要在 GameObject 上播放声音，此时不是实现一个继承自 AudioObject 的 Component 挂在 GameObject 上，而是实现一个持有 AudioObject 的 Component。由此保持游戏逻辑与音频系统的清晰边界，避免让继承结构变得僵化且难以维护。

```
public class AudioObject : IDisposable
{
    public ulong audioObjectId => m_audioObjectId;
    protected ulong m_audioObjectId;
    
    public AudioObject()
    {
        m_audioObjectId = AudioObjectIdDispatcher.GetId();
        AkSoundEngine.RegisterGameObject(m_audioObjectId, someDebugName);
    }
    
    public void Dispose() { /* ... */ }
}

public class AudioEmitterObject : AudioObject
{
    public void SetPosition() { /* ... */ }
    public void DrawDebugGizmos() { /* ... */ }
    // ...
}

public class AudioTempEmitterObejct : AudioObjectBase
{
    // ...
} 
public class AudioComponent : MonoBehaviour
{
    private AudioEmitterObject m_audioObj;
    
    private void Update()
    {
        m_audioObj.SetPosition(transform.Position);
    }
    // ...
} 
```

## Logger

不知道大家在游戏开发的过程中是否经常会遇到这样的情况：

- 游戏里突然出现了一个奇怪的声音，当你要查找这个声音是从哪里来的时候，却难以下手？
- 人物在没有水的地面上走动时，却播放了踩在水面上的脚步声？
- 又或者，同事非常偶然地触发了一个不该出现的声音，而你在自己的电脑上却无论如何也复现不了？

为了应对这种情况，我们在几个重要的音频调用入口处，都埋下了日志的打印。Unity 的日志是默认带有调用栈的，在刚刚提到的情况中，我们就可以到同事运行游戏的电脑上，在日志中搜索错误触发的事件，然后从调用栈中看到触发的来源。而上面说到的 AudioAdapter 层的代码帮了我们很多大忙的其中一个，就是它成为了埋日志的地方。

虽然这么做可以让我们事无巨细地看到每一个音频请求，但是在我们的游戏中，音频的触发频率可能会很高，一秒钟的时间可能产生上百条 Log，而打印带有调用栈的 Log 也非常耗时，如果不加以控制，日志系统将成为开发过程中的一个性能瓶颈。

我们需要一些手段来管理海量的日志，于是我们写了一套 AudioLogger 系统。该系统非常重要的一个功能是将 Log 按不同的类别（Channel / Category）区分开来，并可以动态地开关每个类别。AudioLogger 结合一些条件编译的手段或者实时的判断，做到当某个类别没有开启时，几乎不产生任何性能开销。并且杜绝在 Release Build 版本中打印无用的日志。

```
public class AudioLogger
{
    // 使用条件宏，Release Build中根本不会调用，也没有参数计算的开销
    [Conditional("Debug")]
    public void Log<t1, t2="">(EAudioLogChannel channel, string str, T1 param1, T2 param2)
    {
        if (NotEnabled(channel)
        {
            return;
        }
        // Some String Formatting Code
        Debug.Log(finalString);
    }
} 
```

![02-AudioLogChannel](https://1940263.fs1.hubspotusercontent-na1.net/hubfs/1940263/Arknights%20Endfield/02-AudioLogChannel.png)

此外，传到 AudioAdapter 的请求中，音频对象是基于 ID 的。这也意味着，如果我们需要打印与音频对象相关的 Log，我们需要一些手段来把该对象的名字还原回字符串。后续 Debug 一节我将详细说明我们是怎么实现的。

总之，我们可以通过一个专用的 Logger 系统，将我们的日志信息和游戏的其他部份分离开来并加以管理。通过类别机制来避免不同的人在类似的操作下，使用不同的关键字，导致调试时难以利用日志信息。在这里付出的努力是非常值得的。

## 音频资源管理

《终末地》的 Wwise 工程中目前合计存在超过 150,000 个 Sound 对象。这还不包含使用 External Source 进行管理的语音资产。然而，在移动平台上，我们的音频内存预算只有 50MB。面对数量如此庞大的对象，我们是如何合理地进行管理，尽量减少运行时的内存使用呢？

我们主要使用了 Auto-Defined SoundBanks 再结合一些其他方法，比如合理使用 Streaming，引入强调避免内存碎片等内存分配器等，来达成这个目标。下图展示了某一次安卓手机的跑测中，20分钟的 Wwise 内存使用情况，可以看到其呈现出一条平稳的直线。

![03-AudioMemUsage](https://1940263.fs1.hubspotusercontent-na1.net/hubfs/1940263/Arknights%20Endfield/03-AudioMemUsage.png)

通过精细的内存管理，我们成功将内存占用控制在预算内。

### Auto-Defined SoundBanks

传统的手动分配 Bank 的方法，占据音频设计师大量的时间，容易引入错误，还需要在游戏系统中考虑加载卸载的时机。不仅如此，根据二八定律，往往手动划分的 Bank 中，每个 Bank 中只有少量资产是经常被使用的，而剩下大部分偶尔被使用的资产，却不得不一起加载，占据大量的内存空间造成浪费。如果可以使得 Bank 的划分更加自动化、粒度更小就好了！

Wwise 在很多年前就已经在 Unreal 整合中推行 EBP 的方案，后续由 SSOT 系统接替了 EBP。再后来，Wwise 在 2022 版本中引入了 Auto-Defined Banks 的概念 [Automatically Defining SoundBanks | Audiokinetic](https://www.audiokinetic.com/en/public-library/2022.1.19_8584/?source=Help&id=auto_defining_soundbank)。Auto-Defined Banks 自动为每一个音频事件、每一个媒体生产一个独立的 Bank。

这带来了几个好处。此前，如果要使用类似的方法管理资产，需要像 Unreal 的 EBP 系统，通过外部系统来为每个事件创建 Bank，且需要额外维护引用关系。而现在，通过 Auto-Defined Banks，我们可以完全不需要操心 Bank 如何划分且大量节省内存的使用。

比较不巧的是，当我们决定采用这一新推出的方案时，Wwise Unity Integration 尚未提供官方支持。这促使我们基于其设计理念，自主实现了一套管理系统。这也为我们对于资产的管理带来了巨大的灵活性，为后续优化打下了坚实的基础。

### 简洁的异步加载

使用 AutoBank，意味着我们需要随时随地进行资产的加载。而每次进行资产的加载时，我们必须使用异步的方式来防止卡顿游戏。

但是，如果每个想要 PostEvent 的地方，都需要手动编写加载和卸载的代码，那可太麻烦了！

因为每个 Event 的加载都需要至少经过 LoadBank 和 Preparae 两个阶段。LoadBank 加载了我们要使用的 Event 所在的 Bank，包含该自身的元数据。而 PrepareEvent 则让 Wwise 自动查找该 Event 有哪些引用，并加载被引用的其他 Bank。在这两个步骤都执行完毕之后，才可以真正执行 PostEvent 操作。又比如，如果多个按照一定顺序执行的系统，我们想要按照系统执行的顺序来调用 Event。如果它们各自实现自己的加载逻辑，又该如何保证调用的顺序呢？

于是我将再次搬出 AudioAdapter，这次它又帮了我们一个大忙！

我们希望任何地方在想要 PostEvent 时，都依旧和之前一样简单：PostEvent，然后就不用管它了！因此，我们将资产的加载逻辑，也放在了 AudioAdapter 中的 PostEvent 接口内部，透明地实现了资源的加载过程。

当调用 AudioAdapter 所提供的 PostEvent 接口时，实际上先走到了资产管理系统，分别执行了 LoadBank, PrepareEvent 操作。最后再回过头来执行 PostEvent。每一步都是异步执行，并在完成后通过回调执行至下一步，对调用方完全无感。至此，我们为游戏逻辑提供了一个极其简洁的音频触发接口。其他人还是简单地使用一句 PostEvent 的代码，就完成了音频事件的调用，完美！

然而，将复杂性隐藏在接口之下，意味着所有复杂度都转移到了 AudioAdapter 的内部实现中。这套异步加载机制引出了一系列必须妥善解决的新问题：

- 在调用 AudioAdapter.PostEvent 的瞬间，就需要返回一个 PlayingId，但此时还没有执行 PostEvent 操作，怎么办？
- 假设我在异步加载完成之前，又执行了 Seek、Pause 等针对 PlayingId 的操作，这些操作是不是就失效了？
- 如何保证多个 Event 之间的相对顺序？
- 如何感知不再使用的 Event，在合适的时机卸载它们以释放内存占用？
- 是否有不能容忍延迟存在的事件？
- 每个 Event 一个 Bank，将产生大量零碎的小文件，IO 性能如何？
- ……

这里每一个问题都可以再花费大量的篇幅来展开，在这里可能没办法和大家分享所有的细节，但是可以和大家分享下我们解决这些问题的核心思路：

- AudioAdapter.PostEvent 执行时，生成一个自身维护的 ID，再通过一张 LUT，记录这个 ID 和实际 PlayingId 的映射关系
- 操作执行时，通过上述 LUT 表查找事件是否已经触发，若未触发则通过队列暂时缓存
- 在完成 PrepareEvent，即将 PostEvent 时，通过队列判断触发顺序，如果早于自身触发的事件还未加载完毕，则继续等待
- 引入合适的内存淘汰策略
- 引入预加载机制
- 在 Wwise FilePackger 的基础上进行了一些修改，主要是排序，来使得 IO 更高效

![04-AutoBnkLoadedMedia](https://1940263.fs1.hubspotusercontent-na1.net/hubfs/1940263/Arknights%20Endfield/04-AutoBnkLoadedMedia.png)

移动平台上，通过 Auto-Bank，内存中缓存了接近1000+个媒体文件，只使用了 14.6 MB 的内存


通过这一系列设计，我们将 Auto-Defined Banks 的粒度优势与异步加载的流畅性结合了起来，同时通过AudioAdapter 背后的调度，掩盖了加载流程的复杂度，最终实现了在严格内存限制下，对超大规模音频资产的高效、稳定、易用的管理。

## Gameplay 的音频交互

在搭建了资源管理、对象抽象等基础设施后，我们要再具体的游戏业务中开始接入音频功能了。我们再来看看《终末地》中，音频系统和 Gameplay 系统是怎么连接起来的。

### Listener

世界中首先要有一个 Listener 来收听 Emitter 发出的声音。

《终末地》在多种不同的游戏状态之间实现了无缝切换，比如从游戏局内到叙事演出，比如集成工业系统中的第三人称视角和俯瞰视角。每种不同的游戏状态，Listener 的位置可能通过不同的策略来决定，而不是简单地跟随摄像机移动。而不同的策略之间，也需要一定的手段来让声音平滑过渡。

于是，类似游戏中的摄像机管理系统，我们的 AudioManager 下有一个管理 Listener 的子系统，下称 ListenerManager。ListenerManager 中持有一个由 AudioObject 派生的 AudioListenerObject 对象。该对象在自身的初始化方法中，向音频引擎将自己注册为 Lisener，并且实现了平滑移动位置的算法、位置偏移的管理等。ListenerManager 本身的职责，则是决定从游戏世界中的什么对象获取坐标信息，然后采用什么策略来更新 AudioListenerObject 的位置，并在策略之间发生切换时，告知 AudioListenerObject 开始平滑过渡。

### Emitter

《终末地》的世界中存在多种类型的对象需要发出声音：角色、敌人、NPC、交互物、可以踢着玩的交通锥、集成工业系统的建筑……

对于每一类对象，与其相关的 AudioComponent 都持有一个 AudioObject，Component 类主要管理着与 Gameplay 相关的行为，并按需把这些行为转换为 Switch、RTPC 或不同的 Event 给 AudioObject。通过这种 Gameplay 系统持有并驱动 AudioObject 的模式，我们清晰地分离两者的职责，也使得 AudioObject 可以被非常灵活地组合到不同系统中。比如每个人物带有一个 CharacterAudioComponent，该 Component 在初始化时，会设置一系列的 Switch，来指定鞋型、服装类型等用于驱动脚步声等 Foley。

叙事演出中常常会使用长焦镜头进行视觉表达，摄像机实际距离被拍摄物体较远，但被拍摄物体却依旧占据了画面的主体。此时音频的衰减常常会大于我们相对于对画面的主观感受。针对这种情况，我们通过推导，先得出一个焦段和画面比例的关系公式，然后音效设计师通过主观感受，给出若干个焦距和衰减系数的对应关系数据，通过这些数据拟合出公式的系数。

人物类的发声体就使用上述公式通过镜头的焦距来设置衰减距离的 ScalingFactor，并计算其自身在屏幕内的占比 情况等因素，自适应地控制声音的细节，达到影视化的视听效果。

![05-AdaptiveScalingFactorTool](https://1940263.fs1.hubspotusercontent-na1.net/hubfs/1940263/Arknights%20Endfield/05-AdaptiveScalingFactorTool.png)

用于调试焦距和衰减 Scaling Factor 的工具


也有一些模块或系统，他们本身管理着若干个不与游戏世界中对象一一对应的发声体，比如环境声。这些系统本身直接持有并使用若干个 AudioObject。

![06-DialogInScreen](https://1940263.fs1.hubspotusercontent-na1.net/hubfs/1940263/Arknights%20Endfield/06-DialogInScreen.jpg)

叙事演出中用于计算屏幕占比的矩形

## 音频子系统

前面所提到的音频架构，可以主要总结为两个层次：基础设施与游戏交互。

基础设施负责对业务层隐藏一些细节，提供让业务层易于使用的接口。在应对《终末地》复杂的游戏内容时，我们发现在这两层之间，还需要一类高度封装、功能内聚的模块——即音频子系统。这部分内容可能根据项目实际情况的不同而不同，下面我将通过两个具体的例子来说明这一设计。

### 环境声

环境声系统负责构建游戏世界的听觉氛围，其特点是数量多、分布广泛、且需要与场景流式加载紧密配合。传统上，将音频组件直接挂在美术场景中的做法，在《终末地》这类使用无缝流式加载的大型项目中会遇到各种问题：跨部门协作容易出现数据冲突、数据难以拆分管理、运行时性能低下等。

因此，我们决定将其设计为一个独立的子系统。具体而言，我们将环境声分为三类进行管理：

- 可跟随关卡、任务状态而改变的动态声源
- 由人物、交互物、动画等驱动的动态声源
- 不跟随逻辑变化的，不产生位移的静态环境声

前两者与游戏系统本身会有比较紧密的耦合，大部分这类音效的接入都有音频策划与相应模块的策划或程序同事沟通完成。我们的重点在于第三个提到的静态环境声。

对于静态环境声，我们使用了“行为和数据分离”的思想，分离了编辑时的各种功能和运行时的管线。这彻底杜绝了与美术资源的提交冲突，并提升了运行时的效率。

编辑时，音效设计师需要打开音频场景，同时也可以叠加打开美术场景作为参考。然后在音频场景中放置各种音频组件的“Creator”。这些 Creator 仅存在于 Editor 下，用于提供丰富的编辑功能以及在场景文件中存储数据。

然后通过一系列烘焙流程，这些 Creator 的数据被提取为紧凑的二进制数据。

运行时，原始的场景文件并不会被直接加载，而是通过场景的流加载系统读取玩家当前所在位置，从上述烘焙后的二进制数据中读取所需的部分，并交由音频系统还原回各种发声体。

![07-AudioRoomCreator](https://1940263.fs1.hubspotusercontent-na1.net/hubfs/1940263/Arknights%20Endfield/07-AudioRoomCreator.png)

用于编辑 SpatialAudio 中 Room 的 Creator

这套设计充分体现了子系统的价值： 它将环境声所有复杂的编辑、数据管理和运行时逻辑封装在音频内部，对外仅提供简单的数据接口，与游戏逻辑彻底解耦。基于此，《终末地》得以在庞大而无缝的世界中，布置大量细腻的环境声细节，为玩家提供稳定而沉浸的听觉体验。

![08-AudioScene](https://1940263.fs1.hubspotusercontent-na1.net/hubfs/1940263/Arknights%20Endfield/08-AudioScene.png)

开发状态下的音频场景

### 语音

终末地每个版本都录制了大量的台词，并且同时完成多国语音的制作。语音系统面临着一组独特挑战。

- 需要管理多语言本地化
- 不能像音效一样简单地通过在 Wwise 中设置发声数限制来管理嘈杂度
- 同时使用了 Wwise 自身的本地化机制与 External Source 两种截然不同的方法来管理不同类型的语音
- 多个外部模块可能操作相同的说话对象

为了解决这些问题，我们又一次实践了“组合优于继承”和“分层架构”的思想来搭建我们的语音系统。

我们学习了 Wwise Event 的思路，将每一句语音使用一个 VoiceID 进行包装。VoiceID 背后记录了一句语音的各种细节数据。在最底层，有一个类似 AudioAdapter 的类，作为语音系统和底层音频系统的桥梁，VoicePlayer。它对调用方隐藏了语音系统运行的细节。对于调用方来说，他们只需要像使用 PostEvent 一样，调用 PlayVoice(VoiceID) 即可方便地播放语音。VoicePlayer 负责接收 VoiceID，并解析其中的属性，如果是 External Source 则应用本地化相关的决策。然后调用 External Source 或普通事件的接口进行播放。最后应用一些 RTPC 等参数。

在 VoicePlayer 之上，我们又构建了几个更贴近游戏业务的模块。在这些模块里，每一句语音都由一个 VoiceContext 的对象代表，这些模块持有并管理 VoiceContext 对象。该对象记录了一系列和一句语音紧密相关的核心数据：

- 这句语音的 PlayingID
- 这句语音的说话人是谁？
- 这句语音的优先级有多高？
- 这句语音能不能被其他语音打断？
- ……

这些模块主要用于协调多个外部系统的语音请求。比如，战斗系统想要播放战斗的呐喊，探索系统想要播放一个附近有宝箱的提示语音。在这种情况下，战斗的语音比探索的语音更加重要，我们不能简单地以停止最早播放的语音这样的手段来进行管理。语音系统就在这个过程中，通过人为配置的一系列优先级、打断逻辑、嘈杂度、冷却时间等参数来保证语音播放的合理性。

语音子系统将语音播放、规则管理与业务逻辑分离。游戏系统只需声明意图（比如：播放一句战斗呐喊），而所有关于优先级、冲突解决和资源调度的复杂决策，都在子系统内部闭环完成，保证了语音表现既富有情感又井然有序。

## 音频的配置

在过往的做法中，为了将音频部门和其他部门“解耦”，可能会通过一张唯一的表格来对音频事件的配置进行管理。业务部门在需要配置音频时，按照他们的规范，在表格填写上一些 ID，音频设计师在音频制作完成之后，再往表格中填写业务 ID 所对应的音频 ID。音频部门在整个项目过程中，唯一需要离开 Wwise 做的事情就是填写这张表格。他们的接口在播放音频时，先到表格中查询他们所填写的 ID 对应的音频 ID，然后触发音频的播放。

这样的形式在管理上或许非常集中和方便，但会牺牲配置的灵活性，也增加了不必要的中间层。在《终末地》中，我们选择了一种更直接、更贴近业务的方式。

在我们的工作模式中，音频部门会在开发的早期就参与设计和讨论，音频的配置也分散在各个模块，暴露原始的音频 ID，按需进行配置。当然我们也离不开 Excel 表格，只不过我们通过表格配置的音频，按照不同模块有着不同的格式和划分，直接进行配置。比如对于可拾取的物品，表格只有简单的3列：物品ID，掉落时音效，拾取时音效。

Wwise Unity Integration 使用 Unity 的 ScriptableObject 对象来存储每个事件的相关信息。然而对于音频事件来说，最关键的信息其实只有 Event 的 ID 这一项。大量的 ScriptableObject 在如此规模的项目使用起来，也存在比较大的性能瓶颈。

为了解决这个问题，我们编写了一个非常轻量的 Structure。``

```
public struct AudioID
{
    public uint id;
#if UNITY_EDITOR
    public string name;
#endif

    public static implicit operator uint(AudioID audioId) { /* ... */ }
    // ...
} 
```

再配合 PropertyDrawer 脚本，为该类型编写了属性输入框在编辑时的逻辑，做到了以字符串编辑，以整数存储的功能。

在任何需要配置音频的地方，都直接使用该类型，一方面可以直接享受到 PropertyDrawer 带来的便捷，另一方面，在运行时它就是非常简单的一个 uint，不需要再创建或载入额外的对象了。

配合一些 Attribute，我们还可以做到通过 Attribute 来提示该音频事件应该具备哪些属性，在配置时顺带进行一些正确性的检查。比如某个地方应该配置一个 loop 的音频事件还是一个 oneshot 的音频事件。

![09-AudioPropertyDrawer](https://1940263.fs1.hubspotusercontent-na1.net/hubfs/1940263/Arknights%20Endfield/09-AudioPropertyDrawer.png)

AudioID 虽然只是一个轻量的结构体，但它统一了整个项目中音频事件的配置方式。这不仅消除了大量资产对象带来的性能瓶颈，还通过 PropertyDrawer 和 Attribute 机制，将配置的便捷性、正确性校验和运行时效率融为一体。

## 音频系统的 Debug

### 信息反查

前面在分享 AudioLogger 系统的时候，提到了 AudioAdapter 中需要操作游戏对象的方法中，都是通过 ulong ID 来进行的。若 Debug 的时候只能看到 ID 而看不到名字，是非常不方便的。因此，我们使用一个 Container 维护了一系列 Debug Only 的调试信息反查表。比如，在通过 ID Dispatcher 分配 ID 时，同时可选传入该 ID 的调试名称。而 AudioObject 在初始化时，又可以将自身的信息传递给 ID Dispatcher。在打印日志，或绘制 Debug Gizmos 时，就可以从该容器捞出所需的 ID 的文本信息了。

```
public static class AudioDebugInfoContainer
{
    public Dict<ulong, string> s_objectNameLut;
    
    public void SetAudioObjectName(ulong id, string debugName) { /* ... */ }
    public void RemoveAudioObjectName(ulong id) { /* ... */ }
    public string GetAudioObjectName(ulong id) { /* ... */ }
}
// ID Dispatcher
public static ulong GetId(string debugName)
{
    var id = s_currentId++;
#if DEBUG
    AudioDebugInfoContainer.SetAudioObjectName(id, debugName);
#endif
    return id;
}
```

### 可视化的调试系统

作为程序员，我们看习惯了密密麻麻的日志。而对于音频设计师来说，给他们一堆日志他们可能会觉得头皮发麻。因此，提供可视化的 Debug 工具也十分重要。

可视化并不是要使用花里胡哨的手段来展示各种信息，更重要的是针对数据的特点，以合适的方法，用图形将数据呈现出来。Gizmos 作为最常见且最重要的手段，自然不必多说。

针对不同的抽象数据或流程，我们可以使用各种不同的手段来呈现。比如《终末地》的语音系统，借鉴了 Wwise 用于呈现混音管线的 Voice Graph，以节点图的形式，向设计师呈现了一句语音从收到上游请求到最终播放的决策链路。通过该工具，语音设计师可以追踪一句语音的完整生命周期。从发起、优先级判断、到如何被从大量语音中选中，经过了哪些条件检查，最终的是否成功播放。每一步决策都清晰可见，使得语音系统的调试效率大大提升。

![10-VoiceProfiler](https://1940263.fs1.hubspotusercontent-na1.net/hubfs/1940263/Arknights%20Endfield/10-VoiceProfiler.png)

### 是时候拥有一个工具箱了

随着调试、测试、配置等各方面需求和工具等增加，每位参与音频工作的同事，有时候都会搞不清楚一个功能应该在哪里找到，如何使用。所以，我们把所有的音频相关功能都收集到一起，形成了一个音频工具箱，通过唯一的入口进入，并按照几个主要的模块进行划分，确保每位团队成员都能快速找到所需工具。

![11-AudioToolBox1](https://1940263.fs1.hubspotusercontent-na1.net/hubfs/1940263/Arknights%20Endfield/11-AudioToolBox1.png)

![11-AudioToolBox2](https://1940263.fs1.hubspotusercontent-na1.net/hubfs/1940263/Arknights%20Endfield/11-AudioToolBox2.png)

## 性能

我们对于性能的关注，贯穿了整个项目的研发周期。

在我们的项目中，性能问题不是只有引擎和客户端程序员要关注的事情。每个人在工作过程中都持续地留意着性能的问题。

对于音频部门来说，可能大部分时候会觉得，音频系统的性能问题是通过 Wwise 中的最大发生数限制、转码格式、合理的效果器链配置等手段来解决的。这方面的优化，Wwise 官方也在文档和 Blog 中提供了大量的指导。需要特别指出的是，Wwise Profiler 中的 CPU 使用率并不是指 Wwise 占整个系统的 CPU 使用率。而是指渲染当前音频帧所花费的时间占整个音频帧时间的比率。当 Wwise Profiler 中显示的 CPU 使用率超过 100% 时，音频必然出现卡顿，但并不意味着整个游戏或整个系统会发生卡顿。

然而，游戏引擎中的音频相关性能问题却常常被忽略。游戏引擎中 CPU 的性能问题可以分为几大类：

- 由逻辑阻塞主线程带来的卡顿。如 GC、同步操作、单帧内的突发逻辑等带来的耗时尖刺。可以通过对象池来减少GC、异步化同步操作来减少主线程等待、分帧执行平摊工作量等手段缓解。
- 由密集运算带来的平均帧耗时增加。大型项目中，往往存在一些复杂玩法，需要各种数学计算来满足音频的效果需求。对于这类情况，可以通过异步多线程的手段来进行优化。
- 其他更底层的问题，如缓存命中、分支预测失败、线程上下文切换的开销等。这部分需要更加底层的手段来进行优化，甚至可能不是我们仅仅通过 C# 层面的代码重构就能解决的。

现代 PC 的 CPU 拥有较强的计算能力，部分密集计算的性能问题可能被硬件能力所掩盖，使得阻塞型问题（如 GC 尖刺）在开发中更容易被感知。而移动端 CPU 的绝对性能较弱，对两类问题都更加敏感——不仅密集计算更容易突破帧耗时预算，GC 等阻塞型问题造成的卡顿也更为明显。作为一款多端运营的游戏，我们必须同时应对这两类挑战。

音频系统中，在构建 Auto-Defined Banks 的全异步资产管理系统时，我们已经使用异步的方式，顺带着解决掉由资产加载带来的阻塞问题了。

对于 GC 和其他尖刺，我们还借助 Unity 的 Profiler，在音频系统中多处埋点，统计每个部分的耗时和 GC 或其他造成单帧卡顿的原因，通过多种方式进行优化。比如我们允许 Wwise Unity Interation 使用 unsafe 的代码，直接通过指针或强制类型转换，来减少 SWIG 生成代码在执行时所带来的 GC。

对于密集计算的场景，则更多发生在 Gameplay 相关的部分中。

![13-Blight](https://1940263.fs1.hubspotusercontent-na1.net/hubfs/1940263/Arknights%20Endfield/13-Blight.png)

比如，侵蚀(Blight)可以以不规则形状占据大片体积，我们希望侵蚀的声音可以听起来十分巨大，且当玩家围绕着侵蚀移动时，可以持续地体验到这种体积感。

我们尝试过 Wwise 的 MultiPosition 方案，但由于侵蚀可以被销毁，可以动态变化，MultiPositon 方案可能会带来一些声音的突变等问题。

最后，我们决定以类似体积声的思路，自己编写一些算法，根据侵蚀的若干个关键位置和玩家之间的关系，计算出一个代表侵蚀的发声体，使玩家在移动过程中始终能感受到侵蚀的巨大体积感。侵蚀这类由多个位置组成的单个声源，我们称为簇声源(Cluster Emitter)。

每一片侵蚀都存在数十个关键位置，而周边的地图中，又可能存在数十片侵蚀区域，计算量相当可观。

类似的挑战还有，比如伊冯干员的角色副本中，存在大量可逐根交互的激光阵列。

激光阵其实是大量的线声源，在这个场景下，如果逐个进行计算，每帧可能需要计算数百根激光的发声体。

![14-Laser](https://1940263.fs1.hubspotusercontent-na1.net/hubfs/1940263/Arknights%20Endfield/14-Laser.png)

面对密集计算场景时，逐对象独立计算的传统Update模式存在两个问题：一是引擎安全检查等带来的逐次调用开销（Overhead）；二是计算无法高效分配至多个CPU核心。

所以，我们使用一个辅助模块，通过 Unity 的 Jobs 系统，通过异步多线程的方式进行了优化。

每个对象初始化时，都会将自己注册至辅助系统。辅助系统收集一系列对象的信息，在每帧开始时通过 Jobs 系统分配至多个核心进行计算。在主线程中完成分配工作后，游戏的其他模块或系统就可以继续工作而不被阻塞。最后，在一帧的末尾，读回计算的结果，并批量同步给每个音频对象，再由音频对象同步给 Wwise。

上述的方法可以覆盖一些比较通用的发声体类型的性能问题。但在性能优化上，我们不追求通过某种大而统一的系统来一次性解决所有问题。而是根据每种情况思考并寻找合适的解决方法。

比如集成工业系统就是一个典型的例外。他本身是相对独立且完整的一套系统，它的音频功能，不是由简单的一类发声源就能解决的，而是涉及了距离筛选、状态判断、空间聚合等多个方面。

为了在减少发声体数量的同时又保证大批量工业建筑的听感，我们使用了我们称为“群感声源”的手段来解决问题。

我们以玩家为中心，在玩家周围划分若干个扇形区域，运行时统计区域内的活跃设建筑数量，并将它们的位置根据距离进行加权平均，最后得到一个代表该“扇区”的发声点。这个发声点播放着一个 loop 的声音实例，并且根据机器数量等数据，通过 RTPC 动态调整音量、样本 blend 等参数。将“群感声源”和近处建筑自身的声音进行叠加，最终就得到了集成工业系统的声景。

![15-WulingAIC](https://1940263.fs1.hubspotusercontent-na1.net/hubfs/1940263/Arknights%20Endfield/15-WulingAIC.png)

对此，我们使用流水线(Pipeline)的思维，将集成工业系统的音频计算分为多个 Pass 进行处理。

第一个 Pass 批量计算每个建筑到玩家的距离，第二个 Pass 根据距离和机器的工作状态做出 Culling 的决策，第三个 Pass 依赖之前的某些计算结果进一步进行聚合，将与玩家距离较远但各自之间距离较近的机器合并为“群感声源”，在减少发声源数量的同时，还可以使用更有氛围感的声音来营造集成工业系统大量机器工作的声景。然后还有用于计算水管等不规则折线发声体的 Pass。

通过这种多Pass流水线架构，集成工业系统中成百上千个建筑的音频计算被合理计划和分配，最终将耗时控制在可接受的范围内。

通过各种优化手段，我们使得中低端手机的玩家，也能拥有出色的音频体验。

## 最后

回顾《终末地》的音频系统建设，我们的核心思想可以凝练为：

- 组合优于继承
- 分层系统
- 行为与数据分离

基于这些思想，我们的实践是：

- 基础设施层，统一了接口与生命周期，是系统的稳定基石。
- 游戏交互层，提供了游戏世界和音频系统交互的入口。
- 音频子系统层，通过构建出高内聚的模块，来使得业务侧可以轻松应对复杂的音频需求。

这套架构贯穿了《终末地》项目的音频系统，通过清晰的边界和接口，让我们和项目组可以低成本、高效率进行合作，更确保了在面对不断增长的内容复杂度和严苛的多端性能要求时，音频系统始终保持稳定、可控且易于扩展。

没有一套架构是完美的、不变的，我们的音频系统在整个开发周期中其实也经历过若干次较大的重构，最终在项目上线前达到了目前相对稳定的状态。但我相信，在项目早期投入精力根据项目类型、项目实际需求去思考和搭建合适的架构，并在开发过程中敢于在发现错误的设计时进行重构，是应对大型游戏的系统搭建的良好策略。

希望我们在《终末地》中的这些实践和思考，能为大家带来有价值的参考！

