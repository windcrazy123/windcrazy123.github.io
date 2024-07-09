---
layout: post
title: "由于Gradle版本问题导致的打包失败"
date: 2024-7-4 09:00:00 +0800 
categories: UE
tag: Misc
---
* content
{:toc #markdown-toc}

在新电脑配置Android打包环境后打包可能出现各种各样的错误，本篇将由于Gradle而出现的错误，进行分享。

<!-- more -->

# 遇到的问题

```
UATHelper: 打包 (Android (ASTC)): Creating rungradle.bat to work around commandline length limit (using unused drive letter Y:)
UATHelper: 打包 (Android (ASTC)): Making .apk with Gradle...
UATHelper: 打包 (Android (ASTC)): Downloading https://services.gradle.org/distributions/gradle-7.5-all.zip
UATHelper: 打包 (Android (ASTC)): ..............................................................................................................................................................
UATHelper: 打包 (Android (ASTC)): Unzipping C:\Users\My\.gradle\wrapper\dists\gradle-7.5-all\6qsw290k5lz422uaf8jf6m7co\gradle-7.5-all.zip to C:\Users\My\.gradle\wrapper\dists\gradle-7.5-all\6qsw290k5lz422uaf8jf6m7co
UATHelper: 打包 (Android (ASTC)): Welcome to Gradle 7.5!
UATHelper: 打包 (Android (ASTC)): Here are the highlights of this release:
UATHelper: 打包 (Android (ASTC)):  - Support for Java 18
UATHelper: 打包 (Android (ASTC)):  - Support for building with Groovy 4
UATHelper: 打包 (Android (ASTC)):  - Much more responsive continuous builds
UATHelper: 打包 (Android (ASTC)):  - Improved diagnostics for dependency resolution
UATHelper: 打包 (Android (ASTC)): For more details see https://docs.gradle.org/7.5/release-notes.html
UATHelper: 打包 (Android (ASTC)): To honour the JVM settings for this build a single-use Daemon process will be forked. See https://docs.gradle.org/7.5/userguide/gradle_daemon.html#sec:disabling_the_daemon.
UATHelper: 打包 (Android (ASTC)): Daemon will be stopped at the end of the build 
UATHelper: 打包 (Android (ASTC)): FAILURE: Build failed with an exception.
UATHelper: 打包 (Android (ASTC)): * What went wrong:
UATHelper: 打包 (Android (ASTC)): A problem occurred configuring root project 'app'.
UATHelper: 打包 (Android (ASTC)): > Could not resolve all files for configuration ':classpath'.
UATHelper: 打包 (Android (ASTC)):    > Could not resolve com.android.tools.build:gradle:7.4.2.
UATHelper: 打包 (Android (ASTC)):      Required by:
UATHelper: 打包 (Android (ASTC)):          project :
UATHelper: 打包 (Android (ASTC)):       > No matching variant of com.android.tools.build:gradle:7.4.2 was found. The consumer was configured to find a runtime of a library compatible with Java 8, packaged as a jar, and its dependencies declared externally, as well as attribute 'org.gradle.plugin.api-version' with value '7.5' but:
UATHelper: 打包 (Android (ASTC)):           - Variant 'apiElements' capability com.android.tools.build:gradle:7.4.2 declares a library, packaged as a jar, and its dependencies declared externally:
UATHelper: 打包 (Android (ASTC)):               - Incompatible because this component declares an API of a component compatible with Java 11 and the consumer needed a runtime of a component compatible with Java 8
UATHelper: 打包 (Android (ASTC)):               - Other compatible attribute:
UATHelper: 打包 (Android (ASTC)):                   - Doesn't say anything about org.gradle.plugin.api-version (required '7.5')
UATHelper: 打包 (Android (ASTC)):           - Variant 'javadocElements' capability com.android.tools.build:gradle:7.4.2 declares a runtime of a component, and its dependencies declared externally:
UATHelper: 打包 (Android (ASTC)):               - Incompatible because this component declares documentation and the consumer needed a library
UATHelper: 打包 (Android (ASTC)):               - Other compatible attributes:
UATHelper: 打包 (Android (ASTC)):                   - Doesn't say anything about its target Java version (required compatibility with Java 8)
UATHelper: 打包 (Android (ASTC)):                   - Doesn't say anything about its elements (required them packaged as a jar)
UATHelper: 打包 (Android (ASTC)):                   - Doesn't say anything about org.gradle.plugin.api-version (required '7.5')
UATHelper: 打包 (Android (ASTC)):           - Variant 'runtimeElements' capability com.android.tools.build:gradle:7.4.2 declares a runtime of a library, packaged as a jar, and its dependencies declared externally:
UATHelper: 打包 (Android (ASTC)):               - Incompatible because this component declares a component compatible with Java 11 and the consumer needed a component compatible with Java 8
UATHelper: 打包 (Android (ASTC)):               - Other compatible attribute:
UATHelper: 打包 (Android (ASTC)):                   - Doesn't say anything about org.gradle.plugin.api-version (required '7.5')
UATHelper: 打包 (Android (ASTC)):           - Variant 'sourcesElements' capability com.android.tools.build:gradle:7.4.2 declares a runtime of a component, and its dependencies declared externally:
UATHelper: 打包 (Android (ASTC)):               - Incompatible because this component declares documentation and the consumer needed a library
UATHelper: 打包 (Android (ASTC)):               - Other compatible attributes:
UATHelper: 打包 (Android (ASTC)):                   - Doesn't say anything about its target Java version (required compatibility with Java 8)
UATHelper: 打包 (Android (ASTC)):                   - Doesn't say anything about its elements (required them packaged as a jar)
UATHelper: 打包 (Android (ASTC)):                   - Doesn't say anything about org.gradle.plugin.api-version (required '7.5')
UATHelper: 打包 (Android (ASTC)): * Try:
UATHelper: 打包 (Android (ASTC)): > Run with --stacktrace option to get the stack trace.
UATHelper: 打包 (Android (ASTC)): > Run with --info or --debug option to get more log output.
UATHelper: 打包 (Android (ASTC)): > Run with --scan to get full insights.
UATHelper: 打包 (Android (ASTC)): * Get more help at https://help.gradle.org
UATHelper: 打包 (Android (ASTC)): BUILD FAILED in 11m 50s
```

此时配置：

```c
[/Script/AndroidPlatformEditor.AndroidSDKSettings]
SDKPath=(Path="C:/Users/My/AppData/Local/Android/Sdk")
NDKPath=(Path="C:/Users/My/AppData/Local/Android/Sdk/ndk/25.1.8937393")
JavaPath=(Path="C:/Program Files/Android/Android Studio/jre")
SDKAPILevel=matchndk
NDKAPILevel=latest
```

# 解决办法

在报错中看到了

```
`Incompatible because this component declares an API of a component compatible with Java 11 and the consumer needed a runtime of a component compatible with Java 8`
```

怀疑：**需要Java11**

**解决办法：**

## 一：下载并安装Java11

从[Oracle官方网站](https://www.oracle.com/java/technologies/downloads/#java11-windows)下载Java11并安装。

1. 下载：[jdk-11.0.23_windows-x64_bin.exe](https://www.oracle.com/java/technologies/downloads/#license-lightbox)
2. 安装：打开程序一步一步走就行。
3. (非必需)如果下载需要登录，可以上网查一下`Oracle帐号`，网上有需要的分享账号可以登录，或者可以直接看：[Oracle官网 账号及密码](https://blog.csdn.net/lu1053682495/article/details/134807450)

> 如果还不会请看(也有配置环境变量的步骤)：[jdk11 下载与安装（非常详细，一步不落！！！）](https://javaziliao.com/post/931.html)

## 二：配置环境变量

配置环境变量是为了让操作系统能够找到Java 11的安装路径。

1. 打开系统环境变量（右键此电脑->属性->高级系统设置->环境变量）。
2. 在系统变量中，找到`Path`，点击后再点击编辑。
3. 点击新建，输入刚才安装Java11的位置里面的bin文件夹的位置，例如：`C:\Program Files\Java\jdk-11\bin`（默认路径）
4. 点击确定。

## 三：配置Android Studio(可能不需要，但我还是配置了)

告诉Android Studio使用Java 11作为默认的Java版本，每个人由于Android Studio版本不同可能不一样。

1. 打开Android Studio。
2. 点击`File`菜单，选择`Project Structure`。
3. 在左侧面板中，选择`SDKs`。
4. 在中间面板中，点击`+`号，选择`JDK`。
5. 点击`JDK home path`的选择文件夹按钮，选择Java 11的安装路径，如`C:\Program Files\Java\jdk-11`。
6. 点击应用按钮
7. 点击左侧面板的`Project`，在右侧的Project SDK中选择你刚才新添加的11。
8. 点击`应用(Apply)`按钮.

> 参考：[解决 Could not resolve com.android.tools.build:gradle:7.4.1](https://blog.csdn.net/hool_wei/article/details/129112242)

到这里大家应该都可以成功打包了

 <span id="jump">1. </span>不过如果和我`NDKAPILevel=latest`配置相同且双击`Install_ProjectName-arm64.bat`后安装失败且控制台出现下面的报错，则需要改为`NDKAPILevel=android-29`：

```
Installing existing application. Failures here indicate a problem with the device (connection or storage permissions) and are fatal.

C:\Users\MY\Documents\Unreal Projects\VRTest\Build\Android_ASTC>C:/Users/My/AppData/Local/Android/Sdk\platform-tools\adb.exe  install VRTest-arm64.apk
Performing Streamed Install
adb: failed to install VRTest-arm64.apk: Failure [INSTALL_FAILED_OLDER_SDK: Failed parse during installPackageLI: /data/app/vmdl1195727077.tmp/base.apk (at Binary XML file line #8): Requires newer sdk version #33 (current version is #29)]

There was an error installing the game or the obb file. Look above for more info.

Things to try:
Check that the device (and only the device) is listed with "ADB$ devices" from a command prompt.
Make sure all Developer options look normal on the device
Check that the device has an SD card.
请按任意键继续. . .
```

  2. 如果出现下面的报错请移步***新的问题***章节

```
A problem occurred configuring root project 'app'.
> Could not resolve all files for configuration ':classpath'.
   > Could not download netty-common-4.1.52.Final.jar (io.netty:netty-common:4.1.52.Final)
      > Could not get resource 'https://repo.maven.apache.org/maven2/io/netty/netty-common/4.1.52.Final/netty-common-4.1.52.Final.jar'.
         > Read timed out
```

# 新的问题

配置成功后，出现了新的报错:

```
A problem occurred configuring root project 'app'.
> Could not resolve all files for configuration ':classpath'.
   > Could not download netty-common-4.1.52.Final.jar (io.netty:netty-common:4.1.52.Final)
      > Could not get resource 'https://repo.maven.apache.org/maven2/io/netty/netty-common/4.1.52.Final/netty-common-4.1.52.Final.jar'.
         > Read timed out
```

经查阅说是Bintray 和 JCenter 截止 2021 年 3 月 30 日之后不能上传任何库，2022 年 2 月 1 日之后不能下载任何库。不过你可以先试试梯子可不可以，因为我觉得也有可能去国外的网速太慢了，毕竟2023年我还可以使用***打包。如果不行再继续往下看。

> 参考：[JCenter挂掉后的解决方案](https://blog.csdn.net/u010629285/article/details/116735063)

Gradle的全局配置(没办法进行项目配置，因为这些配置文件在`Intermediate`每次打包都会更新，不过应该可以不更新，但是我没查，如果你知道请告诉我)

在 C盘用户目录下的.gradle/ 目录下创建 init.gradle 文件(如：`C:\Users\My\.gradle`)，并添加以下内容：

```java
allprojects {
    repositories {
        def ALIYUN_REPOSITORY_URL = 'https://maven.aliyun.com/repository/public'
        all { ArtifactRepository repo ->
            if(repo instanceof MavenArtifactRepository){
                def url = repo.url.toString()
                if (url.startsWith('https://repo1.maven.org/maven2')) {
                    project.logger.lifecycle "Repository ${repo.url} replaced by $ALIYUN_REPOSITORY_URL."
                    remove repo
                }
            }
        }
        maven { url ALIYUN_REPOSITORY_URL }
    }
}
```

保存，退出，再次打包就可以了.

> 参考：[Maven、Gradle 配置国内镜像源](https://www.cnblogs.com/chansblogs/p/12943991.html)

如果安装失败请点击[跳转](#jump)
