---
title: OCLint 自定义规则101
date: 2017-03-14 10:31:46
tags:
---

## OCLint 自定义规则101

[TOC]

### 一 、准备开发环境

* mac系统安装有Xcode，git，ruby（一般都会有），还有要会科学上网

* 在OCLint的github上clone下代码 https://github.com/oclint/oclint 。（不要下载官方网站的代码），目录如下：
  <!-- more -->
  ```shell
  ├── README.md
  ├── oclint-core
  ├── oclint-driver
  ├── oclint-metrics
  ├── oclint-reporters
  ├── oclint-rules
  └── oclint-scripts
  ```

* `cd`进入`oclint-scripts`文件加，执行`./make`。大约30分钟后编译完成，大概过程是下载LLVM、clang的源代码，编译LLVM、clang与OCLint预计OCLint的默认规则。（如果通过官网下载的OCLint代码，执行make时会通过SVN下载LLVM的代码，然而即便我科学上网了还是总是下载不成功，建议直接同git上下载OCLint的源码）

* 编译成功后就可以写规则并且编译执行了。为了方便，OCLint提供了一个叫`scaffoldRule`的脚本程序，它在`oclint-rules`目录下。我们通过他传入要生成的规则名，级别，类型，脚本就会在目录`oclint-rules/rules/custom/`自动帮我们生成一个模板代码,并且加入编译路径中。举个例子：

  ```shell
  # 生成一个名为HdwTestRule类型是ASTVisitor的规则模板
  oclint-scripts/scaffoldRule HdwTestRule -t ASTVisitor
  ```

  生成两个文件：

  ```shell
  # CMakeLists.txt 是对规则HdwTestRule的编译描述，由make程序在编译时使用。HdwTestRule.cpp的内容之后再分析。
  ├── custom
  │   ├── CMakeLists.txt
  │   └── HdwTestRule.cpp
  ```

* 接着就可以对新添加的内容进行编译了，不过相比于重新执行`oclint-scripts/make`来说有一个更加优雅的办法，就是将规则相关的内容整合成一个Xcode工程，并且我们的每个规则都是一个scheme，编译时可以只选择编译那个选择的规则生成对应的dylib。做饭很简单，OCLint工程使用`CMakeLists`的方式维护各个文件的依赖关系，我们可以使用CMake自带的功能将这些CMakeLists生成一个`xcodeproj`工程文件。but how？下面栗子：

  ```shell
  # 在OCLint源码目录下建立一个文件夹，我这里命名为oclint-xcoderules
  mkdir oclint-xcoderules
  cd oclint-xcoderules
  # 创建一个脚本(代码如下段)，并执行它(我写的方便修改参数，其实里面就一句命令，直接执行也行)(PS:刚创建的文件是没有执行权限的，不要忘了chmod)
  ./create-xcode-rules.sh
  ```

  ```shell
  #! /bin/sh -e
  cmake -G Xcode -D CMAKE_CXX_COMPILER=../build/llvm-install/bin/clang++  -D CMAKE_C_COMPILER=../build/llvm-install/bin/clang -D OCLINT_BUILD_DIR=../build/oclint-core -D OCLINT_SOURCE_DIR=../oclint-core -D OCLINT_METRICS_SOURCE_DIR=../oclint-metrics -D OCLINT_METRICS_BUILD_DIR=../build/oclint-metrics -D LLVM_ROOT=../build/llvm-install/ ../oclint-rules
  ```

  于是我们就得到了xcode工程：

  ```shell
  ├── CMakeCache.txt
  ├── CMakeFiles
  ├── CMakeScripts
  ├── OCLINT_RULES.build
  ├── OCLINT_RULES.xcodeproj
  ├── cmake_install.cmake
  ├── create-xcode-rules.sh
  ├── lib
  ├── rules
  └── rules.dl
  ```

  打开`OCLINT_RULES.xcodeproj`:

  ![Snip20170313_3](OCLint 自定义规则101/Snip20170313_3.png)

* 选择自己的规则，编译，成功后就可以在Products下看到生成的dylib啦，想想还有点小激动呢

### 二、开发规则

上面准备环境的一些步骤其实都是来自对OCLint官网的总结，同样如何去写自定义的规则还是要去看文档 http://docs.oclint.org/en/stable/ ，甚至于还要看clang的文档 http://clang.llvm.org/docs/IntroductionToTheClangAST.html  。我这里只能给出一些小建议，大概就是如何123去完成自定义的规则，希望再你阅读完文档但是不知道如何下手的时候能够帮到忙。

 《 *寓言故事*》

