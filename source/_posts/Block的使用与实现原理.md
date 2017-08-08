---
title: Block的使用与实现原理
date: 2016-10-11 10:05:49
tags:
- Objective-C
---

## Block的使用与实现原理

[TOC]

## 什么是block？

Block是C语言的扩充，是一个`自动`包含`局部变量`的`匿名函数`。在C语言中所有的函数都要有名字，声明一个C函数：

```c
int fun(int par1);
```
<!-- more -->
使用C函数：

```c
int count = fun(2);
```

可以通过函数指针使用函数

```c
int (*funptr)(int) = &fun;
int count = (*funptr)(2);
```

block在相当程度上和函数是十分相似的，但是他是一个`匿名`的函数。

### block语法

我们知道了block是一个匿名的函数，除此之外block具有一个显著的特点就是block总有一个`^` ,我们看一个定义block的栗子。

```objective-c
^(int event){
  return event;
}
```

实际上它是下面一种写法的简化形势

```objective-c
^ int (int event){
  return event;
}
```

总结其定义形势为：`^` `返回值类型`( `参数列表` ){`表达式`}

其中：

- `返回值类型`与c语言返回值类型相同

- `参数列表`与c语言参数内部相同

- `表达式`当然也和c语言参数相同

- `表达式`中return返回的值的类型必须与`返回值类型`相同

- 我们刚刚已经看到了一种Block的缩写形式，但这不是唯一的缩写形式，Block有着各种各样的缩写，有时候简单的令人怀疑其语法的正确性：

  - 省略返回值:

    ```objective-c
    ^(int count){return count +1;}
    ```

    编译器此时根据return的返回值推断出block的返回类型。

  - 在没有参数的情况下可以把参数不错去除：

    ```objective-c
    ^{return 211314;}
    ```

#### block 类型变量

我们知道C/C++可以把函数的地址复制给函数指针，从而利用函数指针传递一个函数到另一个地方，与C函数相同，block同样有相应的block类型的变量。我们还知道一个整数数据可以用int类型描述，一个浮点数据可以用double或float等类型描述。那么一个Block由什么类型描述了？当然是block类型来描述了，但并不如此简单直白，准确的说有无数种Block类型，这些类型根据Block的返回值，参数互不相同。举个栗子：

声明一个名为`myFirstBlock`的`block类型`变量, 它可以用来描述返回值为整型具有一个整形参数的Block。

```objective-c
int (^myFirstBlock)(int par1);
```

再声明一个名为`mySecondBlock`的`Block类型`变量,它用来描述返回值为整形具有两个整形参数的Block。

```objective-c
int (^mySecondBlock)(int par1,int par2);
```

我们可以称`myFirstBlock` 与`mySecondBlock`为Block类型变量，但其实他们的类型并不相同。

我们还自然的看到，声明一个Block变量与声明一个C函数指针惊人的相似，区别只在于把`*`改为`^`。

下面我们试着给刚刚声明的两个变量赋值：

```objective-c
myFirstBlock = ^int(int evemt){return 1;}
mySecondBlock = ^(int a,int b){return 2;}
```

和其他任何类型的变量一样，Block类型变量可以充当函数局部变量、函数参数、静态变量、全局变量等。我想任何从C++、Java转过来的程序员都不喜欢objective-c的语法，特别是它奇怪的函数调用方式，通过函数返回一个Block可以让我们得到一个小小的安慰，举个例子：

```objective-c
#include <Foundation/Foundation.h>

@interface CalculatorManager : NSObject
-(int (^)(int,int))add;
@end
@implementation CalculatorManager
-(int (^)(int,int))add
{
	return [^(int a,int b){return a+b;} copy];
}
@end

int main(){
    @autoreleasepool{
    	CalculatorManager *manager = [[CalculatorManager alloc] init];
    	
    	int ret = manager.add(1,2);

    	NSLog(@"%d", ret);

    }
    return 0;
}
```

看到了`manager.add(1,2)`这样的调用方式我的泪水掉下来，oc的函数命名和调用方式大概永远也得不到我的任何好感，但我无能为力，好在有Block可以欺骗一下自己。但这种障眼法是如何起作用的呢？主要是：

