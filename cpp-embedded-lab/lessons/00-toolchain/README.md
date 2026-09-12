# 第 00 课 · 工具链：代码是怎么变成可执行文件的

> 这一课不用写 TODO，但要**动手跑**。目标是让你对"编译"这件事不再黑箱。
> 嵌入式最恶心的问题（重复定义、找不到符号、链接顺序、头文件互相包含）
> 全都出在这一课讲的东西上。

---

## 1. 四个阶段

```
main.cpp  counter.cpp  math_util.cpp
        │
        │  ① 预处理  (#include 展开、宏替换、条件编译)
        ▼
      main.i  counter.i  ...          纯文本，通常几千行
        │
        │  ② 编译    (C++ 语法 → 中间表示 → 优化)
        ▼
      (中间表示)
        │
        │  ③ 汇编    (生成目标平台的汇编指令)
        ▼
      main.asm  main.obj               .obj/.o = 机器码 + 符号表 + 重定位信息
        │
        │  ④ 链接    (拼装目标文件，解析跨文件符号引用，套链接脚本)
        ▼
       app.exe / firmware.elf / firmware.bin
```

动手跑一遍：

```powershell
cd C:\Users\guowang\Documents\ChatGPT\嵌入式\cpp-embedded-lab
.\tools\raw-build.ps1
```

它会依次生成 `build/raw/main.i`（预处理结果）、`.obj`、`.asm`，最后链接出 `app.exe`。
**去把 `main.i` 打开看一眼**：你会发现 `#include <cstdio>` 展开成了几千行——
这就是为什么头文件要尽量小。

加 `-KeepAsm` 能保留汇编清单：

```powershell
.\tools\raw-build.ps1 -KeepAsm
```

> **顺带记一个真实世界的坑**：MSVC 默认把 `__cplusplus` 这个宏报成
> `199711L`（也就是 C++98），哪怕你明明在用 C++17。必须加 `/Zc:__cplusplus`
> 它才肯说实话。很多开源库靠这个宏判断标准版本，被这个默认值坑过无数次。
> 本工程的脚本已经加上了，你以后自己写 CMake 时记得也加。

---

## 2. 声明 vs 定义（嵌入式 bug 的头号来源）

看这个工程的三个文件：

| 文件 | 内容 | 作用 |
|---|---|---|
| `counter.h` | `int32_t counter_next();` | **声明**：告诉编译器"有这么个函数，别处有" |
| `counter.cpp` | `int32_t counter_next() { ... }` | **定义**：真正生成代码，只有一个地方有 |
| `main.cpp` | `#include "counter.h"` | 使用它，编译时只知道声明，链接时才去找定义 |

必须刻在脑子里的三条：

1. **声明可以有很多份，定义只能有一份**（ODR：One Definition Rule）。
   头文件被 `#include` 进 10 个 `.cpp`，里面的**函数定义**就会出现 10 次 → 链接报
   `already defined`。所以头文件里只放声明；函数体放 `.cpp`。

2. **`#include` 是纯文本粘贴**，没有任何智能。所以要用 `#pragma once`
   （或用 `#ifndef` 宏）防止同一个头文件被粘贴两次。

3. **链接报错和编译报错完全是两码事**：
   - `error C2065: 未声明的标识符` → 编译器不认识这个名字 → 少了 `#include` 或拼错了
   - `error LNK2019: 无法解析的外部符号` → 编译通过了，但链接找不到实现 →
     少了 `.cpp` 文件、函数签名对不上、或者库没链上

**这两种错误的排查方向完全不同。分清它们，你就赢了一半。**

---

## 3. `extern "C"`：和 C 代码（以及硬件厂家 SDK）打交道

C++ 会把函数名"改名"（name mangling），把参数类型编进符号里，用来支持重载。
C **不会**。于是：

```cpp
// counter.h 里的 C++ 函数，编译出来的符号是 ?counter_next@@YAHXZ 之类
int32_t counter_next();
```

```c
// 一个 C 文件如果调用它，链接时按 C 的规则去找 _counter_next → 找不到
```

解决办法是告诉 C++："这个函数按 C 的规矩命名"：

```cpp
#ifdef __cplusplus
extern "C" {
#endif

int32_t counter_next();

#ifdef __cplusplus
}
#endif
```

工程里的 `math_util.h` 就是这么写的。`__cplusplus` 宏在 C++ 编译器里才有定义，
所以同一个头文件 C 和 C 都能用。**你在嵌入式里天天会见到这个写法**，
因为 HAL 库、RTOS、厂家驱动几乎全是 C。

---

## 4. 关于 `static` 和匿名命名空间（嵌入式里很常用）

```cpp
// counter.cpp
static uint32_t g_count = 0;      // C 风格：只在本编译单元可见
namespace { uint32_t g_other; }   // C++ 风格：等价效果，更推荐
```

作用：**把符号藏起来，不导出到链接阶段**。
嵌入式的实际意义：

* 避免两个模块不小心用了同名全局变量，链接时报 `already defined`
* 链接器能直接丢掉用不到的代码，固件更小
* 不给别人留下乱用的机会

原则：**全局变量/内部函数默认加 `static` 或放进匿名命名空间，除非确实要跨文件用。**

---

## 5. 本课任务

1. 跑 `.\tools\raw-build.ps1`，再跑一遍 `-KeepAsm` 版本
2. 打开 `build/raw/main.i`，找到 `counter_next` 那几行，理解预处理只做文本替换
3. 回答下面三个问题（写不写下来都行，但要在心里过一遍）：
   - 如果我把 `counter.cpp` 从 `raw-build.ps1` 的编译命令里删掉，会在第几步、报什么错？
   - 如果我把 `counter.h` 里的 `#pragma once` 删掉，并在 `main.cpp` 里
     `#include "counter.h"` 两次，会出问题吗？为什么？
   - `math_util.h` 里的 `extern "C"` 如果去掉，从 C 代码调用它会怎么样？

4. 想验证第 1 个问题，直接改 `raw-build.ps1` 试一次——**故意弄坏再修好，
   比一次成功学到的东西多得多。**

---

## 6. 下一课预告

第 01 课开始写代码：定宽整型、位运算、寄存器字段读写、饱和运算、Q15 定点乘法。
这些是你后面写驱动、写 PID、写协议解析时每天都要用的东西。
