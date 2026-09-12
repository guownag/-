# 第 03 课 · 函数、头文件与编译单元

**这一课解决的是：代码怎么拆成模块、符号怎么隐藏、函数怎么当数据传。**

```powershell
cd C:\Users\guowang\Documents\ChatGPT\嵌入式\cpp-embedded-lab
.\tools\build.ps1 -Lesson 03
```

> **每一道题的详细解释都写在 `exercises.cpp` 里**，就在函数的正上方。
> 格式是固定的四段：【含义】【场景】【思路】【陷阱】。
> 下面这几节是背景知识，配合着看会更透。

---

## 1. 头文件放声明，源文件放定义

第 00 课已经讲过这条规则，这一课要把它变成肌肉记忆。

```
module.h     只放：函数声明、类型定义、常量、模板
module.cpp   只放：函数体（定义）
```

**为什么？**因为头文件会被 `#include` 进很多个 `.cpp`，每个 `.cpp` 都会
编译成一份 `.obj`。如果函数体写在头文件里，链接时就会出现多个同名符号，
报 `already defined`。

这一课的 `linkage_demo.h` / `linkage_demo.cpp` 就是标准范例：

```cpp
// linkage_demo.h
std::uint32_t counter_next();      // 声明：告诉编译器"有这么个函数"

// linkage_demo.cpp
std::uint32_t counter_next() {     // 定义：真正生成代码
    ++g_counter;
    return g_counter;
}
```

**自己做一个实验**：把 `linkage_demo.cpp` 里的 `counter_next` 函数体
复制到 `linkage_demo.h` 里，再跑一次 `build.cmd -Lesson 03`。
你会看到链接错误 `error LNK2005: ... 已经在 ... 中定义`——
那些符号被两个 `.obj` 各定义了一份。看完记得改回来。

---

## 2. 内部链接：让符号消失

有时候你写了一个辅助函数或全局变量，**只给自己这个 `.cpp` 用**，
不希望别人能链接到它。这就是**内部链接**（internal linkage）。

两种写法，效果一样：

```cpp
// 写法 A：C 风格
static std::uint32_t g_counter = 0;

// 写法 B：C++ 风格，更推荐
namespace {
std::uint32_t g_counter = 0;
}  // namespace
```

`linkage_demo.cpp` 用的是写法 B。这个 `g_counter` 虽然在文件里是个全局变量，
但**链接器根本不知道它的名字**。

**嵌入式的实际意义：**

* 两个模块不会不小心用了同名的内部变量，链接时不会冲突
* 链接器知道这个符号外部用不到，可以把它和它牵连的代码一起丢掉，**固件更小**
* 不给别人留下乱用你内部状态的机会

原则：**模块内部的全局变量和辅助函数，默认放匿名命名空间，除非确实要跨文件用。**

---

## 3. `extern "C"`：和 C 世界打交道

这是 C++ 和 C 混编时的第一道关，而嵌入式里躲不开——几乎所有 SDK
（HAL 库、FreeRTOS、厂家驱动）都是 C 写的。

**问题的根源是"名字修饰"（name mangling）。**

C++ 支持函数重载，所以编译器必须把参数类型编进符号名里，才能区分
`foo(int)` 和 `foo(double)`。于是 `lab03_c_add` 编译出来的符号可能长这样：

```
?lab03_c_add@@YAHHH@Z
```

而 C 编译器生成的符号就是简简单单的 `_lab03_c_add`。两边对不上，
链接时报"无法解析的外部符号"。

**解决办法**是告诉 C++："这个函数按 C 的规矩命名。"

```cpp
#ifdef __cplusplus
extern "C" {
#endif

std::int32_t lab03_c_add(std::int32_t a, std::int32_t b);

#ifdef __cplusplus
}
#endif
```

那个 `#ifdef __cplusplus` 是**必须的**：C 编译器不认识 `extern "C"` 这个
关键字，直接写在头文件里 C 那边就编译不过。而这个宏只有 C++ 编译器才定义，
于是 C 看到的是干干净净的函数声明，C++ 看到的是包了 `extern "C"` 的版本。

**这一个头文件，C 和 C++ 都能用。**记住这个模式，你在嵌入式里会见到几千次。

---

## 4. 函数指针：把函数当数据传

这是本课最重要的部分，也是 C++ 里最像"硬件思维"的一个特性。

```cpp
using BinOp = std::int32_t (*)(std::int32_t, std::int32_t);
```

读法是从中间往外：

```
std::int32_t (*)(std::int32_t, std::int32_t)
             ^^
       这是一个指针，指向"收两个 int32_t、返回 int32_t"的函数
```

有了它，函数就能像变量一样被传递：