- `manager.add`其实是调用一个叫add的属性的get方法，当然我们并没有定义什么add属性，但这并不影响它发送add消息。
- add消息的返回值是一个block，我们当然可以在它后面加上`(1,2)`使用这个block。于是就有了`manager.add(1,2)`这种优美的调用方式。
- 其实这个例子可以更进一步达到所谓连锁编程的效果，本文不做探讨了，有兴趣可以参看http://www.tuicool.com/articles/EfaYb2r。

即使我们队Block的声明与Block变量的定义语法有所了解，写出类似`-(int (^)(int,int))add;`这样的代码还是辣眼睛。于是c语言中typedef就被我们拎了出来。如：

```objective-c
typedef int (^blockType)(int);
```

### 截获自动变量

前面说Block是C语言的扩充，是一个`自动包含` 局部变量的`匿名函数`。通过上面的语法描述我比较能够理解其`匿名函数`的说法，但它的自动包含局部变量是声明意思呢？所谓自动包含是指在定义block时，Block会自动保存当前cpu栈帧的所有变量的值。举个栗子吧：

```objective-c
void fun(){
	int a = 10;
	int b = 20;
	void (^block)() = ^(){
		NSLog(@"%d   %d",a,b);
	};
	a = 30 ;
	block();//输出为：  10   20
}
```

block会在其定义的时候自动捕获它用到的变量`a b`,捕获后的变量保留着捕获那个时刻的值，所以在代码中即便后来`a = 30;`Block的输出还是`20`。

那么能不能再block中修改捕获的变量呢？如下：

```objective-c
void fun(){
	int a = 10;
	int b = 20;
	void (^block)() = ^(){
		a = 40;//编译器报告variable is not assignable (missing __block type specifier)
		NSLog(@"%d   %d",a,b);
	};
	a = 30 ;
	block();
}
```

答案是不行的，编译器会阻止你的行为并且报告`variable is not assignable (missing __block type specifier)`。但从报告中我们看到`__block`，是的如何你给`a`加上`__block`修饰后就可以改变其值。如下：

```objective-c
void fun(){
	__block int a = 10;
	int b = 20;
	void (^block)() = ^(){
		a = 40;
		NSLog(@"%d   %d",a,b);
	};
	a = 30 ;
	block();//输出为：  40   20
}
```

这里可能会发生一个误解，为了说明请看下面这个栗子，并说说这个栗子有无问题：

```objective-c
void fun2(){
	NSMutableArray * array = [[NSMutableArray alloc]init];
	void (^block)() = ^(){
		id object = [[NSObject alloc]init];
		[array addObject:object];
	};
}
```

上面的代码中`array`声明的时候没有加`__block`，在随后的block中使用了它，如果你觉得这有问题那么你发生了误解，在block中对array添加一个元素并无问题，被捕获的仅仅只是一个OC指针。

关于Block捕获变量还有一个令人费解的问题，如:

```objective-c
void fun3(){
	const char text[] = "hello world";
	void (^block)() = ^(){
      //编译报错，提示cannot refer to declaration with an array type inside block
		NSLog(@"%c",text[0]);
	};
}
```

根据刚刚所说的关于Block捕获变量的内容来看，上面的代码没有任何问题，但偏偏编译时会报错，原因是因为苹果公司实现block时没有让block具有获取c数组的能力，职位为什么苹果公司不这么做，我也不知道原因，希望有大神可以解答一下。当然如果真遇到上面的需求，解决起来到时十分方便，自动在block之前将数组的头指针赋值给一个指针变量就行了。如下：

```objective-c
void fun3(){
	const char text[] = "hello world";
	const char * ptr = text;
	void (^block)() = ^(){
		NSLog(@"%c",ptr[0]);
	};
}
```

## Block的实现原理

Block是C语言的扩充，是一个`自动`包含`局部变量`的`匿名函数`。含有Block语法的c或oc代码在编译时，LLVM编译器会先把它转换成为纯C++语法，我们可以通过LLVM编译指令让其保留转换成的中间C++文件，从而了解Block的实现原理。

```c
clang -rewrite-objc filename
```

我们试着将下面的oc代码翻译成C++

