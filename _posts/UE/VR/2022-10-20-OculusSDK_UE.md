---
layout: post
title: "UE 接Oculus平台SDK"
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

<img src="{{ '/styles/images/VR/CreateNewApp.png' | prepend: site.baseurl }}" />

然后输入你应用的名字，然后选择平台我是Quest就选了`Meta Quest（App Lab）`，最后点击`创建`，刷新页面（此示例名称是UESDKTest）；之后点击进入UESDKTest,在左侧边栏点击API就可以看到你的App ID了

<img src="{{ '/styles/images/VR/UESDKTestAPI.png' | prepend: site.baseurl }}" />

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

<img src="{{ '/styles/images/VR/EntitlementCheck.png' | prepend: site.baseurl }}" />



## 测试

在测试 Platform SDK 功能之前，必须将应用程序的发布版本上传到开发人员中心。您只需执行一次此操作，然后可以在本地进行测试并进行更改，而无需重新上传。请参阅[上传 Meta Quest 的应用程序](https://developer.oculus.com/resources/publish-uploading-mobile/)。

我使用UE编辑器上传

<img src="{{ '/styles/images/VR/UEMetaXRPlatformWindow.png' | prepend: site.baseurl }}" />








## 移动应用程序签名

- 签署应用程序。请参阅[移动应用程序签名](https://docs.unrealengine.com/5.2/en-US/signing-android-projects-for-release-on-the-google-play-store-with-unreal-engine/)Unreal 小节。
- **在编辑器中Edit** > **Project Settings** > **Platforms** > **Android**修改以下设置
  - **Android Package Name**- 这应该与开发人员中心设置匹配
  - **Store Version**- 每次上传 apk 都需要一个唯一的商店版本号，该版本号可以是手动递增的数字。
  - **Install Location**- Auto

> 注：如果报错java.io.filenotfoundexception:android.keystore（拒绝访问）这是因为权限问题：你的jdk目录在c盘，当前用户无写入权限。
>
> 方法一更改保存目录：就是讲jdk从c盘挪到其它盘。
>
> 方法二更改权限：以管理员身份运行CMD。（我用这个）
>  1. 键盘按下win键或点击桌面左下角【开始】，输入cmd
>
>  2. 对搜索结果的cmd程序点击【以管理员身份运行】
>
>  3. 重新按照[Unreal小节](https://docs.unrealengine.com/5.2/en-US/signing-android-projects-for-release-on-the-google-play-store-with-unreal-engine/)签署应用程序
>
> 我这里出现了一个警告Warning:JKS 密钥库使用专用格式。建议使用 "keytool -importkeystore -srckeystore ExampleKey.keystore -destkeystore ExampleKey.keystore -deststoretype pkcs12" 迁移到行业标准格式 PKCS12。
>
> 另外，我的keytool.exe在C:\Program Files\Android\jdk\jdk-8.0.302.8-hotspot\jdk8u302-b08\jre\bin>

## Oculus在虚幻中的实例

在[这里](https://developer.oculus.com/documentation/unreal/unreal-samples/)



## 注意

如果使用MQDH(Meta Quest Developer Hub)进行上传，可能会遇到Actions列下一直在转圈加载或Upload为灰色不可点击，这是由于你的MQDH没有[下载cli](https://developer.oculus.com/resources/publish-reference-platform-command-line-utility/)，你需要把下载的exe文件放在 `~/AppData/Roaming/odh/ovr-platform-util.exe`，重启后就可以上传了

<img src="{{ '/styles/images/VR/MQDHUpload.png' | prepend: site.baseurl }}" />



## 参考

Oculus视频-[AR & VR at Meta Open Source](https://www.youtube.com/watch?v=CGdMfIB_00o&list=PLzIwronG0sE4Ooku7Nvlpd4locUZAHzjb)

MQDH下载：https://developer.oculus.com/documentation/unreal/ts-odh/