小明看了OCLint的官方文档，了解了一下clang AST的知识，并读了一些OCLint AST的源码，大概知道OCLint调用clang 的API把一个个源文件生成一个一个AST，然后遍历树中的每个节点传入各个规则的整个过程。然后小明按照与上面类似的步骤最终生成并打开了OCLINT_RULES.xcodeproj。他觉得最好的学习方式是先阅读OCLint给出的默认规则的代码，大概读了几个后他似有所悟，于是就打开了之前自定义自动生成的`HdwTestRule.cpp`文件开始自己编写。

小明决定写一个规则用于检查代码中“含有self的block中没有使用strongify”的那些block。

小明打开HdwTestRule.cpp，他看到一大堆(大概有两千行)注释代码，类似于：

```c++
class MissingStrongifyInCatchSelfBlockRule : public AbstractASTVisitorRule<MissingStrongifyInCatchSelfBlockRule>
    {
      /* Visit NullStmt
    bool VisitNullStmt(NullStmt *node)
    {
        return true;
    }
     */

    /* Visit CompoundStmt
    bool VisitCompoundStmt(CompoundStmt *node)
    {
        return true;
    }
     */

    /* Visit LabelStmt
    bool VisitLabelStmt(LabelStmt *node)
    {
        return true;
    }
     */

    /* Visit AttributedStmt
    bool VisitAttributedStmt(AttributedStmt *node)
    {
        return true;
    }
     */

    /* Visit IfStmt
    bool VisitIfStmt(IfStmt *node)
    {
        return true;
    }
     */

    /* Visit SwitchStmt
    bool VisitSwitchStmt(SwitchStmt *node)
    {
        return true;
    }
     */

    /* Visit WhileStmt
    bool VisitWhileStmt(WhileStmt *node)
    {
        return true;
    }
     */
    }
```

小明嘴角微微一笑，因为他知道，就在上午他刚读过的OCLint的源码中表明这些以Visit开头的百十个函数都是OCLint提供给开发者的回调函数。小明机智的按下了`command + f`输入了`block`，结果他找到了两个回调：

```c++
bool VisitBlockExpr(BlockExpr *node)
{
    return true;
}
bool VisitBlockDecl(BlockDecl *node)
{
    return true;
}
     
```

小明知道`BlockExpr`和`BlockDecl`都是clang定义的AST树节点的类型，OCLint在调用clang API遍历语法树时遇到block表达式或声明语句会分别调用`BlockExpr`和`BlockDecl`对应的两个visit回调。但是小明现在有点尴尬，因为它不确定应该在VisitBlockExpr中处理还是在VisitBlockDecl处理检测`含有self的block中没有使用strongify`这件事情，并且他也忘了上午刚刚在clang文档中看到的block节点内部的遍历方法。小明有点慌乱，在浏览器中不停的切换着OCLint文档页面与clang文档页面的tab，他自己也不知道自己要找什么，突然，不知道为什么，也许是隔壁小王的一个喷嚏提醒了他，clang有一个dump功能可以查看一段代码的语法树结构，小明很激动，他觉得这也许能帮助他。于是他飞快的建立了一个名为`testOCLint.xcodeproj`的xcode控制台应用工程，并写了三行代码:

```objective-c
int main(int argc, const char * argv[]) {
    id aBlock = ^{
        NSLog(@"Hello, World!");
    }
    return 0;
}
```

不到30秒做完这一切的他似乎很是得意，右手拿起刚刚泡好的热茶喝了一口，可是突然，不知是茶有点热还是他忘记了clang如何dump出AST，嘴角刚漏出的笑意僵住了，只见他飞速的移动鼠标在浏览器上搜索了一会，随即又在控制台输入：

```shell
clang -Xclang -ast-dump -fsyntax-only ./testOCLint/main.m
```

看着命令行飞速闪过的彩色的输出，小明忘记了刚刚的一点不快，眼神最终钉在了最后几行输出上：