```objective-c
int main()
{
	void (^blk)(void) = ^{};
	blk();
	return 0;
}
```

执行`clang -rewrite-objc filename` 后得到：

```c++
#ifndef __OBJC2__
#define __OBJC2__
#endif
struct objc_selector; struct objc_class;
struct __rw_objc_super { 
	struct objc_object *object; 
	struct objc_object *superClass; 
	__rw_objc_super(struct objc_object *o, struct objc_object *s) : object(o), superClass(s) {} 
};
#ifndef _REWRITER_typedef_Protocol
typedef struct objc_object Protocol;
#define _REWRITER_typedef_Protocol
#endif
#define __OBJC_RW_DLLIMPORT extern
__OBJC_RW_DLLIMPORT void objc_msgSend(void);
__OBJC_RW_DLLIMPORT void objc_msgSendSuper(void);
__OBJC_RW_DLLIMPORT void objc_msgSend_stret(void);
__OBJC_RW_DLLIMPORT void objc_msgSendSuper_stret(void);
__OBJC_RW_DLLIMPORT void objc_msgSend_fpret(void);
__OBJC_RW_DLLIMPORT struct objc_class *objc_getClass(const char *);
__OBJC_RW_DLLIMPORT struct objc_class *class_getSuperclass(struct objc_class *);
__OBJC_RW_DLLIMPORT struct objc_class *objc_getMetaClass(const char *);
__OBJC_RW_DLLIMPORT void objc_exception_throw( struct objc_object *);
__OBJC_RW_DLLIMPORT int objc_sync_enter( struct objc_object *);
__OBJC_RW_DLLIMPORT int objc_sync_exit( struct objc_object *);
__OBJC_RW_DLLIMPORT Protocol *objc_getProtocol(const char *);
#ifdef _WIN64
typedef unsigned long long  _WIN_NSUInteger;
#else
typedef unsigned int _WIN_NSUInteger;
#endif
#ifndef __FASTENUMERATIONSTATE
struct __objcFastEnumerationState {
	unsigned long state;
	void **itemsPtr;
	unsigned long *mutationsPtr;
	unsigned long extra[5];
};
__OBJC_RW_DLLIMPORT void objc_enumerationMutation(struct objc_object *);
#define __FASTENUMERATIONSTATE
#endif
#ifndef __NSCONSTANTSTRINGIMPL
struct __NSConstantStringImpl {
  int *isa;
  int flags;
  char *str;
#if _WIN64
  long long length;
#else
  long length;
#endif
};
#ifdef CF_EXPORT_CONSTANT_STRING
extern "C" __declspec(dllexport) int __CFConstantStringClassReference[];
#else
__OBJC_RW_DLLIMPORT int __CFConstantStringClassReference[];
#endif
#define __NSCONSTANTSTRINGIMPL
#endif
#ifndef BLOCK_IMPL
#define BLOCK_IMPL
struct __block_impl {
  void *isa;
  int Flags;
  int Reserved;
  void *FuncPtr;
};
// Runtime copy/destroy helper functions (from Block_private.h)
#ifdef __OBJC_EXPORT_BLOCKS
extern "C" __declspec(dllexport) void _Block_object_assign(void *, const void *, const int);
extern "C" __declspec(dllexport) void _Block_object_dispose(const void *, const int);
extern "C" __declspec(dllexport) void *_NSConcreteGlobalBlock[32];
extern "C" __declspec(dllexport) void *_NSConcreteStackBlock[32];
#else
__OBJC_RW_DLLIMPORT void _Block_object_assign(void *, const void *, const int);
__OBJC_RW_DLLIMPORT void _Block_object_dispose(const void *, const int);
__OBJC_RW_DLLIMPORT void *_NSConcreteGlobalBlock[32];
__OBJC_RW_DLLIMPORT void *_NSConcreteStackBlock[32];
#endif
#endif
#define __block
#define __weak

#include <stdarg.h>
struct __NSContainer_literal {
  void * *arr;
  __NSContainer_literal (unsigned int count, ...) {
	va_list marker;
	va_start(marker, count);
	arr = new void *[count];
	for (unsigned i = 0; i < count; i++)
	  arr[i] = va_arg(marker, void *);
	va_end( marker );
  };
  ~__NSContainer_literal() {
	delete[] arr;
  }
};
extern "C" __declspec(dllimport) void * objc_autoreleasePoolPush(void);
extern "C" __declspec(dllimport) void objc_autoreleasePoolPop(void *);

struct __AtAutoreleasePool {
  __AtAutoreleasePool() {atautoreleasepoolobj = objc_autoreleasePoolPush();}
  ~__AtAutoreleasePool() {objc_autoreleasePoolPop(atautoreleasepoolobj);}
  void * atautoreleasepoolobj;
};

#define __OFFSETOFIVAR__(TYPE, MEMBER) ((long long) &((TYPE *)0)->MEMBER)
void printf(){

}


struct __main_block_impl_0 {
  struct __block_impl impl;
  struct __main_block_desc_0* Desc;
  __main_block_impl_0(void *fp, struct __main_block_desc_0 *desc, int flags=0) {
    impl.isa = &_NSConcreteStackBlock;
    impl.Flags = flags;
    impl.FuncPtr = fp;
    Desc = desc;
  }
};
static void __main_block_func_0(struct __main_block_impl_0 *__cself) {
printf();}

static struct __main_block_desc_0 {
  size_t reserved;
  size_t Block_size;
} __main_block_desc_0_DATA = { 0, sizeof(struct __main_block_impl_0)};
int main()
{
 void (*blk)(void) = ((void (*)())&__main_block_impl_0((void *)__main_block_func_0, &__main_block_desc_0_DATA));
 ((void (*)(__block_impl *))((__block_impl *)blk)->FuncPtr)((__block_impl *)blk);
 return 0;
}
static struct IMAGE_INFO { unsigned version; unsigned flag; } _OBJC_IMAGE_INFO = { 0, 2 };
```

