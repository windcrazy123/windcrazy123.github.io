---
layout: post
title: "虚函数"
date: 2021-10-21 09:00:00 +0800 
categories: CPP
tag: CPP
---
* content
{:toc #markdown-toc}


CPP中虚函数的一些知识
<!-- more -->

# 虚函数的作用

实现类的多态性

如果函数使用了**virtual**关键字，程序将根据引用或指针指向的 **对象类型** 来选择此方法，否则使用 **引用类型** 或 **指针类型** 来选择方法。

# 虚函数的一些标识符

## override

这个标识符告诉编译器这是重写的方法，如果方法不匹配，那么将无法通过编译。不写也可以，但是如果函数签名不一样那么会认为这是一个新的函数。

## final

如果你不想派生类重写基类的虚方法，此时可以使用`final`标识符，这个时候如果派生类重写了基类虚方法，那么将无法编译

```c++
class A
{
public:
    virtual void someMethod() { cout << "A" << endl; }
}

class B: public A
{
public:
    // 基类A的someMethod方法没有final标识符，那么B可以重写该方法
    // 但是此虚方法使用了final标识符，后面的派生类无法重写
    virtual void someMethod() override final { cout << "B" << endl; }
}

class C: public B
{
public:
    // 无法编译，因为不允许重写
    virtual void someMethod() override { cout << "C" << endl; }
}
```
final标识符还可以直接用于类，此时该类将不能被继承：
```c++
class A
{
public:
    virtual void someMethod() { cout << "A" << endl; }
};

// B可以继承A
class B final: public A
{
public:
    virtual void someMethod() override { cout << "B" << endl; }
};

// B无法被继承，此时无法编译
class C: public B
{
public:
    virtual void someMethod() override { cout << "C" << endl; }
};
```

# 协变返回类型

前面说过，要想成功重写方法，基类虚方法与派生类虚方法必须匹配，其中返回类型也必须一致。但是有时候返回类型不相同，也能实现重写，此时返回类型存在继承关系：基类方法返回类型是一个指向某一类的指针或者引用，而派生类重写版本的返回类型是指向派生类的指针或者引用。这种情况称为协变返回类型。下面是一个例子：

```cpp
class Super
{
public:
    virtual Super* getThis() { return this; }
};

class Sub : public Super
{
    virtual Sub* getThis() override { return this; }
};
```

# 纯虚函数与抽象基类

有时候，基类的某个虚方法并不需要实现，但是希望派生类能够提供重写的版本。这个时候，你需要定义纯虚函数。纯虚函数在类的定义中显示说明该方法不需要实现，其作用在于指明派生类必须要重写它。纯虚函数的定义很简单：方法声明后紧跟着=0。如果一个类中至少含有一个纯虚函数，那么这个类是抽象基类，因为这个类无法实例化。（一般接口类会有这样的操作）当继承一个抽象类时，必须重写所有纯虚函数，否则继承出来的类也是一个抽象类。

# 虚基类

虚基类主要是用来解决菱形层次结构中的歧义基类问题。解决这个错误的方法很多，比如你可以在Copier类中明确声明继承的版本：`using A::Parent::Func;`。但是这本质上没有解决多版本的继承问题。

此时，你可以用虚基类，使用虚基类，只需要在继承列表中加上`virtual`关键字：

```cpp
class PoweredDevice
{
public:
    PoweredDevice(int power)
    {
        cout << "PoweredDevice: " << power << endl;
    }

    virtual void reportError() { cout << "Error" << endl; }
};

class Scanner : virtual public PoweredDevice
{
public:
    Scanner(int scanner, int power) :
        PoweredDevice(power)
    {
        cout << "Scanner: " << scanner << endl;
    }
};

class Printer : virtual public PoweredDevice
{
public:
    Printer(int printer, int power) :
        PoweredDevice(power)
    {
        cout << "Printer: " << printer << endl;
    }
};

class Copier : public Scanner, public Printer
{
public:
    // Note: 虚基类是由派生最远的类负责创建，所以，
    //       构造函数初始化列表中需要增加虚基类的构造函数调用
    Copier(int scanner, int printer, int power):
        Scanner(scanner, power), Printer(printer, power),
        PoweredDevice(power)//这一行在普通继承中是没有的，普通继承一般只需写父类构造而基类构造由父类调用
    {}

};
int main()
{
    Copier copier(1, 2, 3);

    // 合法
    copier.reportError(); 

    // output:
    // PoweredDevice: 3
    // Scanner : 1
    // Printer : 2
    // 可以看到PoweredDevice继承了一次

    return 0;
}
```
利用虚基类，可以解决上面多重继承中歧义基类问题，基类仅被继承一次。但是要注意的是此时的虚基类由派生最远的类负责创建（可以看成该类的直接基类），因为`PoweredDevice`并没有无参构造函数，所以在Copier构造函数初始化列表中必须加上`PoweredDevice`的有参构造函数调用！

# 使用注意

注意千万不要在构造函数与析构函数中调用虚函数。我们知道派生类对象在创建时，首先基类部分先被创建，如果你在基类构造函数调用虚函数时，它此时将无法调用派生类版本的函数，因为派生类对象还未创建，此时派生类虚函数没有作用的对象。那么，它只能调用基类版本的虚函数。对于析构函数，派生类对象中的派生部分先被析构，如果你在基类析构函数中调用了虚函数，它也只能调用基类版本的虚函数，因为派生类对象已经不存在了。

使用虚函数是有代价的，相对于普通函数，虚函数的调用代价稍高，但是这种差别不会太大。

友元函数不能为虚，因为友元函数不是类成员，只有类成员才能是虚函数。

## 使用虚函数后的变化

（1） 对象将增加一个存储地址的空间（32位系统为4字节，64位为8字节）。
（2） 每个类编译器都创建一个虚函数地址表
（3） 对每个函数调用都需要增加在表中查找地址的操作。

## 析构函数要声明为虚函数

对于析构函数，大部分时间我们只需要使用编译器提供的默认版本就好，除非涉及到释放动态分配的内存。但是如果存在继承，虚函数最好声明为虚函数。否则删除一个实际指向派生类的基类指针，只会调用基类的析构函数，而不会调用派生类的析构函数以及派生类数据成员的析构函数，这样就可能造成内存泄露。

```cpp
class Resource
{
public:
    Resource() { cout << "Resource created!" << endl; }
    ~Resource() { cout << "Resource destoryed!" << endl; }
};

class Super
{
public:
    Super() { cout << "Super constructor called!" << endl; }
    ~Super() { cout << "Super destructor called!" << endl; }
};

class Sub : public Super
{
public:
    Sub() { cout << "Sub constructor called!" << endl;}

    ~Sub() { cout << "Sub destructor called!" << endl；}
private:
    Resource res;
};
```

如果执行下面的代码：

```cpp
int main()
{
    Sub* sub = new Sub;
    Super* super = sub;
    delete super;

    cin.ignore(10);
        return 0;
}
```

其输出为：

```cpp
Super constructor called!
Resource created!
Sub constructor called!
Super destructor called!
```

可以看到，派生类的析构函数没有执行，其数据成员Resource也没有被析构。但是如果你将析构函数都声明为虚函数，上面的代码将得到如下的结果：

```cpp
Super constructor called!
Resource created!
Sub constructor called!
Resource destoryed!
Sub destructor called!
Super destructor called!
```

此时，程序按照预期输出，所以，对于继承问题，没有理由不将析构函数声明为虚函数！

## 对象切片

使用引用或者指针的方式，多态性都能够实现，但是传值的方式就存在问题。当我们将一个派生类对象直接赋值给基类对象时，仅仅基类的部分被复制，派生类的那部分信息将丢失。我们称这种现象为“对象切片”：对象丢失了自己原有的部分信息。使用对象本身并没有问题，但是处理不当，会造成很多问题，看下面的例子：

```cpp
int main()
{
    Derived d1{5};
    Derived d2{2};
    Base& b = d2;
    b = d1;   // 有隐患
    return 0；
}
```

上面的例子很简单，但是会有问题：首先d2引用给b时，b将指向d2，这没有问题。但是将d1的值直接赋值给b时，会发生对象切片，只有d1的基类部分复制给b。此时，问题来了，你会发现现在d2拥有d1的基类部分与d2的派生部分，这显得很混乱！所以，尽可能地别使用对象切片，否则你会麻烦不断！

## 动态转型(dynamic_cast)

看下面的例子：

```cpp
void process(Base* ptr)
{
    Derived* derived = static_cast<Derived*>(ptr);
    // 后序处理
    // ...
}
```

process函数接收一个基类指针，但是在内部使用`static_cast`向下转型为派生类指针，然后进行后序处理。如果送入process函数的指针实际上就是指向派生类对象，那么上面的代码是没有问题的。但是，如果仅仅传入就是指向基类对象的指针，或者指向其他派生类的指针，那么函数内部的转型将存在问题：由于`static_cast`在运行时是不检查对象实际类型的，这将导致不可控行为！

为了解决这样的隐患，C++引入了运行时的动态类型转化操作符`dynamic_cast`。`dynamic_cast`在运行时检测底层对象的类型信息。如果类型转换没有意义，那么它将返回一个空指针（对于指针类型）或者抛出一个`std::bad_cast`异常（对于引用类型）。所以，可以修改上面的代码如下：

```cpp
void process(Base* ptr)
{
    Derived* derived = dynamic_cast<Derived*>(ptr);
    if (derived == nullptr)
    {
        // 后序处理
        // ...
    }
}
```

尽管如此，向下转型还是不推荐的，除非必要！

# 虚函数实现机制

## 函数调用捆绑

要想深刻理解虚函数机理，首先要了解函数调用捆绑机制。捆绑指的是将标识符（如变量名与函数名）转化为地址。这里我们仅仅关注有关函数调用的捆绑。我们知道每个函数在编译的过程中是存在一个唯一的地址的。如果我们在程序段里面直接调用某个函数，那么编译器或者链接器会直接将函数标识符替换为一个机器地址。这种方式是早捆绑，或者说是静态捆绑。因为捆绑是在程序运行之前完成的。看下面的简单例子：

```cpp
int add(int x, int y)
{
    return x + y;
}

int subtract(int x, int y)
{
    return x - y;
}

int multiply(int x, int y)
{
    return x * y;
}

int main()
{
    int x;
    cout << "Enter a number: ";
    cin >> x;

    int y;
    cout << "Enter another number: ";
    cin >> y;

    int op;
    cout << "Enter an operation (0=add, 1=subtract, 2=multiply): ";
    cin >> op;

    int result;
    switch (op)
    {
        // 使用早绑定来直接调用函数
        case 0: result = add(x, y); break;
        case 1: result = subtract(x, y); break;
        case 2: result = multiply(x, y); break;
    }

    cout << "The answer is: " << result << endl;

        return 0;
}
```

由于上面三个函数的调用都是直接使用函数名，采用早捆绑的方式。编译器会将每个函数调用替换为一个跳转指令，这个指令告诉CPU跳转到函数的地址来执行。

但是有时候，我们在程序运行前并不知道调用哪个函数，此时必须使用晚捆绑或者动态捆绑。晚绑定的一个例子就是使用函数指针，修改上面的例子：

```cpp
int main()
{
    int x;
    cout << "Enter a number: ";
    cin >> x;

    int y;
    cout << "Enter another number: ";
    cin >> y;

    int op;
    cout << "Enter an operation (0=add, 1=subtract, 2=multiply): ";
    cin >> op;

    // 定义一个函数指针
    int(*opFun)(int, int) = nullptr;
    switch (op)
    {
        // 使用早捆绑来直接调用函数
        case 0: opFun = add; break;
        case 1: opFun = subtract; break;
        case 2: opFun = multiply; break;
    }

    // 通过函数指针来调用，只能是晚捆绑
    cout << "The answer is: " << opFun(x, y) << endl;

        return 0;
}
```

使用函数指针来间接调用函数，编译器在编译阶段并不知道函数指针到底指向哪个函数，所以必须使用动态捆绑的方式。

动态绑定看起来更灵活，但是其是有代价的。静态捆绑时，CUP可以直接跳转到函数地址。但是动态捆绑，CPU必须先提取指针的地址，然后再跳转到指向的函数地址。这多了一个步骤！

## 虚函数表（vtable）

C++使用了一种称为“虚表”的晚捆绑技术来实现虚函数。虚表是一个函数查询表，以动态捆绑的方式解析函数调用。每个具有一个或者多个虚函数的类都有一张虚表，这个表是在编译阶段建立的静态数组，其中包含了每个虚方法的函数指针，这些指针指向的是该类可见的派生最远的函数实现。编译器处理虚函数的方法是：给每个对象添加一个指针，存放了指向虚函数表的地址，虚函数表存储了为类对象进行声明的虚函数地址。，即，**每个类使用一个虚函数表，每个类对象用一个虚表指针。**举个例子：基类对象包含一个虚表指针，指向基类中所有虚函数的地址表。派生类对象也将包含一个虚表指针，指向派生类虚函数表。

看下面两种情况：

- 如果派生类重写了基类的虚方法，该派生类虚函数表将保存重写的虚函数的地址，而不是基类的虚函数地址。

- 如果基类中的虚方法没有在派生类中重写，那么派生类将继承基类中的虚方法，而且派生类中虚函数表将保存基类中未被重写的虚函数的地址。注意，如果派生类中定义了新的虚方法，则该虚函数的地址也将被添加到派生类虚函数表中。

![](..\..\styles\images\CPP\VirtualPtr.png)

# 函数导出

## 导出类的简单方式

只需要在导出类加上*__declspec(dllexport)*，就可以实现导出类。对象空间还是在使用者的模块里，*dll*只提供类中的函数代码。不足的地方是：使用者需要知道整个类的实现，包括基类、类中成员对象，也就是说所有跟导出类相关的东西，使用者都要知道。这时候的*dll*导出的是跟类相关的函数：如构造函数、赋值操作符、析构函数、其它函数，这些都是使用者可能会用到的函数。这种导出类的方式，除了导出的东西太多、使用者对类的实现依赖太多之外，还有其它问题：必须保证使用同一种编译器。导出类的本质是导出类里的函数，因为语法上直接导出了类，没有对函数的调用方式、重命名进行设置，导致了产生的*dll*并不通用。

## 通过虚函数进行导出

​     结构是这样的：导出类是一个派生类，派生自一个抽象类*——*都是纯虚函数。使用者需要知道这个抽象类的结构。*DLL*最少只需要提供一个用于获取类对象指针的接口。使用者跟*DLL*提供者共用一个抽象类的头文件，使用者依赖于*DLL*的东西很少，只需要知道抽象类的接口，以及获取对象指针的导出函数，对象内存空间的申请是在*DLL*模块中做的，释放也在*DLL*模块中完成*,*最后记得要调用释放对象的函数。

​     这种方式比较好，通用，产生的*DLL*没有特定环境限制。借助了*C++*类的虚函数。一般都是采用这种方式。除了对*DLL*导出类有好处外，采用接口跟实现分离，可以使得工程的结构更清晰，使用者只需要知道接口，而不需要知道实现。

　　部分代码：

（1）DLL头文件：

```c++
//dll导出类
//dll跟其使用者共用的头文件 //MatureApproach.h
#pragma  once
#ifdef MATUREAPPROACH_EXPORTS
#define MATUREAPPROACH_API __declspec(dllexport)
#else
#define MATUREAPPROACH_API __declspec(dllimport)
#endif

class IExport
{
public:
    virtual void Hi() = 0;
    virtual void Test() = 0;
    virtual void Release() = 0;
};

extern "C" MATUREAPPROACH_API IExport* _stdcall CreateExportObj();//构造函数导出
extern "C" MATUREAPPROACH_API void _stdcall DestroyExportObj(IExport* pExport);//析构函数导出
```

（2）导出类头文件：

```c++
//dll导出类
// 实现类
#pragma once

#include "MatureApproach.h"

class ExportImpl : public IExport
{
public:
    virtual void Hi();
    virtual void Test();
    virtual void Release();
    ~ExportImpl();
private:
};
```

这是利用**虚函数与动态绑定**机制，虚函数通过虚表（vtable）动态绑定到实现，**即使未链接导入库**，只要满足以下条件，程序可能在运行时加载DLL并调用函数：

- 首先使用 `LoadLibrary` 和 `GetProcAddress` 手动加载DLL。然后通过工厂函数创建对象（导出工厂函数）。

示例：

```c++
// 手动加载DLL（无需链接.lib）
HMODULE hDll = LoadLibrary(TEXT("MyDLL.dll"));
if (hDll) {
    // 获取工厂函数地址
    typedef MyClass* (*CreateObjFunc)();
    CreateObjFunc createObj = (CreateObjFunc)GetProcAddress(hDll, "CreateObject");
    
    // 创建对象并调用函数
    MyClass* pObj = createObj();
    pObj->MyFunction(); // 需确保MyFunction是虚函数
}
```

# 参考链接

> 1、[C++中虚函数的作用和虚函数的工作原理](https://www.cnblogs.com/zkfopen/p/11061414.html)
>
> 2、[Pointers and references to the base class of derived objects](https://www.learncpp.com/cpp-tutorial/pointers-and-references-to-the-base-class-of-derived-objects/)
>
> 3、[C++虚函数](https://zhuanlan.zhihu.com/p/54145222)
