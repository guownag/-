# 嵌入式 C++ 训练营

给「学过 C++ 但忘了，现在要能上手写嵌入式代码」的人准备的一条可执行路线。

不是读教程，是**写代码**。每一课都是一块能编译、能跑测试的小工程。

---

## 怎么用

```
cd C:\Users\guowang\Documents\ChatGPT\嵌入式\cpp-embedded-lab
.\tools\build.ps1
```

每一课的流程固定是四步：

1. 读该课的 `README.md`（讲清"嵌入式为什么在乎这件事"，不是泛泛的语言教程）
2. 跑 `.\tools\build.ps1 -Lesson 01` → 看到测试**红**（未通过）
3. 改 `lessons/01-.../exercises.cpp`，把函数填出来
4. 再跑，直到**绿**（全部通过）

卡住了或者做完了，可以对照 `solutions/` 同名的参考答案。**先自己写。**

### 命令速查

| 命令 | 作用 |
|---|---|
| `.\tools\build.ps1` | 配置 + 编译 + 跑全部测试 |
| `.\tools\build.ps1 -Lesson 01` | 只编译并测试第 01 课 |
| `.\tools\build.ps1 -Config Release` | Release 构建 |
| `.\tools\build.ps1 -Clean` | 清掉 build 目录重来 |
| `.\tools\raw-build.ps1` | 第 00 课：手工走一遍编译四步 |

---

## 硬性约定

这些规矩从第一课就按嵌入式标准来，因为改习惯比学新东西难得多：

| 约定 | 原因 |
|---|---|
| **C++17**，不用编译器扩展 | 老版 `arm-none-eabi-gcc`、IAR、Keil 对 C++20 支持不齐 |
| **警告即错误**（`/W4 /WX`） | MCU 上出问题的代码，主机上往往只是条警告 |
| **不用异常、不用 RTTI**（后期课程强制关闭） | 代码体积和确定性；`-fno-exceptions -fno-rtti` |
| **不用裸 `int`，只用定宽整型** | `int` 在不同平台是 16/32 位，"看起来对"的溢出跨平台就是 bug |
| **默认不做动态分配** | 堆碎片在跑几个月的设备上是定时炸弹 |
| 目标平台假设 | 32 位 MCU（Cortex-M）、小端、无 OS 或轻量 RTOS、寄存器映射 I/O |

---

## 路线图

### 第一部分 · 基础重建（把 C 和 C++ 的手感捡回来）

| # | 主题 | 你会写出什么 | 状态 |
|---|---|---|---|
| 00 | [工具链](lessons/00-toolchain/README.md) | 看懂 预处理→编译→汇编→链接，多文件工程的声明/定义/ODR | 可做 |
| 01 | [类型、位运算与常量](lessons/01-types-and-bits/README.md) | 定宽整型、掩码、寄存器字段读写、饱和运算、Q15 定点乘法 | 可做 |
| 02 | [指针、引用与内存布局](lessons/02-pointers-and-memory/README.md) | 指针算术、端序读写、`volatile` 寄存器访问、对齐与填充 | 可做 |
| 03 | [函数、头文件与编译单元](lessons/03-functions-and-modules/README.md) | 模块拆分、`extern "C"` 与 C 互操作、函数指针、回调注册表 | 可做 |
| 04 | [类与 RAII 生命周期](lessons/04-classes-and-raii/README.md) | 构造/析构、RAII 临界区与片选、Rule of 0/3/5、静态初始化顺序 | 可做 |
| 05 | [模板与编译期计算](lessons/05-templates-and-compile-time/README.md) | 函数模板、`constexpr`、`static_assert`、固定容量 `FixedVector<T,N>` | 可做 |

### 第二部分 · 嵌入式核心能力

| # | 主题 | 你会写出什么 | 状态 |
|---|---|---|---|
| 06 | 不用堆的容器与字符串 | 固定容量环形队列、`std::string_view`、自己的 `static_vector` | 待开发 |
| 07 | 寄存器级编程 | 内存映射 I/O 模拟、位域与 `#pragma pack` 的坑、结构体映射寄存器 | 待开发 |
| 08 | 中断与并发 | ISR 安全、`std::atomic`、内存序、临界区、`volatile` 的真实作用 | 待开发 |
| 09 | 状态机与函数指针表 | 表驱动状态机、回调注册、用模板代替虚函数 | 待开发 |
| 10 | 无锁环形缓冲 | SPSC 队列、生产者/消费者内存屏障、溢出策略 | 待开发 |
| 11 | 定点数与数值 | 饱和加减乘除、Q 格式、定点 PID、不用浮点的三角函数 | 待开发 |

### 第三部分 · 工程化与综合

| # | 主题 | 你会写出什么 | 状态 |
|---|---|---|---|
| 12 | 不用异常的错误处理 | 错误码、`Result<T>`、断言与看门狗、故障降级 | 待开发 |
| 13 | 硬件抽象层与可测试设计 | HAL 接口、依赖注入、主机端 mock 测试驱动 | 待开发 |
| 14 | 性能与资源 | 栈深度、内联、避免浮点、`const`/`static` 表放 flash、链接脚本意识 | 待开发 |
| 15 | 测试、静态分析与 CI | 主机端单元测试、`clang-tidy`/`cppcheck`、一键流水线 | 待开发 |
| 16 | 综合项目 | 传感器采集 + 定点 PID + 协议解析 + 状态机，纯软件可跑、可测 | 待开发 |

---

## 工程结构

```
cpp-embedded-lab/
├── CMakeLists.txt          公共编译选项 + 课程序列
├── include/lab_check.hpp   极简测试框架（无第三方依赖）
├── tools/                  构建脚本
├── lessons/NN-topic/
│   ├── README.md           这一课的知识点 + 任务说明
│   ├── exercises.h         函数声明（不要改）
│   ├── exercises.cpp       ← 你写代码的地方
│   ├── tests.cpp           验收测试（不要改，改了就是自己骗自己）
│   └── CMakeLists.txt
└── solutions/NN-topic/     参考答案
```