```shell
`-FunctionDecl 0x7fe97956cc70 <./testOCLint/main.m:11:1, line:16:1> line:11:5 main 'int (int, const char **)'
  |-ParmVarDecl 0x7fe97956cab0 <col:10, col:14> col:14 argc 'int'
  |-ParmVarDecl 0x7fe97956cb60 <col:20, col:38> col:33 argv 'const char **':'const char **'
  `-CompoundStmt 0x7fe97956d010 <col:41, line:16:1>
    `-DeclStmt 0x7fe97956cff8 <line:12:5, line:15:5>
      `-VarDecl 0x7fe97956cd38 <line:12:5, line:14:5> line:12:8 aBlock 'id':'id' cinit
        `-ImplicitCastExpr 0x7fe97956cfe0 <col:17, line:14:5> 'id':'id' <BlockPointerToObjCPointerCast>
          `-BlockExpr 0x7fe97956cfc8 <line:12:17, line:14:5> 'void (^)(void)'
            `-BlockDecl 0x7fe97956cd98 <line:12:17, line:14:5> line:12:17
              `-CompoundStmt 0x7fe97956cfa8 <col:18, line:14:5>
                `-CallExpr 0x7fe97956cf60 <line:13:9, col:31> 'void'
                  |-ImplicitCastExpr 0x7fe97956cf48 <col:9> 'void (*)(id, ...)' <FunctionToPointerDecay>
                  | `-DeclRefExpr 0x7fe97956ce68 <col:9> 'void (id, ...)' Function 0x7fe9778a91f0 'NSLog' 'void (id, ...)'
                  `-ImplicitCastExpr 0x7fe97956cf90 <col:15, col:16> 'id':'id' <BitCast>
                    `-ObjCStringLiteral 0x7fe97956cec8 <col:15, col:16> 'NSString *'
                      `-StringLiteral 0x7fe97956ce90 <col:16> 'char [14]' lvalue "Hello, World!"
```

小明知道这是main函数对应的AST结构，他很快看明白这些内容的大概，确定了自己应该处理BlockDecl回调，可他还不太确定应该如何遍历这个节点内容来做到检测`含有self的block中没有使用strongify`，但他心中隐隐有个想法，于是改定了一下main函数：

```objective-c
@interface TestClass : NSObject

@end

@implementation TestClass

- (void)testFun
{
    id aBlock = ^{
        NSLog(@"Hello, World! %@", NSStringFromClass(self.class));
    }
}

@end

