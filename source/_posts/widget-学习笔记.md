---
title: widget 学习笔记
date: 2016-10-12 10:07:37
tags:
- iOS
---
## widget 学习笔记

[TOC]

## 什么是wiget

iOS7引入了一个叫Today的东东，它和通知在一起出现在系统下拉菜单(通知栏)的两个tab中。每个APP可以建立一些扩展，其中在Today中展示的扩展就是wiget
<!-- more -->
> A widget is an extension that displays a small amount of timely, useful information or app-specific functionality. For example, the News widget shows top headlines. Calendar provides two widgets, one that shows today’s events and one that shows what’s up next. Notes lets you preview recent notes and quickly create new notes, reminders, photos, and drawings. Widgets are highly customizable and can contain buttons, text, layout customizations, images, and more.

在today中的widget应性能时候优越，因为iOS对其限制很多。要考虑内存占用和用户及时响应，并且widget不能使用键盘。

## 如何创建一个wiget

在工程中新建一个target，选择Today Extension，就是通常意义上的窗口插件也就是widget。 ![Snip20161012_1](widget 学习笔记/Snip20161012_1.png)

这么模板会创建一个带有一个TodayViewController、一个info.plist和一个storyboard。如果不想使用storyboard需要修改info.plis的配置。 ![Snip20161012_6](widget 学习笔记/Snip20161012_6.png)

直接点击运行，在iOS10上的表现如下：

![Snip20161012_7](widget 学习笔记/Snip20161012_7.png)

现在先来关心三个问题：

* TodayViewController的生命周期
* widget在today中的大小
* widget与宿主APP的关系

>  podfile 中要添加widget这个target的依赖项，要不然不能使用pod里的类库

## TodayViewController的生命周期

TodayViewController会在每次用户下拉通知栏是被创建，此时`viewDidLoad`会被调用，在通知栏弹走时`viewDidDisappear`被调用。所以数据更新在viewWillAppear时是合适的，此外考虑到如何用户长时间盯着下拉栏看，我们要考虑一定时器更新数据的策略。但一般只是在widget上添加一些按钮，快速进行APP的某一个页面中而已。

就这样？额，复杂的我还没遇到，以后遇到坑在补充吧。

## widget在today中的大小

widget在tody中的大小是确定的，以iPhone6\7为例：宽359 高110。widget最大可以为528。可以动态调节高度，使用preferredContentSize属性。实现`widgetActiveDisplayModeDidChange`可使用展开缩入功能。遇到的坑：

* 不能依赖`widgetLargestAvailableDisplayMode`实现展开缩小功能，NCWidgetDisplayModeExpanded只描述一种能力，表示自己的widget可以被展开。

  > 也就是说如果你设置为NCWidgetDisplayModeCompact，则widget的右上角永远不会出现展开缩小按钮。如何你设置为NCWidgetDisplayModeExpanded，则在第一次运行时展开，以后系统内部维护了一个flag控制器是否展开。（我也觉得这样很奇怪，但这是我实验得出的结论，不知是否是其他原因导致的）

* 在`widgetActiveDisplayModeDidChange`中缩小时高度不是preferredContentSize设置的值，而是固定值110。

  > 这个也是实验得出的结论,下面代码中无论把110改为何值，缩小后的高度都是110。但如果你什么都不写（即把这一行删掉）那么它不能展开了。。。真是无语，我测试代码如此，不知道是不是其他原因导致。

  ```objective-c
  -(void)widgetActiveDisplayModeDidChange:(NCWidgetDisplayMode)activeDisplayMode withMaximumSize:(CGSize)maxSize
  {
      NSLog(@"maxSize(w:%f h:%f)",maxSize.width,maxSize.height);
      
      if (activeDisplayMode == NCWidgetDisplayModeCompact) {
          self.preferredContentSize = CGSizeMake([UIScreen mainScreen].bounds.size.width, 110);
      } else {
          self.preferredContentSize = CGSizeMake([UIScreen mainScreen].bounds.size.width, 300);
      }
      
  }
  ```

