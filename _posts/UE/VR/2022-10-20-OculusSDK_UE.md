---
layout: post
title: "UE 接OculusSDK"
date: 2022-10-20 09:00:00 +0800 
categories: UE
tag: VR
---
* content
{:toc #markdown-toc}

> 参考：[https://developer.oculus.com/documentation/unreal/ps-get-started/](https://developer.oculus.com/documentation/unreal/ps-get-started/)

这是一篇关于UE如何使用Oculus提供的平台SDK来做一些平台上的功能，如：获取Oculus用户ID，排行榜，好友邀请等。

<!-- more -->

## 创建应用程序

需要通过Oculus平台，才能使用平台SDK，他会生成一个App ID，如果不知道应用程序的App ID，则它无法初始化

[^1]: 类似Online Subsystem Steam的[Steam App ID](https://docs.unrealengine.com/5.3/en-US/online-subsystem-steam-interface-in-unreal-engine/#steamappid)



>**App ID**
>Used to initialize the Platform SDK
>
>**App Secret**
>This secret token is used to make requests to Oculus APIs on behalf of your app rather than a user.


登录到[开发者界面](https://developer.oculus.com/manage/)点击右上角绿色Create New App

![](D:\Githubio\windcrazy123.github.io\styles\images\VR\CreateNewApp.png)

然后输入你应用的名字，然后选择平台我是Quest就选了`Meta Quest（App Lab）`，最后点击`创建`，刷新页面（此示例名称是UESDKTest）；之后点击进入UESDKTest,在左侧边栏点击API就可以看到你的App ID了

![](D:\Githubio\windcrazy123.github.io\styles\images\VR\UESDKTestAPI.png)

## 配置开发环境

Meta Quest 平台功能直接[集成](https://developer.oculus.com/documentation/unreal/unreal-engine/#integration-and-tools)到虚幻编辑器中。

- 有一个Oculus特供的虚幻引擎，在[这里](https://github.com/Oculus-VR/UnrealEngine)获得，里面包含了Platform SDK

  [^2]: 分支中有[额外实现](https://developer.oculus.com/documentation/unreal/unreal-compatibility-matrix/#features)


> Tag里的名称是按照如下命名 的：“oculus-(UnrealEngineVersion)-(OVRPluginVersion)-(OculusSdkVersion)”, 例如： “oculus-5.1.1-release-1.82.0-v50.4”.

- 或者使用插件 (OVRPlugin，有好几个名字知道是platformSDK就行了) 提供对虚幻引擎的集成支持，在[这里](https://developer.oculus.com/downloads/unreal-engine/)获得

>虽然两种办法都可以，但是Oculus官方说：Professional developers who wish to take full advantage of the features available with the Oculus Unreal integration should download and build the Unreal Engine source code. 

打开项目在编辑器中选择**Edit** > **Plugins** > **Virtual Reality** >**Meta XR**(Oculus VR),然后Enabled

选择**Edit** > **Plugins** > **Online Platform** >**Online Subsystem Oculus**

调整 DefaultEngine.ini 文件以使用 `[ProjectDirectory]/Config/DefaultEngine.ini`

```ini
[ OnlineSubsystemOculus ] 
bEnabled = true
OculusAppId =[ app_id_here ]
RiftAppId =[ rift 应用程序 ID ]
```

`OculusAppId`是开发人员仪表板上的[API](https://developer.oculus.com/manage/app/api/)页面检索到的唯一应用程序 ID 。就是咱第一步做的事情（不要中括号比如：OculusAppId =1111111）

`RiftAppId`is the App ID if creating a Quest Link App. This ID is different from the OculusAppId.

>注意：如果启用Platform Plugin，那么应用程序将需要访问User ID 和User Profile，并且需要[Data Use Checkup](https://developer.oculus.com/resources/publish-data-use/) 流程作为你向 Meta Quest 商店提交的一部分。

## 配置您的应用程序以进行本地开发

此步骤将允许开发人员在开发过程中运行本地构建的应用程序。具体参考[权力检查](https://developer.oculus.com/documentation/unreal/ps-entitlement-check/)，[创建组织并管理用户](https://developer.oculus.com/resources/publish-account-management-intro/)

### 权力检查

 在start message pump后立马添加**Entitlement Get Is Viewer Entitled** 节点 .

![](D:\Githubio\windcrazy123.github.io\styles\images\VR\EntitlementCheck.png)





















[**生成keystore是报错拒绝访问(已测试)**](https://blog.51cto.com/rickyh/1889153)

1. 請點選左下角的「開始」。
2. 請在下方的搜尋欄位輸入「CMD」。
3. 對搜尋結果的CMD應用程式點選滑鼠右鍵。 (如圖1.)
4. 再點選「以系統管理員身分執行(A)」

Warning:
JKS 密钥库使用专用格式。建议使用 "keytool -importkeystore -srckeystore ExampleKey.keystore -destkeystore ExampleKey.keystore -deststoretype pkcs12" 迁移到行业标准格式 PKCS12。

C:\Program Files\Android\jdk\jdk-8.0.302.8-hotspot\jdk8u302-b08\jre\bin>

https://www.youtube.com/watch?v=CGdMfIB_00o&list=PLzIwronG0sE4Ooku7Nvlpd4locUZAHzjb