说实话我真不想看它，管TM如何实现我会用不就行了，但紧接着工作中遇到的各种坑使我又有了想要了解他的动力。经过大概3分钟的删代码后，可以发现其实有用的代码只有下面这些：

```c++
struct __block_impl {
  void *isa;
  int Flags;
  int Reserved;
  void *FuncPtr;
};

struct __main_block_impl_0 {
  struct __block_impl impl;
  struct __main_block_desc_0* Desc;
  __main_block_impl_0(void *fp, struct __main_block_desc_0 *desc, int flags=0) {
    impl.isa = &_NSConcreteStackBlock;
    impl.Flags = flags;
    impl.FuncPtr = fp;
    Desc = desc;
  }
};
static void __main_block_func_0(struct __main_block_impl_0 *__cself) {
printf();}

static struct __main_block_desc_0 {
  size_t reserved;
  size_t Block_size;
} __main_block_desc_0_DATA = { 0, sizeof(struct __main_block_impl_0)};
int main()
{
 void (*blk)(void) = ((void (*)())&__main_block_impl_0((void *)__main_block_func_0, &__main_block_desc_0_DATA));
  
 ( (void (*)(__block_impl *)) ((__block_impl *)blk)->FuncPtr)((__block_impl *)blk);
  
 return 0;
}
```

这已经减少很多了，于是又有了分析下去的信心，然后再仔细看一下，我靠！我貌似看懂了。。。

首先找一下我们匿名的Block变成什么了，通过查找`printf();`我们发现程序中多了一个有名字的函数：

```c++
static void __main_block_func_0(struct __main_block_impl_0 *__cself) 
{
	printf();
}
```

我靠！ 它有名字了，是的，经过转换后所有Block都得到了一个名分，命名规则也很好掌握`__main_block_func_0`意为main 函数中第0个block函数。这个函数有一个类型为`__main_block_impl_0`的参数`__cself`这个self记录了当前Block在定义的时候CPU栈帧中变量的值和其他信息，具体如何要分析一下`__main_block_impl_0`。结构体`__main_block_impl_0`中第一个是`__block_impl`于是我们马上意识到`__block_impl`是前者的父类，将其强制类型转换没有任何压力，从代码上看确实有大量的强制类型转换。那么看看`__main_block_impl_0`的两个部分`__block_impl`和`__main_block_desc_0`

