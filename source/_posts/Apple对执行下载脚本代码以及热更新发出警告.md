---
title: Apple对执行下载脚本代码以及热更新发出警告
date: 2017-03-08 10:15:46
tags:
- iOS
---

## Apple对执行下载脚本代码以及热更新发出警告

今天很多开发人员收到了苹果公司的一封邮件,内容如下：
<!-- more -->
> Dear Developer,
>
> Your app, extension, and/or linked framework appears to contain code designed explicitly with the capability to change your app’s behavior or functionality after App Review approval, which is not in compliance with section 3.3.2 of the Apple Developer Program License Agreement and App Store Review Guideline 2.5.2. This code, combined with a remote resource, can facilitate significant changes to your app’s behavior compared to when it was initially reviewed for the App Store. While you may not be using this functionality currently, it has the potential to load private frameworks, private methods, and enable future feature changes.
>
> This includes any code which passes arbitrary parameters to dynamic methods such as dlopen(), dlsym(), respondsToSelector:, performSelector:, method_exchangeImplementations(), and running remote scripts in order to change app behavior or call SPI, based on the contents of the downloaded script. Even if the remote resource is not intentionally malicious, it could easily be hijacked via a Man In The Middle (MiTM) attack, which can pose a serious security vulnerability to users of your app.
>
> Please perform an in-depth review of your app and remove any code, frameworks, or SDKs that fall in line with the functionality described above before submitting the next update for your app for review.
>
> Best regards,
>
> App Store Review

随即，像JSPatch、RN等项目立即备受关注：

JSPatch：https://github.com/bang590/JSPatch/issues/746

react-native的情况： https://github.com/facebook/react-native/issues/12778

Weex ：https://github.com/alibaba/weex/issues/2875



对我们带来两个风险：

* 我们的项目接入的公司一个热修复机制库是基于JSPatch的，从各个issues讨论的结果来看，JSPatch在下个版本不出意外的话是上不了了。
* 上面的一条使得应用失去了热更新功能，但不是最糟糕的，目前并不知道苹果是怎么检查出一个应用包含下载脚本代码机制的。如果真的是检查ipa包里是否有这些方法dlopen(), dlsym(), respondsToSelector:, performSelector:, method_exchangeImplementations()，那真是完蛋。这个问题要持续跟进

解决方法：

* 去除热更新机制
* 做好去除热更新依赖的准备
* 检查项目中各个对runtime调用的地方，查看影响。关注开发社区，做好应对准备