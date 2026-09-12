//
// 第 01 课参考答案。
//
// 建议：先自己写完、测试全绿，再来看这里。你可能会发现自己的写法更好。
// 参考实现刻意写得朴素 —— 嵌入式代码的第一标准是可读、可验证。
//

#include "exercises.h"

namespace lab01 {

std::uint32_t make_mask(unsigned n) {
    // n == 32 时 `1u << 32` 是未定义行为（移位位数 >= 位宽）。
    // 编译器可以假设这种情况不会发生，于是把这段代码优化得面目全非。
    if (n >= 32u) {
        return 0xFFFFFFFFu;
    }
    if (n == 0u) {
        return 0u;
    }
    return (1u << n) - 1u;
}

std::uint32_t insert_field(std::uint32_t reg, unsigned lsb, unsigned width, std::uint32_t value) {
    const std::uint32_t fieldMask = make_mask(width) << lsb;
    reg &= ~fieldMask;                      // 目标字段清零
    reg |= (value << lsb) & fieldMask;      // value 的高位在这里被掩码截断
    return reg;
}

std::uint32_t extract_field(std::uint32_t reg, unsigned lsb, unsigned width) {
    // 先右移到最低位，再掩码。顺序不能反：
    // 反过来在 width == 32 时会左移 32 位，又是 UB。
    return (reg >> lsb) & make_mask(width);
}

unsigned popcount32(std::uint32_t v) {
    // 经典技巧：v & (v - 1) 恰好消掉最低位的那个 1。
    // 循环次数 = 1 的个数，最坏 32 次，没有分支预测灾难，MCU 上很稳。
    unsigned count = 0;
    while (v != 0u) {
        v &= (v - 1u);
        ++count;
    }
    return count;
}

std::int32_t clamp_i32(std::int32_t v, std::int32_t lo, std::int32_t hi) {
    if (v < lo) { return lo; }
    if (v > hi) { return hi; }
    return v;
}

std::int16_t sat_add_i16(std::int16_t a, std::int16_t b) {
    // 关键：先把两个操作数提升到 int32_t 再相加，就永远不会溢出。
    // 如果偷懒写成 int16_t 相加，结果是整型提升成 int 后计算的（在 32 位平台上
    // 恰好不出错），换到 int 只有 16 位的平台上就会静默回绕 —— 这种"换个平台
    // 才炸"的 bug 是嵌入式最怕的。
    const std::int32_t sum = static_cast<std::int32_t>(a) + static_cast<std::int32_t>(b);

    if (sum > 32767) { return 32767; }
    if (sum < -32768) { return -32768; }
    return static_cast<std::int16_t>(sum);
}

std::int32_t q15_mul(std::int32_t a, std::int32_t b) {
    // a * b 最大约 2^30，int32_t 装得下，不会溢出。
    std::int32_t product = a * b;

    // 加半个 LSB 做四舍五入，再算术右移 15 位。
    // 右移 15 位等价于 / 32768，但对负数向负无穷取整 —— 这正是我们要的，
    // 它让四舍五入在正负两侧都对称。
    product += 0x4000;  // 16384 == 32768 / 2
    return product >> 15;
}

}  // namespace lab01
