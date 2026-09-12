# 第 02 课 · 指针、引用与内存布局

**这一课解决的是：地址怎么用、字节序怎么处理、`volatile` 到底管什么、结构体为什么不能直接往协议里扔。**

```powershell
cd C:\Users\guowang\Documents\ChatGPT\嵌入式\cpp-embedded-lab
.\tools\build.ps1 -Lesson 02
```

> **每一道题的详细解释都写在 `exercises.cpp` 里**，就在函数的正上方。
> 每题的格式是固定的四段：
>
> * **【含义】** 这个东西到底是什么，精确地说
> * **【场景】** 嵌入式里什么时候会用到它
> * **【思路】** 从"含义"推到"代码"的那一步怎么走
> * **【陷阱】** 真实项目里踩过的坑，也是测试会盯着你的地方
>
> 下面这几节是**背景知识**，配合着看会更透。做题时先读 `exercises.cpp`。

---

## 1. 指针：嵌入式里绕不开的三件事

### 指针算术

```cpp
std::int32_t data[10];
std::int32_t* p = data;        // 指向 data[0]
p + 3                          // 不是"地址加 3"，是"向后走 3 个 int32_t"，即 +12 字节
*(p + 3)   ==  p[3]            // 完全等价
```

**指针的加减以"元素大小"为单位**，这是指针运算唯一要记住的规则。
在嵌入式里这条规则直接对应硬件行为：`uint32_t*` 加 1 走 4 字节，
所以你用 `uint32_t*` 遍历 DMA 缓冲区不会踩到半路。

### 半开区间 `[first, last)`

看第 4 题 `find_byte(first, last, value)`：传的是"起始"和"末尾的下一个"。

```cpp
find_byte(data, data + 6, x)    // 检查 data[0] ~ data[5]
```

为什么不用"起始 + 长度"？因为长度是有符号的、可能算错，而指针比较天然严谨：

```cpp
for (const uint8_t* p = first; p != last; ++p)   // 长度是 0 时循环体一次都不进
```

`last` 是开区间这件事在测试里专门验了：`find_byte(data, data + 3, 0xDD)` 必须
返回 `nullptr`，即使 `data[3]` 就是 `0xDD`。

### `const` 的位置

```cpp
const uint8_t* p;          // 指向的内容不能改（指向能改）—— 只读缓冲区就用这个
uint8_t* const p;          // 指针本身不能改（内容能改）—— 少见
const uint8_t* const p;    // 两个都不能改
```

读法：**从右往左读**。`const uint8_t* p` 读作 "p is a pointer to const uint8_t"。

嵌入式里的实际意义：把函数参数尽量标成 `const`。编译器才能确定它是只读的，
从而做更好的优化，也顺带防止你自己手滑写错。

---

## 2. 引用：C++ 给你的语法糖

```cpp
void counter_init(Counter& c);       // 引用：不是指针，但底层就是地址
```

和指针的区别（**编译后基本一样，区别在源码层面**）：

| | 指针 | 引用 |
|---|---|---|
| 能否为空 | 能（`nullptr`） | 不能（必须绑定到一个对象） |
| 能否重新指向 | 能 | 不能 |
| 语法 | `(*p).field` / `p->field` | `r.field` |

**选择标准**：

* 参数一定有值、且要被修改 → 用引用 `T&`
* 参数可能没有值、或者参数本身就是可空的（比如 `find` 的返回值）→ 用指针 `T*`
* 只读大对象 → `const T&`（避免拷贝）

硬件寄存器相关的**一定要用指针**，因为地址是运行时才确定的，而且"可空"本身就是有意义的语义。

---

## 3. 端序：为什么不能 `memcpy` 到结构体

假设你的 MCU 收到了 4 个字节：`78 56 34 12`。

```
小端机器（x86 / Cortex-M 默认）：这 4 字节读成 0x12345678
大端机器：                       这 4 字节读成 0x78563412
```