int main(int argc, const char * argv[]) {
    
    return 0;
}
```

紧接着再次dump：

```shell
|-ObjCInterfaceDecl 0x7fd807eb4298 prev 0x7fd807e89570 <line:193:1, line:197:2> line:193:12 NSXPCListenerEndpoint
| |-super ObjCInterface 0x7fd803a04568 'NSObject'
| |-ObjCProtocol 0x7fd803b58968 'NSSecureCoding'
| |-AvailabilityAttr 0x7fd807eb4318 </System/Library/Frameworks/CoreFoundation.framework/Headers/CFAvailability.h:43:49, col:84> macos 10_8 0 0 "" ""
| |-VisibilityAttr 0x7fd807eb43b8 </System/Library/Frameworks/Foundation.framework/Headers/NSObjCRuntime.h:324:55, col:75> Default
| `-ObjCIvarDecl 0x7fd807eb4448 </System/Library/Frameworks/Foundation.framework/Headers/NSXPCConnection.h:195:5, col:11> col:11 _internal 'void *' private
|-ObjCInterfaceDecl 0x7fd807eb44a0 <./testOCLint/main.m:11:1, line:13:2> line:11:12 TestClass
| |-super ObjCInterface 0x7fd803a04568 'NSObject'
| `-ObjCImplementation 0x7fd807eb45b0 'TestClass'
|-ObjCImplementationDecl 0x7fd807eb45b0 <line:15:1, line:24:1> line:15:17 TestClass
| |-ObjCInterface 0x7fd807eb44a0 'TestClass'
| `-ObjCMethodDecl 0x7fd807eb4648 <line:17:1, line:22:1> line:17:1 - testFun 'void'
|   |-ImplicitParamDecl 0x7fd807eb4708 <<invalid sloc>> <invalid sloc> implicit used self 'TestClass *'
|   |-ImplicitParamDecl 0x7fd807eb4768 <<invalid sloc>> <invalid sloc> implicit _cmd 'SEL':'SEL *'
|   |-VarDecl 0x7fd807eb47d8 <line:19:5, line:21:5> line:19:8 aBlock 'id':'id' cinit
|   | `-ExprWithCleanups 0x7fd807eb4c90 <col:17, line:21:5> 'id':'id'
|   |   |-cleanup Block 0x7fd807eb4838
|   |   `-ImplicitCastExpr 0x7fd807eb4c78 <line:19:17, line:21:5> 'id':'id' <BlockPointerToObjCPointerCast>
|   |     `-BlockExpr 0x7fd807eb4c60 <line:19:17, line:21:5> 'void (^)(void)'
|   |       `-BlockDecl 0x7fd807eb4838 <line:19:17, line:21:5> line:19:17
|   |         |-capture ImplicitParam 0x7fd807eb4708 'self' 'TestClass *'
|   |         `-CompoundStmt 0x7fd807eb4c30 <col:18, line:21:5>
|   |           `-CallExpr 0x7fd807eb4be0 <line:20:9, col:65> 'void'
|   |             |-ImplicitCastExpr 0x7fd807eb4bc8 <col:9> 'void (*)(id, ...)' <FunctionToPointerDecay>
|   |             | `-DeclRefExpr 0x7fd807eb4908 <col:9> 'void (id, ...)' Function 0x7fd803b4c5f0 'NSLog' 'void (id, ...)'
|   |             |-ImplicitCastExpr 0x7fd807eb4c18 <col:15, col:16> 'id':'id' <BitCast>
|   |             | `-ObjCStringLiteral 0x7fd807eb4968 <col:15, col:16> 'NSString *'
|   |             |   `-StringLiteral 0x7fd807eb4930 <col:16> 'char [17]' lvalue "Hello, World! %@"
|   |             `-CallExpr 0x7fd807eb4b40 <col:36, col:64> 'NSString * _Nonnull':'NSString *'
|   |               |-ImplicitCastExpr 0x7fd807eb4b28 <col:36> 'NSString * _Nonnull (*)(Class _Nonnull)' <FunctionToPointerDecay>
|   |               | `-DeclRefExpr 0x7fd807eb4988 <col:36> 'NSString * _Nonnull (Class _Nonnull)' Function 0x7fd803b39420 'NSStringFromClass' 'NSString * _Nonnull (Class _Nonnull)'
|   |               `-PseudoObjectExpr 0x7fd807eb4aa0 <col:54, col:59> 'Class':'Class'
|   |                 |-ObjCPropertyRefExpr 0x7fd807eb4a40 <col:54, col:59> '<pseudo-object type>' lvalue objcproperty Kind=MethodRef Getter="class" Setter="(null)" Messaging=Getter
|   |                 | `-OpaqueValueExpr 0x7fd807eb4a20 <col:54> 'TestClass *'
|   |                 |   `-ImplicitCastExpr 0x7fd807eb49d8 <col:54> 'TestClass *' <LValueToRValue>
|   |                 |     `-DeclRefExpr 0x7fd807eb49b0 <col:54> 'TestClass *const' lvalue ImplicitParam 0x7fd807eb4708 'self' 'TestClass *'
|   |                 |-OpaqueValueExpr 0x7fd807eb4a20 <col:54> 'TestClass *'
|   |                 | `-ImplicitCastExpr 0x7fd807eb49d8 <col:54> 'TestClass *' <LValueToRValue>
|   |                 |   `-DeclRefExpr 0x7fd807eb49b0 <col:54> 'TestClass *const' lvalue ImplicitParam 0x7fd807eb4708 'self' 'TestClass *'
|   |                 `-ObjCMessageExpr 0x7fd807eb4a70 <col:59> 'Class':'Class' selector=class
|   |                   `-OpaqueValueExpr 0x7fd807eb4a20 <col:54> 'TestClass *'
|   |                     `-ImplicitCastExpr 0x7fd807eb49d8 <col:54> 'TestClass *' <LValueToRValue>
|   |                       `-DeclRefExpr 0x7fd807eb49b0 <col:54> 'TestClass *const' lvalue ImplicitParam 0x7fd807eb4708 'self' 'TestClass *'
|   |-BlockDecl 0x7fd807eb4838 <line:19:17, line:21:5> line:19:17
|   | |-capture ImplicitParam 0x7fd807eb4708 'self' 'TestClass *'
|   | `-CompoundStmt 0x7fd807eb4c30 <col:18, line:21:5>
|   |   `-CallExpr 0x7fd807eb4be0 <line:20:9, col:65> 'void'
|   |     |-ImplicitCastExpr 0x7fd807eb4bc8 <col:9> 'void (*)(id, ...)' <FunctionToPointerDecay>
|   |     | `-DeclRefExpr 0x7fd807eb4908 <col:9> 'void (id, ...)' Function 0x7fd803b4c5f0 'NSLog' 'void (id, ...)'
|   |     |-ImplicitCastExpr 0x7fd807eb4c18 <col:15, col:16> 'id':'id' <BitCast>
|   |     | `-ObjCStringLiteral 0x7fd807eb4968 <col:15, col:16> 'NSString *'
|   |     |   `-StringLiteral 0x7fd807eb4930 <col:16> 'char [17]' lvalue "Hello, World! %@"
|   |     `-CallExpr 0x7fd807eb4b40 <col:36, col:64> 'NSString * _Nonnull':'NSString *'
|   |       |-ImplicitCastExpr 0x7fd807eb4b28 <col:36> 'NSString * _Nonnull (*)(Class _Nonnull)' <FunctionToPointerDecay>
|   |       | `-DeclRefExpr 0x7fd807eb4988 <col:36> 'NSString * _Nonnull (Class _Nonnull)' Function 0x7fd803b39420 'NSStringFromClass' 'NSString * _Nonnull (Class _Nonnull)'
|   |       `-PseudoObjectExpr 0x7fd807eb4aa0 <col:54, col:59> 'Class':'Class'
|   |         |-ObjCPropertyRefExpr 0x7fd807eb4a40 <col:54, col:59> '<pseudo-object type>' lvalue objcproperty Kind=MethodRef Getter="class" Setter="(null)" Messaging=Getter
|   |         | `-OpaqueValueExpr 0x7fd807eb4a20 <col:54> 'TestClass *'
|   |         |   `-ImplicitCastExpr 0x7fd807eb49d8 <col:54> 'TestClass *' <LValueToRValue>
|   |         |     `-DeclRefExpr 0x7fd807eb49b0 <col:54> 'TestClass *const' lvalue ImplicitParam 0x7fd807eb4708 'self' 'TestClass *'
|   |         |-OpaqueValueExpr 0x7fd807eb4a20 <col:54> 'TestClass *'
|   |         | `-ImplicitCastExpr 0x7fd807eb49d8 <col:54> 'TestClass *' <LValueToRValue>
|   |         |   `-DeclRefExpr 0x7fd807eb49b0 <col:54> 'TestClass *const' lvalue ImplicitParam 0x7fd807eb4708 'self' 'TestClass *'
|   |         `-ObjCMessageExpr 0x7fd807eb4a70 <col:59> 'Class':'Class' selector=class
|   |           `-OpaqueValueExpr 0x7fd807eb4a20 <col:54> 'TestClass *'
|   |             `-ImplicitCastExpr 0x7fd807eb49d8 <col:54> 'TestClass *' <LValueToRValue>
|   |               `-DeclRefExpr 0x7fd807eb49b0 <col:54> 'TestClass *const' lvalue ImplicitParam 0x7fd807eb4708 'self' 'TestClass *'
|   `-CompoundStmt 0x7fd807eb4cc8 <line:18:1, line:22:1>
|     `-DeclStmt 0x7fd807eb4cb0 <line:19:5, line:22:1>
|       `-VarDecl 0x7fd807eb47d8 <line:19:5, line:21:5> line:19:8 aBlock 'id':'id' cinit
|         `-ExprWithCleanups 0x7fd807eb4c90 <col:17, line:21:5> 'id':'id'
|           |-cleanup Block 0x7fd807eb4838
|           `-ImplicitCastExpr 0x7fd807eb4c78 <line:19:17, line:21:5> 'id':'id' <BlockPointerToObjCPointerCast>
|             `-BlockExpr 0x7fd807eb4c60 <line:19:17, line:21:5> 'void (^)(void)'
|               `-BlockDecl 0x7fd807eb4838 <line:19:17, line:21:5> line:19:17
|                 |-capture ImplicitParam 0x7fd807eb4708 'self' 'TestClass *'
|                 `-CompoundStmt 0x7fd807eb4c30 <col:18, line:21:5>
|                   `-CallExpr 0x7fd807eb4be0 <line:20:9, col:65> 'void'
|                     |-ImplicitCastExpr 0x7fd807eb4bc8 <col:9> 'void (*)(id, ...)' <FunctionToPointerDecay>
|                     | `-DeclRefExpr 0x7fd807eb4908 <col:9> 'void (id, ...)' Function 0x7fd803b4c5f0 'NSLog' 'void (id, ...)'
|                     |-ImplicitCastExpr 0x7fd807eb4c18 <col:15, col:16> 'id':'id' <BitCast>
|                     | `-ObjCStringLiteral 0x7fd807eb4968 <col:15, col:16> 'NSString *'
|                     |   `-StringLiteral 0x7fd807eb4930 <col:16> 'char [17]' lvalue "Hello, World! %@"
|                     `-CallExpr 0x7fd807eb4b40 <col:36, col:64> 'NSString * _Nonnull':'NSString *'
|                       |-ImplicitCastExpr 0x7fd807eb4b28 <col:36> 'NSString * _Nonnull (*)(Class _Nonnull)' <FunctionToPointerDecay>
|                       | `-DeclRefExpr 0x7fd807eb4988 <col:36> 'NSString * _Nonnull (Class _Nonnull)' Function 0x7fd803b39420 'NSStringFromClass' 'NSString * _Nonnull (Class _Nonnull)'
|                       `-PseudoObjectExpr 0x7fd807eb4aa0 <col:54, col:59> 'Class':'Class'
|                         |-ObjCPropertyRefExpr 0x7fd807eb4a40 <col:54, col:59> '<pseudo-object type>' lvalue objcproperty Kind=MethodRef Getter="class" Setter="(null)" Messaging=Getter
|                         | `-OpaqueValueExpr 0x7fd807eb4a20 <col:54> 'TestClass *'
|                         |   `-ImplicitCastExpr 0x7fd807eb49d8 <col:54> 'TestClass *' <LValueToRValue>
|                         |     `-DeclRefExpr 0x7fd807eb49b0 <col:54> 'TestClass *const' lvalue ImplicitParam 0x7fd807eb4708 'self' 'TestClass *'
|                         |-OpaqueValueExpr 0x7fd807eb4a20 <col:54> 'TestClass *'
|                         | `-ImplicitCastExpr 0x7fd807eb49d8 <col:54> 'TestClass *' <LValueToRValue>
|                         |   `-DeclRefExpr 0x7fd807eb49b0 <col:54> 'TestClass *const' lvalue ImplicitParam 0x7fd807eb4708 'self' 'TestClass *'
|                         `-ObjCMessageExpr 0x7fd807eb4a70 <col:59> 'Class':'Class' selector=class
|                           `-OpaqueValueExpr 0x7fd807eb4a20 <col:54> 'TestClass *'
|                             `-ImplicitCastExpr 0x7fd807eb49d8 <col:54> 'TestClass *' <LValueToRValue>
|                               `-DeclRefExpr 0x7fd807eb49b0 <col:54> 'TestClass *const' lvalue ImplicitParam 0x7fd807eb4708 'self' 'TestClass *'
`-FunctionDecl 0x7fd807eb4ed0 <line:26:1, line:29:1> line:26:5 main 'int (int, const char **)'
  |-ParmVarDecl 0x7fd807eb4d10 <col:10, col:14> col:14 argc 'int'
  |-ParmVarDecl 0x7fd807eb4dc0 <col:20, col:38> col:33 argv 'const char **':'const char **'
  `-CompoundStmt 0x7fd807eb4fc0 <col:41, line:29:1>
    `-ReturnStmt 0x7fd807eb4fa8 <line:28:5, col:12>
      `-IntegerLiteral 0x7fd807eb4f88 <col:12> 'int' 0
```

