---
layout: post
title: "NavMesh关卡流送"
date: 2026-03-19 09:00:00 +0800 
categories: UE
tag: AI
---
* content
{:toc #markdown-toc}

这是一篇关于UE Navmesh在关卡流送中的一些知识

<!-- more -->

# 0、前言

在开放大世界中，会使用streaming levels来将整个世界切分为许多小的地图块，在运行时动态的加载需要的地图块，并且及时释放不需要的地图块。通过这种方式，可以确保只有需要的地图块存在于内存中并且会被渲染，能够实现超大的、无缝的游戏世界。

但是在我们使用UE本身提供的NavMesh寻路解决方案时，在这种超大世界中构建NavMesh会导致相当巨大的数据量，导致其无法在内存十分有限的环境中使用。网上也有许多的解决办法，这里就只总结一些较为简单的方法。主要是针对在关卡流送时加载AI角色，却发现他们站在原地。

> 还有更为复杂的Navmesh动态更新方案去参考[UE4中Navmesh动态更新方案及优化](https://zhuanlan.zhihu.com/p/566846141)，此文章主要介绍静态物体的更新，不适用动态生成的障碍物，原理：主要通过修改Poly中的AreaID实现其对Navmesh的影响，使用：重写FRecastQueryFilter一般要重写对应的UNavigationQueryFilter，寻路函数是可以传入UNavigationQueryFilter子类，另外也可以在AIController初始化的时候对DefaultNavigationFilterClass进行赋值，设置默认的过滤类。ue对navmesh的接入主要还是参考recastnavigation自己的obstacle那个sample
>

# 1、Streaming Level中Static的NavMesh

如果想要在制作过程较为流畅建议在Editor Preferences → Level Editor → Miscellaneous中，将Update Navigation Automatically设置为false，这样可以避免在修改NavMeshBoundsVolume时自动生成NavMesh。我们会使用Build菜单中的Build Paths来生成NavMesh，会快许多，然后在Project Settings → Engine → Navigation System中，将Auto Create Navigation Data设置为false，这样可以避免在streaming levels中放置NavMeshBoundsVolume时自动创建出RecastNavMesh（主要防止你不知道他生成在哪了，5.4是只在主关卡生成）

弄好配置之后先确保RecastNavMesh中Runtime Generation设置为Static，就可以为每个streaming level配置好NavMeshBoundsVolume，如果因为配置或其他原因出现了RecastNavMesh，可自行删除，最后我们会在主关卡生成这个，且只有一个。

然后在persistent level中，我们还需要放置一个NavMeshBoundsVolume，但是该Volume不需要包围任何几何对象，它仅仅用于让UE能够在运行时正确载入streaming levels的NavMesh数据

最后将需要烘培的子关卡加载，然后点击Build菜单中的Build Paths，此时开始生成NavMesh，同时会在场景中创建RecastNavMesh

这时你在进行动态加载，比如`Load Stream Level`，AI就又可以正常活动了。

# 2、Dynamic Navigation

在Project Setting中首先将Runtime Generation设置为Dynamic，这个可以支持子关卡的加载和卸载，然后在`Navigation System`的`Navigation Enforcing`中勾选`Generate Navigation Only Around Navigation Invokers`这个不会让电脑加载所有的NavMesh。

然后在主关卡中放入NavMesh包裹整个世界包括子关卡。

在AI角色中添加`Navigation Invoker`组件,它会为你的AI角色设定一个半径，导航系统会围绕这个半径实时构建。这样，就不会让你的电脑过载，因为不会构建整个游戏世界的导航系统，而是只会构建AI角色所需的导航。随着角色行走，会在接近区域附近生成新的导航系统，而它离开的导航系统会被移除。然后你可以设置Tile Generation Radius和Tile Removal Radius

最后将AI角色上的胶囊体组件关闭`Can Ever Affect Navigation`，否则AI会出现奇怪的效果。

# 3、其他

之前提到在streaming levels中放置NavMeshBoundsVolume时自动创建出的RecastNavMesh在5.4时是位于主关卡的，如果说你单独打开地图，自动创建后让他在子关卡生成，你会发现AI还是不动，这个时候你需要将在`Project Setting`的`Navigation System`的`Should Discard Sub Level Nav Data`关闭，这个时候再加载AI就可以动了，当然，这个Nav数据只有子关卡的，没有主关卡的

# 4、参考

[[Tutorial\] How to Stream AI and Navmeshes in Level Streaming[教程]如何在关卡流式中流式传输AI和导航网格              ](https://forums.unrealengine.com/t/tutorial-how-to-stream-ai-and-navmeshes-in-level-streaming/235640)
[在UE4的streaming levels中使用静态NavMesh](https://zhuanlan.zhihu.com/p/352934629)