协议规定的是**字节序列**，不是整数。所以跨设备传输必须规定端序：

* **小端（LE）**：最低有效字节在前。x86、Cortex-M 默认都是它。
* **大端（BE）**：最高有效字节在前。也叫**网络字节序**，
  很多工业协议（Modbus TCP、部分 CANopen 实现）用它。

### 正确的做法

```cpp
// 手写移位，不依赖平台端序，也不要求对齐
std::uint32_t read_u32_le(const std::uint8_t* p) {
    return static_cast<std::uint32_t>(p[0]) |
           (static_cast<std::uint32_t>(p[1]) << 8) |
           (static_cast<std::uint32_t>(p[2]) << 16) |
           (static_cast<std::uint32_t>(p[3]) << 24);
}
```

### 三个坑

```cpp
// 坑 1：依赖平台端序 —— 换个大端芯片（某些 TI DSP、部分网络处理器）就全错
uint32_t v = *reinterpret_cast<const uint32_t*>(p);
```

```cpp
// 坑 2：非对齐访问 —— 在 Cortex-M0 上直接 HardFault
//       （M3/M4/M7 支持非对齐，但性能下降；M0 直接异常）
// 上面那种手写移位的写法天然对齐安全，因为只做字节访问。
```

```cpp
// 坑 3：整数提升 —— 单个 uint8_t 参与运算时会被提升成 int
uint32_t bad  = p[0] << 24;                                        // 有符号 int 左移溢出！
uint32_t good = static_cast<uint32_t>(p[0]) << 24;                 // 安全
```

第 3 个坑特别隐蔽。`p[0]` 是 `uint8_t`，`p[0] << 24` 里它被提升成 `int`，
当 `p[0] >= 128` 时，`int` 左移 24 位超过 `INT32_MAX`，是 **UB**。
所以写协议解析时，**每一个字节都要先 `static_cast<uint32_t>`**。

---

## 4. `volatile`：它管的是"编译器不要乱优化"，不是"线程安全"

这是被误解最多的关键字。先记住一句话：

> **`volatile` 告诉编译器：这个内存可能被程序之外的东西改掉，
> 所以每一次读写都必须真的发生，不能缓存、不能合并、不能删除。**

### 它管用在哪

```cpp
volatile std::uint32_t* const kUartStatus = reinterpret_cast<volatile std::uint32_t*>(0x40004000UL);

while ((*kUartStatus & kTxReady) == 0) { }   // 等硬件把这一位置起来
```

没有 `volatile`，编译器会认为"循环里没人改这个值"，直接优化成：

```cpp
if ((*kUartStatus & kTxReady) == 0) { while (true) {} }   // 死循环
```

因为寄存器是被**硬件**改的，编译器看不见这件事。

另外两个典型场景：

* 中断服务程序（ISR）里修改的变量，主循环里在等它（这在 C++ 里其实不够，
  正确做法是 `std::atomic`，第 08 课讲）
* 延迟循环 `for (volatile int i = 0; i < 1000; ++i) {}`（其实该用硬件定时器）

### 它**不**管什么

```cpp
volatile int g_flag = 0;
// 线程 A                    // 线程 B
g_flag = 1;                  while (g_flag == 0) { }   // volatile 救不了你
```

`volatile` 只保证"这次访问真的发生了"，**不保证**：

* 原子性（32 位写入在 8 位 MCU 上不是原子的）
* 顺序性（编译器和 CPU 都可能重排）
* 多个核/线程之间的可见性

这些是 `std::atomic` 和内存屏障的活。**"volatile 不等于线程安全"这句话，
每年都在各种项目里用血换一次。**

### 关于本课的 `reg_set_bits`

```cpp
void reg_set_bits(volatile std::uint32_t* reg, std::uint32_t mask);
```

为什么参数是 `volatile` 指针？因为这类函数的唯一用途就是操作寄存器。
如果写成 `uint32_t*`，调用方就得 `const_cast` 强转（丢掉 volatile 限定），
那是明确告诉编译器"请随便优化我的硬件访问" —— 灾难。

