---
title: ARKit 学习
date: 2017-09-13 19:13:10
tags:
---

### ARKit学习

welldone 让我们来翻译https://developer.apple.com/documentation/arkit吧  吼吼

https://developer.apple.com/videos/play/wwdc2017/602/

AR (augmented reality 增强现实)，apple给我们提供了一种结合手机摄像头和陀螺仪的AR api。ARKit可以让我们在手机摄像机加入2D或3D模型使得呈现出的捕获画面中模型就好像原本就在真实世界中一样。

ARKit提供给我们陀螺仪运动跟踪，摄像机拍摄，高级图像场景处理等核心功能，并且让我们很方便的显示AR场景。

> 要注意的是ARKit要求具有搭载A9处理器以上设备（如iPhone6s）可以在UIRequiredDeviceCapabilities中设置你的app需要ARKit，也可以用isSupported来动态判断当前设备是否支持ARKit

对现有的或者可以预见的未来的AR技术来说，一个基本的要求就是要能够创建和保持对于使用者所在的现实世界和虚拟模型所在虚拟世界的一种坐标位置的一致性`correspondence`。可以将这种创建和保存一致性的的技术叫做“world tracking”一个AR技术的衡量标准就是当用户在摄像捕获的视频中看到一个虚拟物体，这个物体对于用户来说就好像是在现实世界中一样。

ARKit在camera和world中都使用右手坐标系，Y正方向向上，Z正方形指向观察者，X指向观察者右手边

`Session configurations`可以改变对应真实世界坐标的原点和方向，每一个锚点`anchor`定义了自己的本地坐标系并且同样是右手坐标，Z正方形指向观察者。

### World Tracking 如何工作

为了创建保持上面说的correspondence，ARKit使用一种被称之为`visual-inertial odometry`（世界惯性测量法？）的技术。简单来说就是ARKit会在视频帧中找出一些显著的特征点，并在前后帧中对于他们的变化，将这些变化同摄像机的运动传感器在一起进行处理，得到真实的手机所在位置和运行的变化信息。

World Tracking同时处理和分析摄像机拍摄的内容，使用一个hit-testing的方法用来图像中的像素点对应的世界坐标中位置。ARHitTestResult可以看到详情，也可以使用`planeDetection`设置ARKit自动检测和报告捕捉视频帧中“平坦表面”。

ARKit性能十分优越，但受限于环境和设备，我们在开发中要注意和处理警戒情况

* world tracking要求摄像机场景尽可能的光照充分，world tracking是基于图像分析的，如果摄像机捕捉的视频帧画面的细节太少，如一片空白的墙或黑暗的场景下拍摄的图片，ARKit的识别效果就好很差。
* 可以利用`tracking quality information`跟踪质量信息提供给用户知道，指导用户改变场景环境，从而改善应用体验。从`ARCamera`类中可以获得跟踪状态信息，包括手机运动太远、太快、摇晃剧烈等。
* `planeDetection`需要一定的处理时间，如果使用`planeDetection`进行检测需要保证让用户留有相应停留时间并且关闭planeDetection如果已经检测出了结果。

### ARSession

ARSession在ARkit中是用来管理摄像机和运动处理的类，这个单利类管理了AR的几大主要的过程包括：从设备运动传感器过得数据；从内置摄像机中获取图片；分析和处理数据得到现实世界与模拟世界的坐标关联关系（correspondence）。每一个AR应用都需要一个ARSession，如果在APP中使用了ARSCNView或ARSKView来作为应用中显示AR的视图，那么就自动会有一个ARSession。如果你想自己创建一个渲染器（renderer）那么你就需要自己显式的维护一个ARSession。

开启一个ARSession需要session configuration，ARConfiguration类和其子类如ARWorldTrackingConfiguration等定义了采用何种方式建立与真实世界的坐标关系，会直接影响到AR的表现方式。ARConfiguration是一个抽象类，系统提供三种实现

* `ARWorldTrackingConfiguration`提供一个高质量的AR处理，使用后置摄像头，识别手机的运动和方向允许使用`plane detection `h和`hit testing`
* `AROrientationTrackingConfiguration`提供基本的AR处理功能，同样使用后置摄像头，但仅仅识别手机的方向。
* `ARFaceTrackingConfiguration`使用前置摄像头，捕捉用户面部的表情和运动

### AVDepthData

AVDepthData是用来存放每个像素点的深度（depth data map）和视差（disparity map data）数据信息的容器类，这些深度信息来自手机摄像头。depth data 对象包括了深度数据，转换方法，聚焦参数与摄像机标定参数，这些数据对于rendering过程和相应的计算视觉任务都是必要的。

深度图（depth data map）用米为单位描述每个像素点到一个物体的距离

视差图（disparity map data）描述了两个图片之间差异的归一化数值



### 一个示例程序

这里下载自己喜欢的模型（免费哦）

https://www.turbosquid.com/     

http://www.3dxy.com/3dmodel/49042.html   

https://www.yobi3d.com/cn/  

 https://free3d.com

https://opengameart.org 





*  TODO…..