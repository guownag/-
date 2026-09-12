# 第 04 课 · 类与 RAII 生命周期

**这一课解决的是：怎么让"配对操作"永远不会忘、怎么控制对象什么时候生什么时候死。**

```powershell
cd C:\Users\guowang\Documents\ChatGPT\嵌入式\cpp-embedded-lab
.\tools\build.ps1 -Lesson 04
```

> 每道题的详细解释写在 `exercises.cpp` 里，就在函数上方。
> 下面几节是背景知识。

---

## 1. `class` 和 `struct` 的区别

只有一条：

```cpp
struct Foo { int x; };   // 成员默认 public
class  Bar { int x; };   // 成员默认 private
```

**就这点区别，别的完全一样。**

那怎么选？业界的惯例是按意图：

| 用 `struct` | 用 `class` |
|---|---|
| 只是一个数据打包，没有行为 | 有明确的不变量要维护 |
| 成员可以随便改 | 成员只能在受控的前提下改 |
| 例子：`TestHw`、`SensorFrame` | 例子：`MovingAverage`、`CriticalSection` |

**嵌入式里大量使用 `struct`** —— 寄存器映射、协议帧、配置表，这些就是纯数据，
用 `struct` 更直白。只有当你需要保护状态的时候才用 `class`。

---

## 2. 构造函数：对象诞生时自动执行

```cpp
class MovingAverage {
public:
    explicit MovingAverage(std::size_t window);
};
```

只要写下 `MovingAverage avg(4);`，构造函数就会自动跑，**你不需要手动调用它**。

### 成员初始化列表

冒号后面那一串叫**成员初始化列表**：

```cpp
MovingAverage::MovingAverage(std::size_t window)
    : m_buf{}, m_sum(0), m_window(1u), m_count(0u), m_index(0u) {
    // 函数体
}
```

它的作用是**在对象诞生之前就把成员初值设好**，而不是先默认构造、再在函数体里
赋值。对嵌入式来说有两个实际意义：

1. **没有"未初始化的中间状态"。**写在函数体里的赋值，中间那一瞬间成员是
   垃圾值。如果此时恰好来了个中断，读到的就是垃圾。
2. **`const` 成员和引用成员只能在这里初始化**，在函数体里赋值是编译错误。

**尽量用初始化列表，函数体里只写逻辑。**这是 C++ 的一条基本纪律。

### `explicit` 是什么意思

```cpp
explicit MovingAverage(std::size_t window);
```

它禁止这种隐式转换：

```cpp
MovingAverage avg = 4;      // 不允许（有 explicit）
MovingAverage avg(4);       // 允许
MovingAverage avg{4};       // 允许
```

单参数构造函数默认应该加 `explicit`，除非你**真的**想要那个隐式转换。
不加的话，`4` 会在你意想不到的地方被悄悄转成 `MovingAverage`，
产生很难发现的 bug。

---

## 3. 析构函数：对象死亡时自动执行

```cpp
~MovingAverage();       // 无参数、无返回值
```

它在对象**离开作用域**时自动调用：

```cpp
{
    ChipSelect cs;      // 构造
    // ...
}                       // 这里，cs 的析构函数自动跑
```

不只是正常走到花括号，**`return`、`break`、`continue` 跳出作用域时同样会调用**。
这一点是 RAII 能成立的根本原因。

---

## 4. RAII 的核心思想

**把"获取资源"绑定到构造函数，把"释放资源"绑定到析构函数。**

于是你只要创建一个对象，资源就自动被管住了；对象消失，资源自动归还。
**你不用记得写释放代码，编译器替你记。**

看个对比。手动配对：

```cpp
disable_irq();
if (error) {
    return;                 // ← 忘了开中断！中断从此永远关着
}
do_something();
enable_irq();
```

RAII：

```cpp
{
    CriticalSection cs;     // 自动关中断
    if (error) { return; }  // 提前返回也没关系
    do_something();
}                           // 无论怎么离开，都会开中断
```

**第二种写法里，你根本没有机会犯错。**

---

## 5. 嵌入式的三大 RAII 用途

| 场景 | 构造 | 析构 |
|---|---|---|
| 关中断保护临界区 | 关中断 | 开中断 |
| SPI/I2C 片选 | 拉低 CS | 拉高 CS |
| RTOS 互斥锁 | 加锁 | 解锁 |

还有：打开/关闭外设时钟、进入/退出低功耗模式、开关看门狗喂狗窗口。

**共同特征**：两个操作必须成对出现，中间任何一条退出路径都不能漏。
这就是 RAII 的适用场景。

### 嵌套问题

第 1 题里最关键的一行是这个判断：