```c++
struct __block_impl {
  void *isa;
  int Flags;
  int Reserved;
  void *FuncPtr;
};
```

```objective-c
struct __main_block_desc_0 {
  size_t reserved;
  size_t Block_size;
}
```

这些就是一个Block的所有数据结构了，我们其实可以望文生义猜猜其变量的作用，首先映入眼帘的就是isa，熟知objective-c的我们立即就猜到，尼玛这货不就是一个oc对象吗，是的`__block_impl`就是一个oc对象，也就是说Block就是一个oc对象。Block对象的类型是什么呢，这要看isa指向了谁，我们看代码知道最后isa是由`&_NSConcreteStackBlock`赋值，但这货是神马我就没追究了，求大神告知。

下面看看我们最初的代码到底如何转换成这些数据结构之间的调用。我们的最初代码只有两行，第一行：

```c
void (^blk)(void) = ^{};
```

转换后变为：

```c++
void (*blk)(void) = ((void (*)())&__main_block_impl_0((void *)__main_block_func_0, &__main_block_desc_0_DATA));
```

上面代码有好多强制类型转换，我去除它，简化后：

```c
blk = &__main_block_impl_0(__main_block_func_0,&__main_block_desc_0_DATA)
```

由此我们知道blk被转换成一个指向`__main_block_impl_0`结构体，并且结构体在初始化时把函数指针和DATA传入了。

同样的，最初代码的第二行：

```c
blk();
```

转换后变为：

```c
 ( (void (*)(__block_impl *)) ((__block_impl *)blk)->FuncPtr)((__block_impl *)blk);
```

简化后：

```c
(*blk->FuncPtr)(blk);
```

十分明了对吧。

### 截获自动变量的机制

通过上面的分析我们大概明白了block是个神马东东，也自动代码最后转换成什么样子，以及他们之间是怎么调用生效的。现在只剩下如何捕获自动变量，以及自动变量怎么使用的。

同样通过一个栗子来说明：

```objective-c
void add(int par1,int par2){
   int tmp = par1+par2;
}
int main()
{
	int a = 10;
	int b = 20;
	void(^blk)() = ^void (){
		add(a,b);
	};
	blk();
	return 0;
}
```

用`clang -rewrite-objc filename`翻译得到：

```c++
struct __block_impl {
  void *isa;
  int Flags;
  int Reserved;
  void *FuncPtr;
};


struct __main_block_impl_0 {
  struct __block_impl impl;
  struct __main_block_desc_0* Desc;
  int a;
  int b;
  __main_block_impl_0(void *fp, struct __main_block_desc_0 *desc, int _a, int _b, int flags=0) : a(_a), b(_b) {
    impl.isa = &_NSConcreteStackBlock;
    impl.Flags = flags;
    impl.FuncPtr = fp;
    Desc = desc;
  }
};
static void __main_block_func_0(struct __main_block_impl_0 *__cself) {
  int a = __cself->a; // bound by copy
  int b = __cself->b; // bound by copy

  add(a,b);
 }

static struct __main_block_desc_0 {
  size_t reserved;
  size_t Block_size;
} __main_block_desc_0_DATA = { 0, sizeof(struct __main_block_impl_0)};
int main()
{
 int a = 10;
 int b = 20;

 void(*blk)() = ((void (*)())&__main_block_impl_0((void *)__main_block_func_0, &__main_block_desc_0_DATA, a, b));
 ((void (*)(__block_impl *))((__block_impl *)blk)->FuncPtr)((__block_impl *)blk);

 return 0;
}
```

仔细看一下就觉得不可思议的简单，就是给`__main_block_impl_0`增加两个变量，并且在初始化的时候把当前值传入`__main_block_impl_0`的初始化函数中。在`__main_block_func_0`使用时就可以从self中自己取出来了。原来所谓自动捕获就是赋值呀，擦，要不要这么吓唬人！！如何我们不考虑其他的细节，Block也就如此而已。but，现实是，其他的细节恐怖的让人绝望。下面来分析一些。

#### __block说明符

我们将刚刚的栗子稍微修改一下：

```objective-c
void add(int par1,int par2){
   int tmp = par1+par2;
}
int main()
{
	__block int a = 10;
	int b = 20;
	void(^blk)() = ^void (){
		add(a,b);
	};
	a = 30;
	blk();
	return 0;
}
```