> 真实项目里这些"读-改-写"操作还有个大坑：**读和写之间可能被中断打断**，
> 导致另一个上下文改的那些位被覆盖回去（read-modify-write race）。
> 第 08 课会专门解决它。

---

## 5. 结构体布局：`sizeof` 为什么比你以为的大

```cpp
struct SensorFrame {
    std::uint16_t id;              // 2 字节
    std::int16_t  temperature_q8;  // 2 字节
    std::uint32_t timestamp_ms;    // 4 字节
    std::uint8_t  crc;             // 1 字节
};                                 // 加起来 9 字节

sizeof(SensorFrame) == 12          // 实际是 12！
```

多的 3 字节是**填充（padding）**。原因：

* CPU 访问 4 字节的 `uint32_t` 时，希望它的地址是 4 的倍数（**对齐要求**）。
  非对齐访问在 Cortex-M0 上是硬件异常，在其它平台上是性能惩罚。
* 所以编译器在成员之间、以及结构体末尾插入空洞，让每个成员都落在合适的地址上。

```
偏移:  0    1    2    3    4    5    6    7    8    9    10   11
      [ id ][ id ][tmp ][tmp ][ timestamp   ][crc ][pad ][pad ][pad]
                  ↑ 没有空洞        偏移 4 对齐       ↑ 尾部填充
```

用 `offsetof(SensorFrame, timestamp_ms) == 4` 可以亲眼验证。

### 为什么不能直接往协议里扔结构体

```cpp
// 千万别这么干
send(fd, &frame, sizeof(frame), 0);      // 发出去了 12 字节，包含 3 个垃圾填充字节
```

两个致命问题：

1. **多了 3 个填充字节**，对方按 9 字节的协议解析就会错位
2. **填充字节的内容不确定** —— `memset` 过还好，否则可能泄漏栈上的旧数据
3. **端序还没处理**
4. 换编译器/换 `#pragma pack` 设置/换平台，布局可能又变了

正确做法是**显式逐字段序列化**（第 6 题）：

```cpp
std::size_t pack_frame(const SensorFrame& f, std::uint8_t* out, std::size_t cap);
```

多写十几行代码，换来确定性和可移植性。**这是嵌入式协议处理的铁律。**

### `#pragma pack` 的诱惑

你可能会想："我 `#pragma pack(1)` 让它紧凑不就行了？"

```cpp
#pragma pack(push, 1)
struct WireFrame { std::uint16_t id; std::uint32_t ts; };   // 6 字节，无填充
#pragma pack(pop)
```

能编译，但代价是：

* 所有成员都变成非对齐的，在 Cortex-M0 上访问 `ts` 直接 HardFault
* 编译器为了生成安全代码会插入额外的字节拼装指令（有时反而更慢更大）
* 你把这个平台相关的特性写进了协议格式里

**可以用于映射硬件寄存器组**（那里地址是固定的，需要精确布局），
**不能用于定义协议格式**。第 07 课会细讲寄存器映射。

---

## 6. 本课检查清单

- [ ] `p + 1` 在 `uint32_t*` 上走几个字节？
- [ ] 为什么 `p[0] << 24` 有 UB 风险？怎么改？
- [ ] `volatile` 保证了什么、没保证什么？
- [ ] `sizeof(SensorFrame)` 是 12 但协议要 9 字节，多的 3 字节从哪来？
- [ ] 为什么协议解析不能用 `reinterpret_cast` 直接读结构体？
- [ ] `const uint8_t*` 和 `uint8_t* const` 的区别？

---

## 7. 下一课预告

第 03 课：把代码拆成真正的模块（头文件 + 编译单元），
讲清 `extern "C"`、`static`、匿名命名空间的链接语义，
以及为什么嵌入式的接口设计比 PC 上更克制。
