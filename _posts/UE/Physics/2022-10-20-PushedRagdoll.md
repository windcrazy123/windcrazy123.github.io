---
layout: post
title: "NPC被推动"
date: 2022-10-20 09:00:00 +0800 
categories: UE
tag: Practice
---
* content
{:toc #markdown-toc}

这是一篇关于推动NPC，并且NPC被推动时物理与动画混合的实践，这样就可以在各个方向推动NPC，并且NPC动画有很好的适配，不必单独做多方向的被推动的动画

<!-- more -->

# 被推动八方向Ragdoll

总览：

进入物理模拟：获取HitResult中Trace Direction and Bone Name（因为我们无法从HitResult中获取Impulse）,使用Trace Direction和手工赋值的冲量的大小构建一个模拟的冲量。将此冲量作用于获取到的Bone Name对应的骨骼节点。

物理动画的混合：为了呈现较好的视觉效果，动画中的Speed需要手动设置去适配合适的动画。Direction则需要根据Trace Direction进行归一化。之后再每帧减少Speed和物理模拟的时间，并根据此时间的大小进行物理混合权重。

退出物理模拟：剩余时间结束，停止物理模拟

# RagdollNPC


<img src="{{ '/styles/images/Physics/Practice/RagdollBegin.png' | prepend: site.baseurl }}" />

初始化PhysicalAnimation Profile，设置PhysicalAnimation相关以使物理动画混合过渡自然

其中ragdoll2设置如下

<img src="{{ '/styles/images/Physics/Practice/profile1.png' | prepend: site.baseurl }}" />

<img src="{{ '/styles/images/Physics/Practice/profile2.png' | prepend: site.baseurl }}" />


NPC:HitAction事件

<img src="{{ '/styles/images/Physics/Practice/hitaction1.png' | prepend: site.baseurl }}" />

进入被动触发事件，设置物理过渡到动画的过渡剩余时间，进入布娃娃状态，并对hit到的骨骼施加冲量（冲量根据trace direction设置大小）

<img src="{{ '/styles/images/Physics/Practice/hitaction2.png' | prepend: site.baseurl }}" />

手动设置Speed；

Direction是来调整触发物理模拟后的动画相应

<img src="{{ '/styles/images/Physics/Practice/tick1.png' | prepend: site.baseurl }}" />

tick不断减少time remaining（混合过渡剩余时间）和Speed（播放动画的姿势，跟随剩余时间变化）并设置运动速度（胶囊体速度，跟随Speed变化）

<img src="{{ '/styles/images/Physics/Practice/tick2.png' | prepend: site.baseurl }}" />

利用time remaining混合物理动画，结束后退出

# MaterCharacter

<img src="{{ '/styles/images/Physics/Practice/CharacterSphereTrace.png' | prepend: site.baseurl }}" />

射线检测触发NPC进入布娃娃并将hit结果作为参数传输

# 动画蓝图

<img src="{{ '/styles/images/Physics/Practice/RagdollAnimationEvent1.png' | prepend: site.baseurl }}" />

获取Speed和Direction

<img src="{{ '/styles/images/Physics/Practice/RagdollAnimationEvent2.png' | prepend: site.baseurl }}" />

将两个值给到动画序列

BS_AllDirection

<img src="{{ '/styles/images/Physics/Practice/BS_AllDirection.png' | prepend: site.baseurl }}" />
