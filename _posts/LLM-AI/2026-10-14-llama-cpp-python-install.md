---
layout: post
title: "安装llama_cpp_python库"
date: 2026-10-14 09:00:00 +0800 
categories: StableDiffusion
tag: AI
---
* content
{:toc #markdown-toc}

这是一篇安装llama_cpp_python库的文章

<!-- more -->

# 快速安装

我使用了ComfyUI的便携版因此有`python_embeded\python.exe`，在根目录打开PowerShell输入

```powershell
.\python_embeded\python.exe -m pip install llama-cpp-python
```

按理说，应该没问题，但是我报错了

```
Collecting llama-cpp-python>=0.2.0 (from -r ComfyUI/custom_nodes/ComfyUI-JoyCaption/requirements_gguf.txt (line 1))
Using cached llama_cpp_python-0.3.36.tar.gz (76.6 MB)
ERROR: Could not install packages due to an OSError: [Errno 2] No such file or directory: 'C:\Users\lbd\AppData\Local\Temp\pip-install-_8i1vopc\llama-cpp-python_16d260b3cb3f489ca8142afe2357be5a\vendor\llama.cpp\tools\ui\src\lib\components\app\chat\ChatAttachments\ChatAttachmentsList\ChatAttachmentsListItem\ChatAttachmentsListItemThumbnailFile.svelte'
```

原因可能是：在线下载时解压路径过长，Windows 260 字符路径限制导致解压截断、文件丢失

# 手动安装

## 直接手动下载 wheel 离线安装

1. 打开 https://github.com/abetlen/llama-cpp-python/releases 找到对应 python 版本 + cuda 版本的 `.whl` 文件，比如： `llama_cpp_python-0.3.36-py3-none-win_amd64.whl`（cu130） 下载放到 ComfyUI 根目录
2. Powershell 执行（把文件名替换成你下载的 whl）

```powershell
.\python_embeded\python.exe -m pip install .\llama_cpp_python-0.3.36-py3-none-win_amd64.whl
```

离线安装完全不走源码编译，彻底避开长路径问题。

## 装好 llama-cpp 之后，继续装剩下依赖

```
.\python_embeded\python.exe -m pip install -r ComfyUI/custom_nodes/ComfyUI-JoyCaption/requirements_gguf.txt --no-deps
```

`--no-deps` 跳过 llama-cpp-python，只安装其余依赖包

## 检查

重新再根目录打开PowerShell输入：

```powershell
.\python_embeded\python.exe -m pip show llama-cpp-python
.\python_embeded\python.exe -m pip check llama-cpp-python
```

应该出现如下：

```powershell
PS F:\ComfyUI_Portable> .\python_embeded\python.exe -m pip show llama-cpp-python
Name: llama_cpp_python
Version: 0.3.36
Summary: Python bindings for the llama.cpp library
Home-page: https://github.com/abetlen/llama-cpp-python
Author:
Author-email: Andrei Betlen <abetlen@gmail.com>
License: MIT
Location: F:\ComfyUI_Portable\python_embeded\Lib\site-packages
Requires: diskcache, jinja2, numpy, typing-extensions
Required-by:
PS F:\ComfyUI_Portable> .\python_embeded\python.exe -m pip check llama-cpp-python
No broken requirements found.
```

这时再次打开ComfyUI时，cmd的环境检查就不再报错说明llama_cpp_python安装成功并正确识别