```cpp
std::int32_t apply_binop(BinOp op, std::int32_t a, std::int32_t b) {
    return op(a, b);       // op 是个变量，里面存的是一个函数的地址
}

apply_binop(op_add, 10, 3);    // -> 13
apply_binop(op_mul, 10, 3);    // -> 30
```

**`apply_binop` 根本不知道自己在做加法还是乘法**——这个决定权交给了调用者。
这叫**解耦**。

### 嵌入式为什么离不开它

| 用途 | 说明 |
|---|---|
| 中断向量表 | 芯片内部就是一张"中断号 → 函数地址"的表 |
| 回调注册 | 驱动决定"什么时候"，应用层决定"做什么" |
| 状态机 | 每个状态是一个函数，切换状态就是换指针（第 09 课） |
| 命令解析 | 收到 `"LED ON"` 就查表调用对应的处理函数 |
| RTOS 任务 | 创建任务时传的就是一个函数指针 |

---

## 5. 回调注册表：本课的重头戏

第 3 题让你实现一个小的"事件注册表"，它是嵌入式最经典的架构模式之一。

**场景**：写一个按键驱动。它只知道"检测到按键了"，但"按了之后干什么"
应该由应用层决定：

```cpp
event_subscribe(on_short_press);    // 短按：切换 LED
event_subscribe(on_long_press);     // 长按：进配置模式
event_subscribe(log_to_uart);       // 顺便打个日志
```

驱动和业务逻辑彻底解耦。**同一个按键驱动能原封不动地用在十个项目里。**

实现上就是"一个函数指针数组 + 一个计数器"：

```cpp
namespace {
EventHandler g_handlers[kMaxHandlers];
std::size_t  g_count = 0;
}

bool event_subscribe(EventHandler handler) {
    if (handler == nullptr)      { return false; }
    if (g_count >= kMaxHandlers) { return false; }   // 先检查容量！
    g_handlers[g_count] = handler;
    ++g_count;
    return true;
}
```

**三个要点：**

1. **容量检查必须在写入之前。**先写再判断，数组已经越界了。
2. **用 `kMaxHandlers` 而不是写死 `8`。**以后改容量只改一处。
3. **不要动态分配。**固定数组、固定上限——嵌入式里所有"容器"都长这样。

---

## 6. 输出参数：没有异常时怎么表达失败

C++ 的函数只能返回一个值。但嵌入式里你经常需要"结果 + 状态"两样东西，
而且**不能用异常**（第 12 课讲为什么）。于是标准写法是：

```cpp
bool safe_divide(std::int32_t a, std::int32_t b, std::int32_t* out);
```

**返回值报状态，输出参数带结果。**

```cpp
std::int32_t temp;
if (!sensor_read_temperature(&temp)) {
    handle_error();        // 读失败，temp 里是什么都别信
}
```

**一条必须遵守的规矩：失败时一个字节都不能写。**

因为调用方可能先给变量赋了一个有意义的值，你的函数失败却把它改成了垃圾，
调用方很可能忘了检查返回值，就拿着垃圾数据继续算了。这类 bug 极难查。

---

## 7. 头文件里到底能放什么

| 能放 | 不能放 |
|---|---|
| 函数声明 | **普通函数的定义**（函数体） |
| 类型定义（`struct` / `class` / `using`） | **全局变量的定义** |
| `constexpr` 常量 | 函数体（除非是 `inline` 或 `constexpr`） |
| `constexpr` 函数 | |
| `inline` 函数 | |
| 模板 | |

**为什么 `inline` 和 `constexpr` 是例外？**

`inline` 标记的意思是"这个定义可以出现在多个编译单元里，链接器请把它们
合并成一个"。这正好解决了头文件被多次包含的问题。`constexpr` 函数隐含
`inline`，所以也能放头文件。

**为什么模板必须放头文件？**因为模板本身不生成代码，只有在你**实例化**
（比如写下 `FixedVector<int, 8>`）的时候才生成。编译器必须能看到模板的
完整定义才能实例化它。第 05 课会专门展开这一点。

---

## 8. 本课检查清单

- [ ] 头文件里放了函数体会发生什么？为什么？
- [ ] `static` 函数和匿名命名空间里的函数，有什么共同点？
- [ ] `extern "C"` 解决的是什么问题？为什么头文件里要用 `#ifdef __cplusplus` 包起来？
- [ ] `BinOp fn = op_add;` 和 `BinOp fn = &op_add;` 有什么区别？
- [ ] 调用函数指针之前为什么一定要检查 `nullptr`？
- [ ] 你的 `event_subscribe` 是在写入前还是写入后检查容量的？
- [ ] `safe_divide` 失败时，`*out` 有没有被动过？

---

## 9. 下一课预告

第 04 课：类和 RAII。你会写一个"进入作用域自动关中断、离开作用域自动开中断"
的类——这是 C++ 在嵌入式里最漂亮的用法之一，用好了能消灭一整类 bug。
