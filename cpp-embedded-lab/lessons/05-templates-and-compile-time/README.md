# 第 05 课 · 模板与编译期计算

**这一课解决的是：同一份逻辑怎么适配多种类型、怎么把检查从运行期挪到编译期。**

```powershell
cd C:\Users\guowang\Documents\ChatGPT\嵌入式\cpp-embedded-lab
.\tools\build.ps1 -Lesson 05
```

> **这一课你改的是 `exercises.h`，不是 `exercises.cpp`。**
> 因为模板必须定义在头文件里 —— 这本身就是本课最重要的知识点之一。

---

## 1. 为什么需要模板

假设你要给三种类型写同一个钳位函数：

```cpp
std::int16_t clamp_i16(std::int16_t v, std::int16_t lo, std::int16_t hi);
std::int32_t clamp_i32(std::int32_t v, std::int32_t lo, std::int32_t hi);
std::uint8_t clamp_u8 (std::uint8_t  v, std::uint8_t  lo, std::uint8_t  hi);
```

三段一模一样的代码，只有类型不同。某天你发现边界处理有个 bug，
得改三处，漏一处就埋了个雷。

模板把"类型"也变成参数：

```cpp
template <typename T>
T clamp_value(T v, T lo, T hi) {
    if (v < lo) { return lo; }
    if (v > hi) { return hi; }
    return v;
}
```

`T` 是个占位符。你调用 `clamp_value(5, 0, 10)`，编译器推导出 `T = int`，
然后生成一份 int 版本；你调用 `clamp_value<std::uint32_t>(...)`，
它再生成一份 `uint32_t` 版本。

### 模板是"图纸"，不是"代码"

这句话是理解后面一切的关键：

> **模板本身不生成任何机器码，它只是一张图纸。**
> **只有当你真的用它（术语叫"实例化"）时，编译器才照着图纸造出真代码。**

好处是"零成本"：`clamp_value(5, 0, 10)` 编译出来的汇编，
和手写一个 `clamp_int` 函数**完全一样**。没有虚函数表、没有额外内存、
没有运行时的类型查找。C++ 圈把这种特性叫**零开销抽象**。

---

## 2. 为什么模板必须写在头文件里

这是本课的核心知识点，也是你这次要改 `.h` 的原因。

回顾第 03 课的规则：

```
普通函数：  声明放头文件，定义放 .cpp
```

原因是编译器编译某个 `.cpp` 时，只要知道函数长什么样（声明）就能生成
调用代码，具体实现在**链接阶段**再去找。

但模板不能这样。看这段：

```cpp
// util.cpp
template <typename T>
T clamp_value(T v, T lo, T hi) { ... }      // 只有定义，没有具体类型
```

**编译器看到这个模板时，一个字节的机器码都不会生成。**因为它不知道 `T`
是什么 —— 是 `int`？`float`？还是一个 200 字节的结构体？生成的代码完全不同。

只有到调用点，编译器才知道要造哪个版本：

```cpp
// main.cpp
clamp_value(5, 0, 10);        // 现在才知道：给我造一份 int 版本
```

**但 main.cpp 看不到 util.cpp 里的模板定义**（那是另一个编译单元），
于是它没法生成代码，链接时报"无法解析的外部符号"。

**结论：模板的声明和定义必须放在同一个地方，而且必须能被所有使用者看到。
那就是头文件。**

> 以后你看到 `.tpp`、`.ipp`、`.inl` 这类文件，是同一种思路的变体：
> 把模板实现单独放一个文件，再由头文件 `#include` 进来。

---

## 3. `constexpr`：把计算挪到编译期

`constexpr` 函数有两种身份：

```
实参在编译期已知    ->  它在编译期算完，运行时零开销
实参要到运行时才有  ->  它就当普通函数用
```

```cpp
constexpr std::uint32_t bit_mask(unsigned n) { ... }

constexpr auto kMask = bit_mask(8);   // 编译期就算完，0xFF 直接烧进 flash

std::uint32_t n = read_register();
auto m = bit_mask(n);                 // 运行时算，和普通函数一样
```

**同一个函数，两种用法，不用写两遍。**

### 嵌入式的三个好处

| 好处 | 说明 |
|---|---|
| **不占 RAM** | 结果直接烧进 flash 的常量区 |
| **不花时间** | 运行时零指令 |
| **能编译期检查** | 配 `static_assert` 用，错了编译就过不去 |

第三条是重点。

---

## 4. `static_assert`：让假设在编译期爆炸