这次命令行滚动的输出已经不能对他带来任何波澜，他的目光紧紧盯着那关键的几行：

```shell
`-BlockDecl 0x7fd807eb4838 <line:19:17, line:21:5> line:19:17
|   |         |-capture ImplicitParam 0x7fd807eb4708 'self' 'TestClass *'
|   |         `-CompoundStmt 0x7fd807eb4c30 <col:18, line:21:5>
|   |           `-CallExpr 0x7fd807eb4be0 <line:20:9, col:65> 'void'
```

看到BlockDecl下面的capture节点，他笑了，他之前就知道如果在visitBlockDecl中找到含有self capture节点，在判断其有无strongify，问题就解决了。至于如何得到capture节点确实难不了他，只见他淡定的在Google搜索框里输入`clang BlockDecl`,又毫不犹豫的几乎看都不看搜索列表就点开了第一条，因为他知道第一条总是链接到clang的官方文档。只见浏览器打开的新的tab页果然是clang官方文档并且惊人的就是介绍BlockDecl类的相关内容的页面。小明只定睛一看，随即眼睛又微微闭上，好似在说“我就知道！”。接着他又熟练的按下的`command + f`输入`capture`。几次回车后，他找到了几个相关函数：

```c++
ArrayRef< Capture > 	captures () const
capture_const_iterator 	capture_begin () const
capture_const_iterator 	capture_end () const
bool 	capturesCXXThis () const
```

这时他明白了如何获得bock中捕获变量，随即就写了如下代码：

```c++
class MissingStrongifyInCatchSelfBlockRule : public AbstractASTVisitorRule<MissingStrongifyInCatchSelfBlockRule>
    {
        public:
        
        virtual const string name() const override
        {
            return "含有self的block中没有使用strongify";
        }
        
        virtual int priority() const override
        {
            return 1;
        }
        
        virtual const string category() const override
        {
            return "iMerchant";
        }
        
        bool VisitBlockDecl(BlockDecl *node)
        {
            for (BlockDecl::capture_const_iterator iterator = node->capture_begin() ; iterator != node->capture_end(); iterator ++) {
                ImplicitParamDecl *implicitParamDecl = dyn_cast_or_null<ImplicitParamDecl>(iterator->getVariable());
                if (implicitParamDecl && implicitParamDecl->getName() == "self") {
                    if (!BlockDeclHasStrongify(node)) {
                        addViolation(node, this, description("这个block中引用了self，但是没有使用@strongify"));
                    }
                }
            }
            return true;
        }
    }
