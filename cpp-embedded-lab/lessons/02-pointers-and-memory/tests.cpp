//
// 第 02 课验收测试。不要改。
//

#include <cstddef>
#include <cstdint>

#include "exercises.h"
#include "lab_check.hpp"

using namespace lab02;

int main() {
    std::printf("=== 第 02 课 · 指针、引用与内存布局 ===\n");

    // -----------------------------------------------------------------------
    // 1. 就地反转
    // -----------------------------------------------------------------------
    {
        std::int32_t a[] = {1, 2, 3, 4, 5};
        reverse_in_place(a, 5);
        LAB_CHECK_EQ(a[0], 5);
        LAB_CHECK_EQ(a[1], 4);
        LAB_CHECK_EQ(a[2], 3);
        LAB_CHECK_EQ(a[3], 2);
        LAB_CHECK_EQ(a[4], 1);
    }
    {
        std::int32_t b[] = {10, 20};
        reverse_in_place(b, 2);
        LAB_CHECK_EQ(b[0], 20);
        LAB_CHECK_EQ(b[1], 10);
    }
    {
        std::int32_t c[] = {7};
        reverse_in_place(c, 1);          // 奇数个元素，中间那个不该被动
        LAB_CHECK_EQ(c[0], 7);
    }
    {
        std::int32_t d[] = {-1, -2, 3};
        reverse_in_place(d, 3);
        LAB_CHECK_EQ(d[0], 3);
        LAB_CHECK_EQ(d[1], -2);
        LAB_CHECK_EQ(d[2], -1);
    }
    reverse_in_place(nullptr, 0);        // 空数组不能崩
    {
        std::int32_t e[] = {0, 0, 0, 1};
        reverse_in_place(e, 0);          // n == 0 时不该动任何东西
        LAB_CHECK_EQ(e[3], 1);
    }

    // -----------------------------------------------------------------------
    // 2. 端序读写
    // -----------------------------------------------------------------------
    {
        // 0x12345678 的内存字节序列：小端 78 56 34 12 / 大端 12 34 56 78
        const std::uint8_t le[4] = {0x78u, 0x56u, 0x34u, 0x12u};
        const std::uint8_t be[4] = {0x12u, 0x34u, 0x56u, 0x78u};

        LAB_CHECK_EQ(read_u32_le(le), 0x12345678u);
        LAB_CHECK_EQ(read_u32_be(be), 0x12345678u);
        LAB_CHECK_EQ(read_u32_le(be), 0x78563412u);
        LAB_CHECK_EQ(read_u32_be(le), 0x78563412u);

        std::uint8_t buf[8] = {};
        write_u32_le(buf, 0x12345678u);
        LAB_CHECK_EQ(buf[0], 0x78u);
        LAB_CHECK_EQ(buf[1], 0x56u);
        LAB_CHECK_EQ(buf[2], 0x34u);
        LAB_CHECK_EQ(buf[3], 0x12u);

        write_u32_be(buf, 0x12345678u);
        LAB_CHECK_EQ(buf[0], 0x12u);
        LAB_CHECK_EQ(buf[1], 0x34u);
        LAB_CHECK_EQ(buf[2], 0x56u);
        LAB_CHECK_EQ(buf[3], 0x78u);

        // 写进去再读出来，必须还原（错开一个字节，故意制造非对齐访问）
        std::uint8_t unaligned[8] = {};
        write_u32_le(unaligned + 1, 0xDEADBEEFu);
        LAB_CHECK_EQ(read_u32_le(unaligned + 1), 0xDEADBEEFu);
        write_u32_be(unaligned + 3, 0xCAFEBABEu);
        LAB_CHECK_EQ(read_u32_be(unaligned + 3), 0xCAFEBABEu);
    }

    // -----------------------------------------------------------------------
    // 3. 端序检测
    // -----------------------------------------------------------------------
    LAB_CHECK(is_little_endian());

    // -----------------------------------------------------------------------
    // 4. 查找字节
    // -----------------------------------------------------------------------
    {
        const std::uint8_t data[6] = {0xAAu, 0xBBu, 0xCCu, 0xDDu, 0xEEu, 0xFFu};

        const std::uint8_t* p = find_byte(data, data + 6, 0xDDu);
        LAB_CHECK(p == data + 3);

        // 找到第一个就返回，不是最后一个
        const std::uint8_t dup[4] = {1u, 2u, 1u, 3u};
        LAB_CHECK(find_byte(dup, dup + 4, 1u) == dup + 0);
        LAB_CHECK(find_byte(dup, dup + 4, 3u) == dup + 3);

        LAB_CHECK(find_byte(data, data + 6, 0x11u) == nullptr);
        LAB_CHECK(find_byte(data, data, 0xAAu) == nullptr);       // 空区间
        LAB_CHECK(find_byte(data, data + 3, 0xDDu) == nullptr);   // last 是开区间，取不到
    }

    // -----------------------------------------------------------------------
    // 5. 寄存器位操作（volatile）
    // -----------------------------------------------------------------------
    {
        // 注意这里的写法：reg 本身是 volatile 的（模拟硬件寄存器），
        // 但我们要检查它的值时必须"读一次"，所以用一个普通变量接住。
        std::uint32_t storage = 0u;
        volatile std::uint32_t* reg = &storage;

        reg_set_bits(reg, 0x0000000Fu);
        LAB_CHECK_EQ(storage, 0x0000000Fu);

        reg_set_bits(reg, 0x000000F0u);
        LAB_CHECK_EQ(storage, 0x000000FFu);

        reg_clear_bits(reg, 0x0000000Fu);
        LAB_CHECK_EQ(storage, 0x000000F0u);

        reg_toggle_bits(reg, 0x000000FFu);
        LAB_CHECK_EQ(storage, 0x0000000Fu);

        // 清位只影响 mask 里的位，其它位保持不变
        std::uint32_t storage2 = 0xFFFFFFFFu;
        reg_clear_bits(&storage2, 0x0000FF00u);
        LAB_CHECK_EQ(storage2, 0xFFFF00FFu);
    }

    // -----------------------------------------------------------------------
    // 6. 结构体布局 vs 协议布局
    // -----------------------------------------------------------------------
    //
    // 这一段的期望值在 x86/x64 和 ARM 上都是一样的，可以放心断言。
    // 重点：sizeof 是 12，但真正有用的数据只有 9 字节 —— 那 3 个字节是填充。
    LAB_CHECK_EQ(sizeof(SensorFrame), 12u);
    LAB_CHECK_EQ(offsetof(SensorFrame, id), 0u);
    LAB_CHECK_EQ(offsetof(SensorFrame, temperature_q8), 2u);
    LAB_CHECK_EQ(offsetof(SensorFrame, timestamp_ms), 4u);
    LAB_CHECK_EQ(offsetof(SensorFrame, crc), 8u);

    // 协议里的线上格式是紧凑的 9 字节，和内存布局不是一回事。
    LAB_CHECK_EQ(kFrameWireSize, 9u);

    {
        SensorFrame f;
        f.id             = 0x1234u;
        f.temperature_q8 = -2560;        // -10.0 摄氏度
        f.timestamp_ms   = 0x0000ABCDu;
        f.crc            = 0x5Au;

        // 把整个缓冲区填成 0xEE，这样任何"多写了一个字节"都会被抓到。
        std::uint8_t wire[32];
        for (std::size_t i = 0; i < sizeof(wire); ++i) { wire[i] = 0xEEu; }

        const std::size_t n = pack_frame(f, wire, sizeof(wire));
        LAB_CHECK_EQ(n, 9u);

        // 逐字节核对线上格式（全部小端）
        LAB_CHECK_EQ(wire[0], 0x34u);    // id 低字节
        LAB_CHECK_EQ(wire[1], 0x12u);    // id 高字节
        LAB_CHECK_EQ(wire[2], 0x00u);    // temperature -2560 = 0xF600
        LAB_CHECK_EQ(wire[3], 0xF6u);
        LAB_CHECK_EQ(wire[4], 0xCDu);    // timestamp 0x0000ABCD
        LAB_CHECK_EQ(wire[5], 0xABu);
        LAB_CHECK_EQ(wire[6], 0x00u);
        LAB_CHECK_EQ(wire[7], 0x00u);
        LAB_CHECK_EQ(wire[8], 0x5Au);    // crc

        // 帧一共 9 字节，第 9 个字节往后一个都不能碰。
        // （很多越界写的 bug 就是多写这么一两个字节，而且平时看不出来。）
        LAB_CHECK_EQ(wire[9],  0xEEu);
        LAB_CHECK_EQ(wire[10], 0xEEu);
        LAB_CHECK_EQ(wire[15], 0xEEu);

        // 反序列化必须还原
        SensorFrame back{};
        LAB_CHECK(unpack_frame(wire, n, &back));
        LAB_CHECK_EQ(back.id, f.id);
        LAB_CHECK_EQ(back.temperature_q8, f.temperature_q8);
        LAB_CHECK_EQ(back.timestamp_ms, f.timestamp_ms);
        LAB_CHECK_EQ(back.crc, f.crc);

        // 容量不够：返回 0，且一个字节都不能写
        std::uint8_t small[8];
        for (std::size_t i = 0; i < sizeof(small); ++i) { small[i] = 0xEEu; }
        LAB_CHECK_EQ(pack_frame(f, small, sizeof(small)), 0u);
        LAB_CHECK_EQ(small[0], 0xEEu);

        // 数据不够：返回 false
        SensorFrame dummy{};
        LAB_CHECK(!unpack_frame(wire, 8, &dummy));
    }

    return LAB_REPORT("第 02 课");
}
