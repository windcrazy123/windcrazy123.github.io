---
layout: post
title: "避免ini配置变动重新Cook"
date: 2024-03-20 09:00:00 +0800 
categories: UE
tag: Misc
---
* content
{:toc #markdown-toc}


最近经常打包，发现每次打包资源都全部重新Cook了，这对工作效率影响很大，原因之一就是我们在打包流程中自动修改了`ini`文件来配置版本号等。不经过配置的引擎会在`ini`进行变动的时候清理掉原有Cook过的内容。因此需要通过一系列配置，以保证每次Cook的内容是增量的，来加快工作进度。

<!-- more -->

首先要使增量Cook生效，必须在构建命令中加入`-iterate`，否则在每次Cook前会将Cook产物文件夹整个删除。
然后需要在`DefaultEditor.ini`文件中加入类似如下内容：

```
[CookSettings]
+ConfigSettingBlacklist=Windows.Engine:/Script/IOSRuntimeSettings.IOSRuntimeSettings:VersionInfo
+ConfigSettingBlacklist=*.Engine:/Script/AndroidRuntimeSettings.AndroidRuntimeSettings:VersionDisplayName
+ConfigSettingBlacklist=*.Engine:/Script/AndroidRuntimeSettings.AndroidRuntimeSettings:StoreVersion
```

实际上是通过配置增加`ConfigSettingBlacklist`中的内容项来时引擎在Cook的时候忽略相关的配置字段。其格式为

```
+ConfigSettingBlacklist=BuildMachinePlatform.ConfigFile:SectionName:ValueName
```

其中：

| 占位名                 | C++中的涵义                                    | ini中的涵义                                                  | 参考取值                                            |
| :--------------------- | :--------------------------------------------- | :----------------------------------------------------------- | :-------------------------------------------------- |
| `BuildMachinePlatform` | N/A                                            | 构建机器的平台                                               | 取值可以为`Windows/Mac/Linux`等,支持Wildcard(`*`)。 |
| `ConfigFile`           | C++类中`UCLASS(config=Engine)`配置的内容       | `ini`文件名的后半段，即`Base、Default`等字样后面的部分如`DefaultEditor`就是`Editor` | `Engine/Game/Editor`等                              |
| `SectionName`          | C++类所属的路径名(`Path/Of/Package.ClassPath`) | 每个配置文件中方括号内的内容                                 | [`/Script/EngineSettings.GameMapsSettings`]         |
| `ValueName`            | C++类中的变量名                                | 配置文件中具体的配置里行首的名称                             | `EditorStartupMap`，支持Wildcard(`*`)               |

配置成功后，将会在下下次打包时生效。