```cpp
if (hw().irq_nesting == 0) {
    ++hw().irq_disable_calls;   // 只有真正从"开"变"关"才调用
}
```

没有它，内层 `CriticalSection` 析构时就会**提前把中断打开**，
外层还在保护的代码就裸奔了。而这个 bug 只在嵌套时出现，平时测不出来。

**真实项目里的写法叫"嵌套计数"，这是每个嵌入式工程师都要自己踩一次
才记得住的东西。**

---

## 6. 禁止拷贝：`= delete`

```cpp
CriticalSection(const CriticalSection&)            = delete;
CriticalSection& operator=(const CriticalSection&) = delete;
```

**`= delete` 是"我明确不要这个函数，谁调用就编译报错"。**

为什么 RAII 类通常要禁止拷贝？因为拷贝出来的对象析构时会**再释放一次**：

```cpp
{
    CriticalSection a;                     // 关中断
    CriticalSection b = a;                 // 如果允许拷贝……
}                                          // b 析构：开中断
                                           // a 析构：又开一次！
```

计数乱了，而且实际硬件上可能更严重（片选被误释放、锁被解两次）。
**让编译器在编译期挡住，比运行期出事后调试便宜一万倍。**

---

## 7. Rule of 0 / 3 / 5

这是 C++ 里最重要的设计规则之一：

> **如果你需要自己写析构函数、拷贝构造、拷贝赋值中的任何一个，
> 那你多半三个都需要。**

因为它们管的是同一件事——资源的生命周期。你能想到在析构里释放资源，
就必须想到拷贝时该怎么处理（深拷贝？还是禁止？）。

| Rule | 内容 | 什么时候适用 |
|---|---|---|
| **Rule of 0** | 一个都不写，让编译器生成 | **首选**。用现成的资源管理者组合出你的类 |
| **Rule of 3** | 析构 + 拷贝构造 + 拷贝赋值 | 类里直接管着资源（指针、文件、句柄） |
| **Rule of 5** | 上面三个 + 移动构造 + 移动赋值 | 同上，还想要高效的移动 |

**对嵌入式来说 Rule of 0 是首选。**你的类大多不该自己管动态资源 ——
用固定大小的成员数组，什么都不用写，编译器生成的版本就是对的。
第 4 题的 `MovingAverage` 就是 Rule of 0：成员全是 POD，
编译器生成的拷贝就是逐字节复制，完全正确。

---

## 8. 静态成员和初始化顺序问题

```cpp
struct LifetimeCounter {
    static std::int32_t alive;              // 声明
};

std::int32_t LifetimeCounter::alive = 0;    // 定义，必须写在 .cpp 里
```

**静态成员变量要在 `.cpp` 里定义一次**（头文件里只写声明），否则链接会报
"无法解析的外部符号"。这和普通全局变量的 ODR 规则是一样的。

### 静态初始化顺序问题

如果静态对象的初值需要**调用函数**（比如 `std::string` 的构造），
那么不同编译单元之间的初始化顺序是**不确定的**：

```
// a.cpp
Config g_config;            // 构造函数要跑

// b.cpp
int x = use_g_config();     // 如果 b.cpp 先初始化，g_config 还是空的
```

一个文件里定义的静态对象被另一个文件在初始化阶段使用时，它可能还没构造完。

**两种解决办法：**

1. **用 `constexpr` / POD 常量** —— 初值直接烧进数据段，不涉及任何运行时构造。
   本课那些 `static std::int32_t` 都属于这一类，安全。
2. **用"函数内静态局部变量"**（Meyers singleton）：

```cpp
Config& config() {
    static Config instance;    // 第一次调用时才构造，之后返回同一个
    return instance;
}
```

C++11 起保证这个初始化是线程安全的，而且懒加载，完全没有顺序问题。

**在嵌入式里第 1 条是首选**：能从根上避免运行时构造，就不要引入它。

---

## 9. 本课检查清单

- [ ] RAII 的两个要素是什么？
- [ ] 为什么 `CriticalSection` 的析构函数要判断 `irq_nesting == 0`？
- [ ] `= delete` 和 `= default` 分别是什么意思？
- [ ] 拷贝构造和拷贝赋值有什么区别？哪个会让 `alive` 变？
- [ ] Rule of 0 为什么是首选？
- [ ] 静态初始化顺序问题是怎么产生的？两种解决办法分别是什么？
- [ ] `explicit` 防的是什么？

---

## 10. 下一课预告

第 05 课：模板与编译期计算。你会用 `constexpr` 把检查挪到编译期，
写一个自己的固定容量 `FixedVector<T, N>`，并用 `static_assert` 让
"假设不成立时编译直接失败" —— 这是嵌入式里零成本抽象的根基。