实际上上面的这样需求十分常见，想要在Block中修改变量在Block外面也能使用。这是一个传值的利器，我们来看看编译器做了什么。同样使用`clang -rewrite-objc filename`：

```objective-c
struct __block_impl {
  void *isa;
  int Flags;
  int Reserved;
  void *FuncPtr;
};
struct __Block_byref_a_0 {
  void *__isa;
__Block_byref_a_0 *__forwarding;
 int __flags;
 int __size;
 int a;
};

struct __main_block_impl_0 {
  struct __block_impl impl;
  struct __main_block_desc_0* Desc;
  int b;
  __Block_byref_a_0 *a; // by ref
  __main_block_impl_0(void *fp, struct __main_block_desc_0 *desc, int _b, __Block_byref_a_0 *_a, int flags=0) : b(_b), a(_a->__forwarding) {
    impl.isa = &_NSConcreteStackBlock;
    impl.Flags = flags;
    impl.FuncPtr = fp;
    Desc = desc;
  }
};
static void __main_block_func_0(struct __main_block_impl_0 *__cself) {
  __Block_byref_a_0 *a = __cself->a; // bound by ref
  int b = __cself->b; // bound by copy

  add((a->__forwarding->a),b);
 }
static void __main_block_copy_0(struct __main_block_impl_0*dst, struct __main_block_impl_0*src) {_Block_object_assign((void*)&dst->a, (void*)src->a, 8/*BLOCK_FIELD_IS_BYREF*/);}

static void __main_block_dispose_0(struct __main_block_impl_0*src) {_Block_object_dispose((void*)src->a, 8/*BLOCK_FIELD_IS_BYREF*/);}

static struct __main_block_desc_0 {
  size_t reserved;
  size_t Block_size;
  void (*copy)(struct __main_block_impl_0*, struct __main_block_impl_0*);
  void (*dispose)(struct __main_block_impl_0*);
} __main_block_desc_0_DATA = { 0, sizeof(struct __main_block_impl_0), __main_block_copy_0, __main_block_dispose_0};
int main()
{
 __attribute__((__blocks__(byref))) __Block_byref_a_0 a = {(void*)0,(__Block_byref_a_0 *)&a, 0, sizeof(__Block_byref_a_0), 10};
 int b = 20;

 void(*blk)() = ((void (*)())&__main_block_impl_0((void *)__main_block_func_0, &__main_block_desc_0_DATA, b, (__Block_byref_a_0 *)&a, 570425344));
 (a.__forwarding->a) = 30;
 ((void (*)(__block_impl *))((__block_impl *)blk)->FuncPtr)((__block_impl *)blk);

 return 0;
}
```

仅仅只是加了一个`__block`就导致代码有很大的增加，具体来说就是原来的`int a`变成了一个`__Block_byref_a_0`。这样就可以把时间变量放入结构体中从而达到修改可以保留的目的。

#### Block存储域

回顾一下刚刚写的内容，block被转换为一个结构体局部变量，__block 修饰的临时变量也被转换为一个结构体局部变量。Block同时也是一个objective-c对象，它的类型为` _NSConcreteStackBlock`。但我们还不知道` _NSConcreteStackBlock`是什么，实际上通过查看源码我们知道类似的类型还有：

- _NSConcreteStackBlock
- _NSConcreteGlobalBlock
- _NSConcreteMallocBlock

通过单词中的stack、global、malloc，我们可以猜测：

| block类                 | 存储位置  |
| ---------------------- | ----- |
| _NSConcreteStackBlock  | 栈     |
| _NSConcreteGlobalBlock | 程序数据区 |
| _NSConcreteMallocBlock | 堆     |

我们观察下面的代码：

```objective-c
void print(){}

void(^blk)() = ^void (){print();};

int main()
{
	
	blk();
	return 0;
}
```

转换后的代码为：