```cpp
static_assert(bit_mask(8) == 0x000000FFu, "bit_mask(8) 应该是 0xFF");
```

读作：**"如果这个条件不成立，就别编译了，把这句话打给我看。"**

它和 `assert` 的区别是决定性的：

| | `assert` | `static_assert` |
|---|---|---|
| 检查时机 | 运行期 | **编译期** |
| 失败后果 | 程序终止 | **编译失败** |
| 运行时开销 | 有（Release 里通常被去掉） | **零** |
| 能检查什么 | 运行时的值 | 编译期常量 |

**嵌入式最需要它**，因为很多假设在编译期就能验证：

```cpp
static_assert(sizeof(Header) == 8, "协议头必须是 8 字节");
static_assert(sizeof(std::int32_t) == 4, "这个平台必须支持 32 位整数");
static_assert(kTxBufferSize >= kMaxFrameSize, "缓冲区装不下一帧");
static_assert(array_count(kSineTable) == 256, "正弦表长度必须是 256");
```

这些假设一旦被破坏（换编译器、改结构体、加字段），**编译直接失败**，
而不是等到设备在客户手里跑了三个月之后随机死机。

### 本课的动手环节

`exercises.h` 里我给 `bit_mask` 和 `array_max` 各准备了几行 `static_assert`，
但是**注释掉的**。做完那两题之后把它们打开，编译一次：

* 实现正确 → 正常通过，你会亲眼看到"编译期求值"发生了
* 故意改错一个数字 → 编译器在**编译阶段**就报错，还带着你写的提示语

---

## 5. `FixedVector<T, N>`：嵌入式该用的容器

```cpp
FixedVector<std::int32_t, 8> v;
v.push_back(42);
v[0];            // 42
v.capacity();    // 8
```

**和 `std::vector` 的根本区别：容量固定，从不分配内存。**

| | `std::vector` | `FixedVector<T, N>` |
|---|---|---|
| 内存从哪来 | 运行时向堆要 | **对象自己身上** |
| 内存多大 | 运行时才知道 | **编译期就知道** |
| 会不会失败 | 会（分配失败抛异常） | **不会** |
| 耗时 | 不确定 | **确定** |
| 堆碎片 | 有 | **没有** |

嵌入式里"确定"两个字值一切。你能在编译期说清楚"这个模块最多用 128 字节
RAM"，这才是可交付的工程。

### `static_assert` 保护类的不变量

```cpp
template <typename T, std::size_t N>
class FixedVector {
    static_assert(N > 0u, "FixedVector 的容量必须大于 0");
};
```

`N == 0` 的话 `std::array<T, 0>` 就没有元素，容量相关的逻辑全失去意义。
与其在运行时小心翼翼，不如**让它根本编译不出来**。

---

## 6. 模板的代价

模板不是免费的，代价都在编译期和代码体积上：

| 代价 | 说明 |
|---|---|
| **编译变慢** | 每个用到模板的 `.cpp` 都要重新实例化一遍 |
| **代码膨胀** | `FixedVector<int,8>` 和 `FixedVector<int,16>` 是**两个不同的类型**，各生成一份代码 |
| **错误信息难读** | 一屏几十行报错，其实只有一个字打错了 |
| **调试器支持差** | 断点、变量查看经常不太灵 |

**嵌入式的实际影响**：代码膨胀会吃掉 flash。如果一个模板被实例化很多次，
固件体积可能明显变大。第 14 课会讲怎么测量和控制。

**读模板错误信息的技巧**：只看**第一行**的 `error`，它通常就够定位问题。
后面几十行是实例化调用栈，初学时可以直接忽略。

---

## 7. 本课检查清单

- [ ] 模板为什么必须写在头文件里？
- [ ] `template <typename T>` 里的 `T` 是什么时候被确定的？
- [ ] `constexpr` 函数什么时候会真的在编译期跑？
- [ ] `static_assert` 和 `assert` 的三个区别是什么？
- [ ] `FixedVector` 和 `std::vector` 最根本的区别是什么？
- [ ] 为什么 `FixedVector` 的 `operator[]` 要写两个版本（const 和非 const）？
- [ ] 模板的代价有哪些？

---

## 8. 下一课预告

第 06 课：不用堆的容器与字符串。把 `FixedVector` 扩展成带迭代器、
能配合范围 for 循环的完整版本，再用 `std::string_view` 处理
"不需要分配内存的字符串"。

至此第一部分（基础重建）结束。**从这里开始，后面都是嵌入式专属的内容了。**
