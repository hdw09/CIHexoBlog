---
title: OC的message forwarding
date: 2017-01-03 10:12:38
tags:
- Objective-C
---

## OC的message forwarding

众所周知，OC是一种基于消息的语言，这种特质带给这种语言很强的动态性。当运行时调用一个类或对象的某个功能时，在OC中是以“消息”的方式发送给一个实体（类或对象）。`message`发送给一个实体后，运行时会安装实体的内存模型去查找这个消息，但如果这个实体没有这个消息的处理函数是不是意味着结束呢？不，运行时还有一个称为`message forwarding`查询机制。
<!-- more -->
![Snip20170104_1](OC的message forwarding/Snip20170104_1.png)

啥都不说了🙊，上代码

```objective-c
#include <Foundation/Foundation.h>

@interface TestClass : NSObject

@end

@implementation TestClass

+ (BOOL)resolveInstanceMethod:(SEL)sel
{
    NSLog(@"resolveInstanceMethod: %s",sel);
    return [super resolveInstanceMethod:sel];
}

- (id)forwardingTargetForSelector:(SEL)aSelector
{
    NSLog(@"forwardingTargetForSelector: %s",aSelector);
    return nil;
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
    NSLog(@"forwardInvocation: %@",anInvocation);
    return [super forwardInvocation:anInvocation];
}

@end

int main()
{
    @autoreleasepool
    {
    	TestClass *aTestClass = [[TestClass alloc] init];
        objc_msgSend(aTestClass, @selector(didnotexistmethod:));
    }
    return 0;
}
```

运行输出如下：

```json
2017-01-04 11:44:25.743 unitTest.m_sublime_build[6876:645642] resolveInstanceMethod: didnotexistmethod:
2017-01-04 11:44:25.744 unitTest.m_sublime_build[6876:645642] forwardingTargetForSelector: didnotexistmethod:
2017-01-04 11:44:25.744 unitTest.m_sublime_build[6876:645642] resolveInstanceMethod: didnotexistmethod:
2017-01-04 11:44:25.744 unitTest.m_sublime_build[6876:645642] -[TestClass didnotexistmethod:]: unrecognized selector sent to instance 0x7f8f0be03830
2017-01-04 11:44:25.745 unitTest.m_sublime_build[6876:645642] *** Terminating app due to uncaught exception 'NSInvalidArgumentException', reason: '-[TestClass didnotexistmethod:]: unrecognized selector sent to instance 0x7f8f0be03830'
```

what? `resolveInstanceMethod` 被调用了两次，`forwardInvocation`没有被调用。额，原来是`forwardInvocation`被调用必须是`methodSignatureForSelector`方法可以返回其信息。那么`methodSignatureForSelector`有什么作用呢？我们看一下`methodSignatureForSelector`的原型，传入一个`SEL`返回`NSMethodSignature`

```objective-c
- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
```

看一下所谓方法签名到底是什么`NSMethodSignature`，难道是某种算法产生的摘要？不，如下有三个方法：

```objective-c
- (void)method1Test
{
	NSLog(@"method1Test");
}

- (NSInteger)method2Test
{
	NSLog(@"method2Test");
	return 1;
}

- (TestClass *)method3Test:(NSString*)str
{
	NSLog(@"参数:%@",str);
	return self;
}
```

`NSMethodSignature`的定义如下,可以从类方法`signatureWithObjCTypes`看出，这里传入字符串`types`就是所谓的签名：

```objective-c
@interface NSMethodSignature : NSObject {
@private
    void *_private;
    void *_reserved[6];
}

+ (nullable NSMethodSignature *)signatureWithObjCTypes:(const char *)types;

@property (readonly) NSUInteger numberOfArguments;
- (const char *)getArgumentTypeAtIndex:(NSUInteger)idx NS_RETURNS_INNER_POINTER;

@property (readonly) NSUInteger frameLength;

- (BOOL)isOneway;

@property (readonly) const char *methodReturnType NS_RETURNS_INNER_POINTER;
@property (readonly) NSUInteger methodReturnLength;

@end
```
写一个简单的函数可以从一个`NSMethodSignature`中还原出`type`

