---
layout: post
title: "ClaudeCode"
date: 2026-7-01 09:00:00 +0800 
categories: LLM
tag: AI
---
* content
{:toc #markdown-toc}

这是一篇ClaudeCode使用日志

<!-- more -->

# CLI安装

## Step 1: Install Claude Code

> 参考链接：https://setup-code.com/index#page-quickstart

**Windows PowerShell:**

```powershell
irm https://setup-code.com/install.ps1 | iex
```

可能会报错：

报错信息：Failed to fetch version from https://downloads.claude.ai/claude-code-releases/latest after 3 attempt(s): ECONNREFUSED
![](..\..\styles\images\LLM\ClaudeCodeCLIInstallError.png)

VPN查看你的端口号，我的是7890因此下方用的是7890

```powershell
$env:HTTP_PROXY="http://127.0.0.1:7890";$env:HTTPS_PROXY="http://127.0.0.1:7890"
```

> 注意：`HTTPS_PROXY`也是`http:`不要改为`https:`，因为我已经试过了

然后再次**Windows PowerShell:**

```powershell
irm https://setup-code.com/install.ps1 | iex
```
![](..\..\styles\images\LLM\ClaudeCodeCLIInstallComplete.png)

## Step 2:配置环境

将`C:\Users\username\.local\bin`放入PATH环境变量中

## Step 3:CC Switch

这个是方便切换模型供应商和环境配置的工具可以更方便的使用CLI

> 下载地址：https://github.com/farion1231/cc-switch/releases

添加供应商，填入你的APIKey，选择模型后确认即可，`CC Switch`设置中有一个跳过`Claude Code`初次安装确认设为true

> 具体操作可参考：https://docs.packyapi.com/docs/ccswitch/2-claude.html
>
> 或：https://jishuzhan.net/article/2072246930104741889

## Step 4:使用

Power Shell输入claude
