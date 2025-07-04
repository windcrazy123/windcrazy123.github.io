---
layout: post
title: "ChaosDestructionCaching"
date: 2025-03-20 09:00:00 +0800 
categories: UE
tag: Physics
---
* content
{:toc #markdown-toc}

这是一篇关于ChaosDestruction的Caching学习时的笔记

<!-- more -->

# 0、官方快速学习

需要开启ChaosEditor和ChaosCaching插件

> 参考文档：[Chaos破坏系统](https://dev.epicgames.com/documentation/zh-cn/unreal-engine/chaos-destruction-in-unreal-engine)

# 一、破碎几何体集合

## 1、创建GC

> 首先应该注意：
>
> 1. 几何体集合应该"不漏水"
>    用于创建几何体集合的Actor应该"不漏水"，即没有开放的面或边。有开放面的对象在模拟时的性能更差，会返回不可预测的结果。(如下图，可以看到不仅有重叠UV而且会产生这种奇怪的面片，会严重影响破裂时的效果)
>
>    ![](..\..\..\..\styles\images\Physics\Chaos\WaterTight.png)
>
> 2. 几何体集合不应该有相交几何体
>
>    构成几何体集合的对象不应该彼此相交。由于每个几何体集合是可以模拟的单独对象，因此在模拟开始后，Chaos解算器会试图将每个对象彼此推离。这可能导致不规则、不可预测的结果。
>
> 3. 创建几何体集合应结合实际场景
> 
> 	 创建几何体集合时应考虑你要模拟的物体，比如：模拟石头破裂就是将此物体进行破裂内部UE会进行填充；如果模拟楼房，内部应该是大部分空隙，所以在进行破裂时应该先进行割裂，然后将内部填充删除。
>
> 参考：[创建几何体集合时的最佳实践](https://dev.epicgames.com/documentation/zh-cn/unreal-engine/geometry-collections-user-guide#创建几何体集合时的最佳实践)

选择你要进行破碎的物体，在LevelEditor中的`EditorModes`选择`Fracture Mode`，会弹出四个窗口，如果没有弹出可在MainMenu的`Window->Editor Modes`的子窗口找到并打开

![](..\..\..\..\styles\images\Physics\Chaos\ChaosWindows.png)

![](..\..\..\..\styles\images\Physics\Chaos\OpenChaosWindow.png)

首先需要创建几何体集合(Geometry Collision)，在`Mode Toolbar`中点击`New`并选择保存位置(建议文件夹名字命名为`GC`，文件使用`GC`作为前缀)，之后引擎会将你之前选择的物体替换为你创建的几何体集合，之后的操作都将对此几何体集合操作。

> 注意：可选择多个物体同时进行破裂但不推荐，多选创建出来的几何体集合仍然是一个并且自带一级子树可能对性能有影响
>
> ![](..\..\..\..\styles\images\Physics\Chaos\MultiFracture.png)
>
> 不过你可以利用蓝图Actor来创建，例子参考：[官方教程](https://dev.epicgames.com/documentation/zh-cn/unreal-engine/geometry-collections-user-guide#%E5%88%A9%E7%94%A8%E8%93%9D%E5%9B%BEactor%E5%88%9B%E5%BB%BA%E5%87%A0%E4%BD%95%E4%BD%93%E9%9B%86%E5%90%88)

## 2、破碎

> 详情参考：[破裂几何体集合](https://dev.epicgames.com/documentation/zh-cn/unreal-engine/fracturing-geometry-collections-user-guide)

在`Mode Toolbar`窗口的`Fracture`选择破裂模式后，在`Fracture`窗口进行艺术加工。

在`Noise`，`Common Fracture`等填好感觉差不多的时候，如果对分裂的空间划分不满意，除了修改`Random Seed`外还可以在`Random Seed`值为`-1`时单击某个骨骼进行快速随机。

如果对父级效果不满意可以在`Mode Toolbar`中的`Generate`点击`Reset`后在`Fracture`窗口中点击`Reset`重新进行破碎操作，如果对上一次操作不满意也可以撤回

## 3、编辑骨骼

> 详情参考：[编辑工具用户指南](https://dev.epicgames.com/documentation/zh-cn/unreal-engine/edit-tools-user-guide-in-unreal-engine)

在`Fracture Hierarchy`中选择骨骼后，在`ModeToolbar`中的`Edit`进行编辑，`Prune`可删除所选骨骼，`GeoMrg`可将选择的骨骼进行合并(*如果选择不同子树的的骨骼合并后将并与第一个选择的骨骼的父子树*)

## 4、材质

有时我们可能需要自定义破碎后内部的材质，我们可以在`Mode Toolbar`中的`Utilities`点击`Material`，之后在`Fracture`窗口的`Asset Materials`中点击`Add Material Slot`按钮，之后在`Assign Materials`如图设置然后点击`Assign Materials`蓝色按钮

![](..\..\..\..\styles\images\Physics\Chaos\AddMaterialSlot.png)

然后在破碎的时候在`Fracture`窗口的`Materials`的`Internal Material`选择`[1]...`然后再点击`Fracture`蓝色按钮，如下图，我们可以发现原材质和新增材质应用于内部材质的区别

![](..\..\..\..\styles\images\Physics\Chaos\DifferentInternalMaterial.png)

> 注：你操作时可能不是材质颜色可能类似下图，那么我们在关卡中选中几何体集合在`Details`面板中搜索`Show Bone Colors`并将其改为`False`
>
> ![](..\..\..\..\styles\images\Physics\Chaos\ShowBoneColors.png)

# 二、Chaos场

> 参考文档：[Chaos场用户指南](https://dev.epicgames.com/documentation/zh-cn/unreal-engine/chaos-fields-user-guide-in-unreal-engine)

## 1、FS_AnchorField

锚点场在模拟开始时将落在其体积内的任何几何体集合骨骼设置为 **静态（Static）**，除非在模拟时有另外的力场时其变为动态。

选择几何体集合并前往 **细节（Details）** 面板。向下滚动到 **Chaos物理系统** 分段，然后展开 **初始化场（Initialization Fields）** 数组(如果没有点击右侧加号添加数组)。单击 **索引[0]（Index [0]）** 下拉框并从列表中选择 `FS_AnchorField_Generic`(*你添加到Level中的锚点场，如果你在Level的大纲视图中重命名了就不是这个名字*)。

## 2、FS_SleepDisable

**睡眠（Sleep）** 和 **禁用（Disable）** 场使刚体在速度低于给定 **阈值（Threshold）** 值时静止。更高的阈值将使对象更激进地停止。

这些场用于提高模拟性能，并防止几何体集合的各个碎块在相互碰撞时出现过度抖动。这些场还可用于更好地控制模拟中包含的刚体数量。

> 具体参考[官方文档](https://dev.epicgames.com/documentation/zh-cn/unreal-engine/chaos-fields-user-guide-in-unreal-engine#%E7%9D%A1%E7%9C%A0%E5%92%8C%E7%A6%81%E7%94%A8%E5%9C%BA)

## 3、FS_MasterField

该场附带许多自定义选项，主要用于：

- 通过施加断开骨骼之间连接的外部张力来激活刚体。
- 为刚体应用速度，使其以逼真的方式移动和旋转。

> 具体参考[官方文档](https://dev.epicgames.com/documentation/zh-cn/unreal-engine/chaos-fields-user-guide-in-unreal-engine#%E4%B8%BB%E5%9C%BA)

# 三、录制Cache

选择关卡中的将播放的所有几何体集合，点击顶部菜单栏中的Actor选择`CreateCacheManager`，在关卡多出来的ChaosCacheManager中的CacheMode中选择`RECORD`进行录制

![](..\..\..\..\styles\images\Physics\Chaos\CreateCacheManager.png)

录制好后的Cache可以直接拖到场景里进行播放

# Tips

关于Cache的模拟查看可以直接放在关卡序列中：将场景的`CacheActor`拖到序列中点击加号选择`Chaos Cache`

![](..\..\..\..\styles\images\Physics\Chaos\ViewCache.png)
