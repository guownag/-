#pragma once
//
// 第 01 课 · 类型、位运算与常量
//
// 这个头文件是"契约"，不要改。你要做的是改 exercises.cpp 把函数填出来，
// 直到 tests.cpp 里的检查全部通过。
//

#include <cstddef>
#include <cstdint>

// 编译期断言：如果这个平台没有 32 位整数，就在编译阶段直接报错。
// 嵌入式里这叫"把假设写成代码" —— 假设只藏在大脑里，迟早出事。
static_assert(sizeof(std::int32_t) == 4, "本工程假设 int32_t 是 4 字节");
static_assert(sizeof(std::int16_t) == 2, "本工程假设 int16_t 是 2 字节");

namespace lab01 {

// 1. 位掩码
//
// make_mask(n) 返回低 n 位全为 1 的数：
//   n = 0  -> 0x00000000
//   n = 1  -> 0x00000001
//   n = 8  -> 0x000000FF
//   n = 32 -> 0xFFFFFFFF
//
// 前提：n <= 32。
//
// 提示：最直观的写法 `(1u << n) - 1` 在 n == 32 时是【未定义行为】，
//      因为移位位数等于类型位宽。你必须处理这种情况。
std::uint32_t make_mask(unsigned n);

// 2. 寄存器字段读写 —— MCU 上最常用的两个操作
//
// 现实场景：一个 32 位控制寄存器里，某几位（从 lsb 开始，宽 width 位）
//          表示一个配置值。你要能干净地读出来、写进去，且不影响别的位。
//
// insert_field(reg, lsb, width, value)：
//   把 reg 的 [lsb, lsb+width) 这几位替换为 value 的低 width 位，
//   其它位保持不变。value 超出 width 的高位要被丢弃。
//
// extract_field(reg, lsb, width)：
//   把这几位移到最低位并返回，高位补 0。
//
// 前提：1 <= width <= 32 且 lsb + width <= 32。
//
// 示例：insert_field(0xFFFFFFFF, 4, 4, 0) == 0xFFFFFF0F
//       extract_field(0x12345678, 8, 8) == 0x00000056
std::uint32_t insert_field(std::uint32_t reg, unsigned lsb, unsigned width, std::uint32_t value);
std::uint32_t extract_field(std::uint32_t reg, unsigned lsb, unsigned width);

// 3. 数 1 的个数（popcount）
//
// 统计 v 的二进制里有几个 1。
//
// 注意：不要用编译器内置函数（__popcnt / __builtin_popcount），
//      也不要查表 —— 就用循环或经典的位技巧。
//      MCU 上很多芯片没有这条指令，写不出来说明还没真懂。
unsigned popcount32(std::uint32_t v);

// 4. 钳位（clamp）
//
// 把 v 限制在 [lo, hi] 区间内（含端点）。前提：lo <= hi。
//
// 嵌入式用途：ADC 采样值防越界、PID 输出限幅、PWM 占空比限制。
std::int32_t clamp_i32(std::int32_t v, std::int32_t lo, std::int32_t hi);

// 5. 饱和加法（16 位）
//
// 计算 a + b，结果超出 int16_t 范围时钳到边界，而不是回绕。
//   32767 + 1   -> 32767
//   -32768 + -1 -> -32768
//   100 + 200   -> 300
//
// 关键：这是嵌入式里最重要的一条纪律 —— 溢出时"停在边界"远比"绕回去"安全。
//      回绕会让一个温度值从 32767 突然变成 -32768。
//
// 提示：用 int32_t 做中间计算，算完再钳。
std::int16_t sat_add_i16(std::int16_t a, std::int16_t b);

// 6. Q15 定点乘法 —— 不用浮点的数值计算
//
// Q15 格式：用一个 16 位有符号整数表示 [-1, 1) 区间的数，
//          实际值 = 存储值 / 32768。
//   -32768 (= -32768/32768) 表示 -1.0
//   16384  (= 16384/32768)  表示  0.5
//   32767  ≈ 0.99997         接近 1.0
//
// q15_mul(a, b) 返回 a 和 b 相乘的 Q15 结果。
//
// 数学上 结果 = (a * b) / 32768，要用整数实现：
//   * a * b 最大约 2^30，所以中间结果必须用 int32_t 装；
//   * "/ 32768" 用右移 15 位实现；
//   * 为减小截断误差，移位前先加 0x4000（半个 32768）做四舍五入。
//
// 示例：q15_mul(16384, 16384) == 8192      // 0.5 * 0.5 = 0.25
//       q15_mul(-16384, 16384) == -8192
//       q15_mul(32767, 16384) == 16384     // 四舍五入生效
//
// 提示：负数右移在 C++17 里是"实现定义"的，MSVC/GCC/Clang 都是算术右移
//      （向负无穷取整），这也是嵌入式里的通行做法。
std::int32_t q15_mul(std::int32_t a, std::int32_t b);

}  // namespace lab01
