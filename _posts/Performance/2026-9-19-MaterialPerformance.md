---
layout: post
title: "材质优化"
date: 2026-03-19 09:00:00 +0800 
categories: Performance
tag: Material
---
* content
{:toc #markdown-toc}

这是一篇关于UE 材质优化的一些知识

<!-- more -->

# Use Alpha to Coverage

> 参考：[为什么延迟渲染和MSAA不搭](https://www.blurredcode.com/2022/02/5b548f07/)
>
> [MSAA 与 Alpha to Coverage：用覆盖率实现“半透明”物体渲染](https://zhuanlan.zhihu.com/p/1945853602099857173)

一种用在前向渲染且开启MSAA下优化Mask材质边缘锯齿的方法，在非半透明材质中近似达到半透明边缘效果，是一种视觉与性能上的优化方式
