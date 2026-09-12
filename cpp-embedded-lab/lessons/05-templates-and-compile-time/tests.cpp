//
// 第 05 课验收测试。不要改。
//

#include <cstddef>
#include <cstdint>

#include "exercises.h"
#include "lab_check.hpp"

using namespace lab05;

// 测试用的自定义类型：验证 FixedVector 对"任何类型"都成立
struct Sample {
    std::int32_t value;
    std::uint16_t quality;
};

int main() {
    std::printf("=== 第 05 课 · 模板与编译期计算 ===\n");

    // -----------------------------------------------------------------------
    // 1. clamp_value —— 同一个模板，好几种类型
    // -----------------------------------------------------------------------
    LAB_CHECK_EQ(clamp_value(5, 0, 10), 5);
    LAB_CHECK_EQ(clamp_value(-5, 0, 10), 0);
    LAB_CHECK_EQ(clamp_value(50, 0, 10), 10);
    LAB_CHECK_EQ(clamp_value(0, 0, 10), 0);
    LAB_CHECK_EQ(clamp_value(10, 0, 10), 10);

    // 换成 16 位有符号，同一份代码照样工作
    LAB_CHECK_EQ(clamp_value<std::int16_t>(static_cast<std::int16_t>(-5),
                                           static_cast<std::int16_t>(0),
                                           static_cast<std::int16_t>(10)),
                 static_cast<std::int16_t>(0));
    LAB_CHECK_EQ(clamp_value<std::int16_t>(static_cast<std::int16_t>(500),
                                           static_cast<std::int16_t>(0),
                                           static_cast<std::int16_t>(100)),
                 static_cast<std::int16_t>(100));

    // 无符号类型
    LAB_CHECK_EQ(clamp_value<std::uint32_t>(300u, 0u, 255u), 255u);

    // -----------------------------------------------------------------------
    // 2. array_count —— 编译期拿到数组长度
    // -----------------------------------------------------------------------
    {
        const std::int32_t  a[7] = {};
        const std::uint8_t  b[3] = {};
        const std::uint16_t c[1] = {};

        LAB_CHECK_EQ(array_count(a), 7u);
        LAB_CHECK_EQ(array_count(b), 3u);
        LAB_CHECK_EQ(array_count(c), 1u);

        // 想看"编译期检查"是什样子？去 exercises.h 里把 array_count
        // 和 bit_mask 旁边那几行 static_assert 的注释去掉，再编译一次。
        // （这里不能写 static_assert，否则你没做完之前整个工程都编译不过。）
    }

    // -----------------------------------------------------------------------
    // 3. bit_mask —— constexpr 函数
    // -----------------------------------------------------------------------
    LAB_CHECK_EQ(bit_mask(0u),  0x00000000u);
    LAB_CHECK_EQ(bit_mask(1u),  0x00000001u);
    LAB_CHECK_EQ(bit_mask(4u),  0x0000000Fu);
    LAB_CHECK_EQ(bit_mask(8u),  0x000000FFu);
    LAB_CHECK_EQ(bit_mask(16u), 0x0000FFFFu);
    LAB_CHECK_EQ(bit_mask(31u), 0x7FFFFFFFu);
    LAB_CHECK_EQ(bit_mask(32u), 0xFFFFFFFFu);

    // -----------------------------------------------------------------------
    // 4. FixedVector —— 固定容量容器
    // -----------------------------------------------------------------------
    {
        FixedVector<std::int32_t, 4> v;

        LAB_CHECK_EQ(v.size(), 0u);
        LAB_CHECK_EQ(v.capacity(), 4u);
        LAB_CHECK(v.empty());
        LAB_CHECK(!v.full());

        LAB_CHECK(v.push_back(10));
        LAB_CHECK(v.push_back(20));
        LAB_CHECK(v.push_back(30));
        LAB_CHECK_EQ(v.size(), 3u);
        LAB_CHECK(!v.empty());
        LAB_CHECK(!v.full());

        LAB_CHECK_EQ(v[0], 10);
        LAB_CHECK_EQ(v[1], 20);
        LAB_CHECK_EQ(v[2], 30);

        // 可以改里面的元素
        v[1] = 99;
        LAB_CHECK_EQ(v[1], 99);

        LAB_CHECK(v.push_back(40));
        LAB_CHECK(v.full());
        LAB_CHECK_EQ(v.size(), 4u);

        // 满了之后必须拒绝，而且不能破坏已有数据
        LAB_CHECK(!v.push_back(50));
        LAB_CHECK_EQ(v.size(), 4u);
        LAB_CHECK_EQ(v[0], 10);
        LAB_CHECK_EQ(v[3], 40);

        // clear 之后重新可用
        v.clear();
        LAB_CHECK_EQ(v.size(), 0u);
        LAB_CHECK(v.empty());
        LAB_CHECK(!v.full());
        LAB_CHECK(v.push_back(7));
        LAB_CHECK_EQ(v.size(), 1u);
        LAB_CHECK_EQ(v[0], 7);
    }

    // 容量为 1
    {
        FixedVector<std::int32_t, 1> one;
        LAB_CHECK_EQ(one.capacity(), 1u);
        LAB_CHECK(one.push_back(1));
        LAB_CHECK(one.full());
        LAB_CHECK(!one.push_back(2));
        LAB_CHECK_EQ(one[0], 1);
    }

    // 换成自定义类型，同一个模板照样工作
    {
        FixedVector<Sample, 2> samples;
        LAB_CHECK(samples.push_back(Sample{100, 90u}));
        LAB_CHECK(samples.push_back(Sample{-20, 75u}));
        LAB_CHECK(!samples.push_back(Sample{0, 0u}));
        LAB_CHECK_EQ(samples.size(), 2u);
        LAB_CHECK_EQ(samples[0].value, 100);
        LAB_CHECK_EQ(samples[0].quality, 90u);
        LAB_CHECK_EQ(samples[1].value, -20);
    }

    // const 对象只能用 const 版本的 operator[]
    {
        FixedVector<std::int32_t, 2> v;
        v.push_back(5);
        const FixedVector<std::int32_t, 2>& cv = v;
        LAB_CHECK_EQ(cv[0], 5);
        LAB_CHECK_EQ(cv.size(), 1u);
    }

    // -----------------------------------------------------------------------
    // 5. array_max
    // -----------------------------------------------------------------------
    {
        const std::int32_t a[6] = {4, -7, 12, 0, -3, 9};
        LAB_CHECK_EQ(array_max(a), 12);

        const std::int32_t all_negative[3] = {-5, -20, -1};
        LAB_CHECK_EQ(array_max(all_negative), -1);

        const std::int32_t single[1] = {-42};
        LAB_CHECK_EQ(array_max(single), -42);

        const std::uint16_t u[4] = {1u, 999u, 250u, 3u};
        LAB_CHECK_EQ(array_max(u), 999u);
    }

    // -----------------------------------------------------------------------
    // 6. 对比样例：非模板函数（定义在 exercises.cpp 里）
    // -----------------------------------------------------------------------
    LAB_CHECK_EQ(truncate_to_u8(0x1234ABCDu), 0x000000CDu);
    LAB_CHECK_EQ(truncate_to_u8(0xFFFFFFFFu), 0x000000FFu);
    LAB_CHECK_EQ(truncate_to_u8(0x00000042u), 0x00000042u);

    return LAB_REPORT("第 05 课");
}