* 在展开后，可以随意的设置preferredContentSize并及时有效。

  我在widget上加了两个按钮A,B。展开和缩小时的样子分别如下：
  | 缩小                                       | 展开                                       |
  | ---------------------------------------- | ---------------------------------------- |
  | ![Snip20161012_8](widget 学习笔记/Snip20161012_8.png) | ![Snip20161012_9](widget 学习笔记/Snip20161012_9.png) |
  两个按钮的点击回调是同一个函数，代码如下：
  ```objective-c
  - (void)click:(id)sender
  {
      NSLog(@"我是测试按钮:%@",sender);
      UIButton *temp = (UIButton *)sender;
      if ([temp.titleLabel.text isEqualToString:@"我是测试按钮A"]) {
          self.preferredContentSize = CGSizeMake([UIScreen mainScreen].bounds.size.width, 400);
      }else{//我是测试按钮B
          self.preferredContentSize = CGSizeMake([UIScreen mainScreen].bounds.size.width, 150);
      }
  }
  ```

  当缩小时，点击B是没有反应的。展开时，点击A和B都会改变widget的高度。对此我也十分无语。


## widget与宿主APP的关系

widget与宿主的关系其实就是Extension与宿主APP的关系，https://developer.apple.com/library/content/documentation/General/Conceptual/ExtensibilityPG/ExtensionOverview.html#//apple_ref/doc/uid/TP40014214-CH2-SW2一文中有详细的描述。核心思想就是下面两个图啦：

| Extension生命周期图                           | Extension与上下文（宿主）APP与父APP直接通信            |
| ---------------------------------------- | ---------------------------------------- |
| ![Snip20161012_10](widget 学习笔记/Snip20161012_10.png) | ![Snip20161012_11](widget 学习笔记/Snip20161012_11.png) |

要了解四个通信方向的API：

* Request

* Response

* OpenURL

  * url schemes

    URL Schemes是苹果给出的用来跳转到系统应用或者跳转到别人的应用的一种机制。同时还可以在应用之间传数据。

* Shared resources

  * 扩展（Extension）是 iOS 8 和 OSX 10.10 中引入新功能，Extension 不会单独存在，它将做为 App 的附加功能出现，为 App 提供更好的交互体验。由此 Extension 和容器应用（Containing App）之间的数据共享在所难免。Apple 在它的[官方文档](https://developer.apple.com/library/ios/documentation/General/Conceptual/ExtensibilityPG/ExtensionScenarios.html#//apple_ref/doc/uid/TP40014214-CH21-SW6)中只是以 NSUserDefaults 举例做了介绍，然后丢了一句“ Use Core Data, SQLite, or Posix locks to help coordinate data access in a shared container.”就算完事儿了。下面就来介绍下具体如何操作。

  * 在默认情况下，Extension 是无法直接获取 Containing App 的数据的，但在 iOS 8 中我们可以通过开启 App Groups 以实现同一个 team 的 Apps 及其 Extension 之间的数据共享。开启 App Groups：

    - 在 Xcode 6 的 Targets 中选中主 App‘s Target，找到`Capabilities`标签
    - 找到`App Groups`选项并展开，然后戳一下添加按钮
    - 给你的 App Groups 起个名字，通常是 `group.xxx`的格式，随后 Xcode 自动创建`.entitlements`授权文件，其中包含了共享容器的访问名称，并将此 App Group 登记在你的开发者账号下的，确保只有你的 team 的 app 可以使用这些共享容器 .
    - 选中 Extension’s Target，重复以上操作，勾选刚才创建的 group 就可以了

    现在你把原来存在 App 沙箱中的数据改存在 这个 group 中就可以实现数据的共享了，在具体举例之前，你可能发现仅有数据好像还不够，比如你使用了 CoreData，还需要选中 `.xcdatamodeld`文件和要用到的 model 文件，在 Xcode 的右侧工具栏中的找到`Target Membership`勾选 Extension 名，把它们加到 Extension Target 中。然后把 `.xcdatamodeld`文件加到 Extension 的 Resource Bundle 里面：

    - Targets 中选中 Extension‘s Target，找到`Build Phase`标签
    - 找到`Copy Bundle Resources`选项并展开，然后戳一下添加按钮
    - 添加相应的`.xcdatamodel`文件

    ### 数据共享

    好了，一切准备就绪，把

    ```
    [[[NSFileManager defaultManager] URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask] lastObject];
    ```

    换成

    ```
    [[NSFileManager defaultManager] containerURLForSecurityApplicationGroupIdentifier:@"group.xxx"] 

    ```

    也就是让主 App 和 Extension 都去访问共享容器中的文件，从而实现数据的共享。现在“you can also use Core Data, or in some cases SQLite, to help coordinate data access in a shared container."

    如果是 `NSUserDefaults` 就是把

    ```
    NSUserDefaults *defaults ＝ [NSUserDefaults standardUserDefaults]; 

    ```

    换成

    ```
    NSUserDefaults *sharedDefaults = [[NSUserDefaults alloc] initWithSuiteName:@"group.xxx"];
    ```