```objective-c
void whatIsMethodSignature(NSMethodSignature *methodSign)
{
	char sNSMethodSignature[10] = {0};
        sNSMethodSignature[0] = methodSign.methodReturnType[0];
        for (int i = 0; i < methodSign.numberOfArguments; ++i)
        {
        	sNSMethodSignature[i + 1] = [methodSign getArgumentTypeAtIndex:i][0];
        }
        NSLog(@"NSMethodSignature: %s",sNSMethodSignature);
}
```

最后得到上面三个函数的签名如下：

```json
2017-01-04 15:10:00.099 unitTest.m_sublime_build[7332:729268] NSMethodSignature: v@:
2017-01-04 15:10:00.099 unitTest.m_sublime_build[7332:729268] NSMethodSignature: q@:
2017-01-04 15:10:00.099 unitTest.m_sublime_build[7332:729268] NSMethodSignature: @@:@
```

对比上面三个函数的定义我们很快就能总结出 v=void   @=（NObject类，当然包括self） :=_cmd  q=NSInteger。没错就是这么变态。进而我们总结出函数的签名可以告诉编译器一个函数的结构信息，返回值类型，参数个数与类型。有了签名`runtime`才能构建出`anInvocation`传入给`forwardInvocation`。

```objective-c
SEL selector = @selector(method1Test);
NSMethodSignature *methodSign = [[self class] instanceMethodSignatureForSelector:selector];
NSInvocation *methodInvocation = [NSInvocation invocationWithMethodSignature:methodSign];
[methodInvocation setTarget:self];
[methodInvocation setSelector:selector];
[methodInvocation invoke];
```

```
#include <Foundation/Foundation.h>

@interface TestClass : NSObject

@end

@implementation TestClass

+ (BOOL)resolveInstanceMethod:(SEL)sel
{
    NSLog(@"resolveInstanceMethod: %s",sel);
    return [super resolveInstanceMethod:sel];
}

- (id)forwardingTargetForSelector:(SEL)aSelector
{
    NSLog(@"forwardingTargetForSelector: %s",aSelector);
    return nil;
}

- (NSMethodSignature *)methodSignatureForSelector:(SEL)selector
{
    return [NSMethodSignature signatureWithObjCTypes:"v@:"];

}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
    NSLog(@"forwardInvocation: %@",anInvocation);
}

@end

int main()
{
    @autoreleasepool
    {
    	TestClass *aTestClass = [[TestClass alloc] init];
        objc_msgSend(aTestClass, @selector(didnotexistmethod));
        
    }
    return 0;
}
```

输出如下信息，完整的message forwarding 流程就走了一遍。

```json
2017-01-04 15:39:12.808 unitTest.m_sublime_build[7432:753061] resolveInstanceMethod: didnotexistmethod
2017-01-04 15:39:12.808 unitTest.m_sublime_build[7432:753061] forwardingTargetForSelector: didnotexistmethod
2017-01-04 15:39:12.808 unitTest.m_sublime_build[7432:753061] methodSignatureForSelector: didnotexistmethod
2017-01-04 15:39:12.809 unitTest.m_sublime_build[7432:753061] forwardInvocation: <NSInvocation: 0x7fec2f400a70>
```

总结一下：

*  resolveInstanceMethod函数在运行时(runtime)，没有找到SEL的IML时就会执行。这个函数是给类利用class_addMethod添加函数的机会。如果实现了添加函数代码则返回YES，未实现返回NO。
*  forwardingTargetForSelector:系统给了个将这个SEL转给其他对象的机会。返回参数是一个对象，如果这个对象非nil、非self的话，系统会将运行的消息转发给这个对象执行。
*  methodSignatureForSelector:这个函数和后面的forwardInvocation:是最后一个寻找IML的机会。这个函数让重载方有机会抛出一个函数的签名，再由后面的forwardInvocation:去执行。
*  真正执行从methodSignatureForSelector:返回的NSMethodSignature。在这个函数里可以将NSInvocation多次转发到多个对象中，这也是这种方式灵活的地方。


