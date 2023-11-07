---
layout: post
title: "WidgetComponent"
date: 2021-04-18 09:00:00 +0800 
categories: 未整理
tag: 未整理
---
* content
{:toc #markdown-toc}

这是随手记得，很杂，在做的过程中一直改，还未整理，仅供参考

<!-- more -->

继承自MeshComponent，自然有材质只是需要预定的参数接口实现，只有SlateUI参数是必需的，其他参数的值可在蓝图中修改

<img src="{{ '/styles/images/NoComp/WidgetComponent/1.png' | prepend: site.baseurl }}" />

<img src="{{ '/styles/images/NoComp/WidgetComponent/2.png' | prepend: site.baseurl }}" />

注意：**WidgetComponent**有个**CanEditChange**函数如果返回false那么改动是无效的，比如：在屏幕空间下（Space==EWidgetSpace::Screen）调节BackgroundColor是无效的

<img src="{{ '/styles/images/NoComp/WidgetComponent/3.png' | prepend: site.baseurl }}" />


## 屏幕空间下如何更改前后顺序

默认情况下，UI的前后顺序根据与摄像机的距离进行判断。如果想修改的话怎么弄呢？

修改Widget组件的Layers的两个参数：分别是SharedLayerName层级名称、LayerZOrder层级

注意：两个都要改，从源码中得知：设置层级时，先在Map中找Name是否存在，如果不存在则添加一个新的，并把层级ZOrder写入。

注意：引擎默认是只能初始化修改，不可动态修改，不过可以把原先Widget删除，再创建一个层级靠前的Widget即可。

<img src="{{ '/styles/images/NoComp/WidgetComponent/4.png' | prepend: site.baseurl }}" />

<img src="{{ '/styles/images/NoComp/WidgetComponent/5.png' | prepend: site.baseurl }}" />

<img src="{{ '/styles/images/NoComp/WidgetComponent/6.png' | prepend: site.baseurl }}" />


## 疑难杂症-VR

在世界空间下：widget interaction只会响应他所hit的第一个widget component，以下为第一个widget component的几种状态：

1、UI正对玩家/widget interaction且widget component的visible为true时：interaction的debug线会显示hit到第一个widget component上，并且与之正常交互，可以看到

1_2、widget component不是自己的组件，UI正对玩家/widget interaction且widget component的visible为true，only owner see为true时：interaction的debug线会显示hit到第一个widget component上，并且无法与之正常交互，无法看到

2、UI正对玩家/widget interaction且widget component的visible为false时：interaction的debug线不会显示hit到第一个包括后面的widget component上，并且无法与之正常交互，无法看到

2_2、widget component不是自己的组件，UI正对玩家/widget interaction且widget component的visible为false，only owner see为true时：interaction的debug线不会显示hit到第一个包括后面的widget component上，并且无法与之正常交互，无法看到

3、UI背对玩家/widget interaction且widget component的visible为true时：interaction的debug线会显示hit到第一个widget component上，并且与之正常交互，无法看到

3_2、widget component不是自己的组件，UI背对玩家/widget interaction且widget component的visible为true，only owner see为true时：interaction的debug线会显示hit到第一个widget component上，并且无法与之正常交互，无法看到

4、UI背对玩家/widget interaction且widget component的visible为false时：interaction的debug线不会显示hit到第一个包括后面的widget component上，并且无法与之正常交互，无法看到

4_2、widget component不是自己的组件，UI背对玩家/widget interaction且widget component的visible为false，only owner see为true时：interaction的debug线不会显示hit到第一个包括后面的widget component上，并且无法与之正常交互，无法看到

## 参考

[曲面UI实现浅析和拓展](https://blog.csdn.net/t1370620378/article/details/122845580)

[WidgetComponent分析](https://zhuanlan.zhihu.com/p/399672590)