```objective-c
struct __blk_block_impl_0 {
  struct __block_impl impl;
  struct __blk_block_desc_0* Desc;
  __blk_block_impl_0(void *fp, struct __blk_block_desc_0 *desc, int flags=0) {
    impl.isa = &_NSConcreteGlobalBlock;
    impl.Flags = flags;
    impl.FuncPtr = fp;
    Desc = desc;
  }
};
static void __blk_block_func_0(struct __blk_block_impl_0 *__cself) {
print();}

static struct __blk_block_desc_0 {
  size_t reserved;
  size_t Block_size;
} __blk_block_desc_0_DATA = { 0, sizeof(struct __blk_block_impl_0)};
static __blk_block_impl_0 __global_blk_block_impl_0((void *)__blk_block_func_0, &__blk_block_desc_0_DATA);
void(*blk)() = ((void (*)())&__global_blk_block_impl_0);

int main()
{

 ((void (*)(__block_impl *))((__block_impl *)blk)->FuncPtr)((__block_impl *)blk);
 return 0;
}
```

从上面看到通过什么全局变量blk，转换后的Block对象类型是` _NSConcreteGlobalBlock` ，并用static声明表示存储在数据区。不存在捕获变量的问题。

那么现在还有一个` _NSConcreteMallocBlock`悬而未决，我们不着急用`clang -rewrite-objc filename` 生产一个栗子。我们先考虑一个问题：局部的Block在超出其作用域后会如何

> 类型为` _NSConcreteGlobalBlock`的block，也就是全局Block变量，在定义后的任何地方都可以使用。但案例栈上的Block，如果其所属的变量作用域结束，该Block也就被废弃了。同样由`__block`修饰的变量在超出作用域的时候也会被废弃.

如果想在超出Block定义范围以外的地方使用Block就必须先把Block赋值到堆上，再把指针传过去供别人使用。回忆一下，一般是函数参数，函数返回值，赋值类的Block变量，这些地方是符合`超出Block定义范围以外的地方使用Block`。

那为题来了，我们如何把一个Block赋值的推上呢？复杂吗？平时使用他没有感觉到赋值到堆上的操作呀。这是因为一切都是自动的，或者手动很简单就赋值好了。

原因是因为在大多数情况下（函数参数，函数返回值，赋值类的Block变量等），编译器会判断我们很可能在作用域以外的地方还要使用这个Block，因此自动的帮我们赋值到堆上去了，并且把指针传出。

 很遗憾没能获得中间自动转换后的代码，可以参考http://www.jianshu.com/p/51d04b7639f1 进行理解

> 但一个block从栈上复制到堆上后，使用__block修饰的变量也会发生变化。变量会复制到堆上并被Block持有。

现在可以讨论一下刚刚遇到的由__block修饰的变量生成的结构体。

```c++
struct __Block_byref_a_0 {
  void *__isa;
__Block_byref_a_0 *__forwarding;
 int __flags;
 int __size;
 int a;
};
```

回顾之前代码，发生所有访问`a`的地方都是通过`__forwarding`指针进行，并且这个指针指向自己。之所有这么设计是因为当 _block修饰的变量复制到堆上后，要保证栈上的变量和堆上的变量指向同一个地方。这时候`__forwarding`就起作用了。`__block`修饰的变量在赋值到堆上时，大概的步骤是：

- malloc 创建内存区
- memcopy赋值
- 修改原对象的`__forwarding`指针指向目标对象

为了验证，我们可以修改之前代码，加上`copy`让Block赋值到堆上：

```objective-c
void add(int par1,int par2){
   int tmp = par1+par2;
}
int main()
{
	__block int a = 10;
	int b = 20;
	void(^blk)() = [^void (){
		add(a,b);
	} copy];
	a = 30;
	blk();
	return 0;
}
```

转换后：