```

现在就剩下函数`BlockDeclHasStrongify`如何实现的问题，判断这个block中是否含有`@strongify`,他觉得只要得到block部分对应的源码，然后再字符串匹配一下strongify就行。关于如何获得AST节点在源码中的位置，以及获得对应的代码，他在之前阅读OCLint中预设规则源码的时候已经学到了，至于匹配查找他知道正则表达式可以使用。之后经历查文档->看dump->生成dylib->复制dylib到OCLint规则目录下->执行检测->查看报告->发现问题->修改代码重新生成dylib这个恶心又麻烦的调试过程后，终于完善了`VisitBlockDecl`和`BlockDeclHasStrongify`函数：

```c++
bool BlockDeclHasStrongify(BlockDecl *node)
        {
            regex pattern("\\^[^\\{\\}\\^]*\\{[^\\{\\}]*\\}");
            regex patternAnnotationStrongify("//[^\n]*@strongify");
            regex patternOtherAnnotationStrongify("/\\*[\\s\\S]*@strongify[\\s\\S]*\\*/");
            
            clang::SourceManager *sourceManager = &_carrier->getSourceManager();
            SourceLocation startLocation = node->getLocStart();
            SourceLocation endLocation = node->getLocEnd();
            if (sourceManager->isMacroBodyExpansion(startLocation)
                || sourceManager->isInSystemMacro(startLocation)
                || sourceManager->isMacroArgExpansion(startLocation)) {
                return true;
            }
            SourceLocation startSpellingLoc = startLocation, endSpellingLoc = endLocation;
            if (!startLocation.isFileID()) {
                startSpellingLoc = sourceManager->getSpellingLoc(startLocation);
                endSpellingLoc = sourceManager->getSpellingLoc(endLocation);
            }
            int length = sourceManager->getFileOffset(endSpellingLoc) -  sourceManager->getFileOffset(startSpellingLoc);
            if (length > 0) {
                string blockStatementString = StringRef(sourceManager->getCharacterData(startLocation) + 1, length).str();
                match_results<string::const_iterator> result;
                string newBlockStatementString(blockStatementString);
                do {
                    blockStatementString = newBlockStatementString;
                    newBlockStatementString = std::regex_replace(blockStatementString, pattern, "");
                } while (newBlockStatementString.length() != blockStatementString.length());
                blockStatementString = std::regex_replace(blockStatementString, patternAnnotationStrongify, "");
                blockStatementString = std::regex_replace(blockStatementString, patternOtherAnnotationStrongify, "");
                if (blockStatementString.find("@strongify") == string::npos) {
                    return false;
                } else {
                    return true;
                }
            } else {
                return false;
            }
        }
```

就在这时小王叫他吃饭，小明对小王说他成功写了一个OC语法检查的插件，小王也说了自己今天的收获。然而这些都不影响他们晚饭照常点了黄焖鸡，并讨论国家大事......