```c++
void add(int par1,int par2){
   int tmp = par1+par2;
}
struct __Block_byref_a_0 {
  void *__isa;
__Block_byref_a_0 *__forwarding;
 int __flags;
 int __size;
 int a;
};

struct __main_block_impl_0 {
  struct __block_impl impl;
  struct __main_block_desc_0* Desc;
  int b;
  __Block_byref_a_0 *a; // by ref
  __main_block_impl_0(void *fp, struct __main_block_desc_0 *desc, int _b, __Block_byref_a_0 *_a, int flags=0) : b(_b), a(_a->__forwarding) {
    impl.isa = &_NSConcreteStackBlock;
    impl.Flags = flags;
    impl.FuncPtr = fp;
    Desc = desc;
  }
};
static void __main_block_func_0(struct __main_block_impl_0 *__cself) {
  __Block_byref_a_0 *a = __cself->a; // bound by ref
  int b = __cself->b; // bound by copy

  add((a->__forwarding->a),b);
 }
static void __main_block_copy_0(struct __main_block_impl_0*dst, struct __main_block_impl_0*src) {_Block_object_assign((void*)&dst->a, (void*)src->a, 8/*BLOCK_FIELD_IS_BYREF*/);}

static void __main_block_dispose_0(struct __main_block_impl_0*src) {_Block_object_dispose((void*)src->a, 8/*BLOCK_FIELD_IS_BYREF*/);}

static struct __main_block_desc_0 {
  size_t reserved;
  size_t Block_size;
  void (*copy)(struct __main_block_impl_0*, struct __main_block_impl_0*);
  void (*dispose)(struct __main_block_impl_0*);
} __main_block_desc_0_DATA = { 0, sizeof(struct __main_block_impl_0), __main_block_copy_0, __main_block_dispose_0};
int main()
{
 __attribute__((__blocks__(byref))) __Block_byref_a_0 a = {(void*)0,(__Block_byref_a_0 *)&a, 0, sizeof(__Block_byref_a_0), 10};
 int b = 20;
 void(*blk)() = (void (*)())((id (*)(id, SEL, ...))(void *)objc_msgSend)((id)((void (*)())&__main_block_impl_0((void *)__main_block_func_0, &__main_block_desc_0_DATA, b, (__Block_byref_a_0 *)&a, 570425344)), sel_registerName("copy"));
 (a.__forwarding->a) = 30;
 ((void (*)(__block_impl *))((__block_impl *)blk)->FuncPtr)((__block_impl *)blk);
 return 0;
}
```

 其中`__main_block_copy_0`会被Block对象在调用copy时调用。真正起作用的是`_Block_object_assign((void*)&dst->a, (void*)src->a, 8);`，这个函数的源码在http://llvm.org/svn/llvm-project/compiler-rt/trunk/lib/BlocksRuntime/runtime.c 中，代码很直白如下：

```c++
/*
 * When Blocks or Block_byrefs hold objects then their copy routine helpers use this entry point
 * to do the assignment.
 */
void _Block_object_assign(void *destAddr, const void *object, const int flags) {
    //printf("_Block_object_assign(*%p, %p, %x)\n", destAddr, object, flags);
    if ((flags & BLOCK_BYREF_CALLER) == BLOCK_BYREF_CALLER) {
        if ((flags & BLOCK_FIELD_IS_WEAK) == BLOCK_FIELD_IS_WEAK) {
            _Block_assign_weak(object, destAddr);
        }
        else {
            // do *not* retain or *copy* __block variables whatever they are
            _Block_assign((void *)object, destAddr);
        }
    }
    else if ((flags & BLOCK_FIELD_IS_BYREF) == BLOCK_FIELD_IS_BYREF)  {
        // copying a __block reference from the stack Block to the heap
        // flags will indicate if it holds a __weak reference and needs a special isa
        _Block_byref_assign_copy(destAddr, object, flags);
    }
    // (this test must be before next one)
    else if ((flags & BLOCK_FIELD_IS_BLOCK) == BLOCK_FIELD_IS_BLOCK) {
        // copying a Block declared variable from the stack Block to the heap
        _Block_assign(_Block_copy_internal(object, flags), destAddr);
    }
    // (this test must be after previous one)
    else if ((flags & BLOCK_FIELD_IS_OBJECT) == BLOCK_FIELD_IS_OBJECT) {
        //printf("retaining object at %p\n", object);
        _Block_retain_object(object);
        //printf("done retaining object at %p\n", object);
        _Block_assign((void *)object, destAddr);
    }
}
```

这里说明一下，声明时候Block会被复制到堆上：

- 调用Block的copy实例方法
- Block作为函数的返回值时
- 将Block赋值给带有`__strong`修饰符修饰的（id类型或Block类型）成员变量时