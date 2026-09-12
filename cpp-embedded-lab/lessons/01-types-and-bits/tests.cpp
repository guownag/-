//
// 第 01 课的验收测试。不要改这个文件 —— 改了就是自己骗自己。
//
// 每一条断言都在描述一个"必须成立的事实"。仔细读右边的期望值，
// 它们本身就是题目描述的一部分。
//

#include <cstdint>

#include "exercises.h"
#include "lab_check.hpp"

using namespace lab01;

int main() {
    std::printf("=== 第 01 课 · 类型、位运算与常量 ===\n");

    // 1. make_mask
    LAB_CHECK_EQ(make_mask(0), 0x00000000u);
    LAB_CHECK_EQ(make_mask(1), 0x00000001u);
    LAB_CHECK_EQ(make_mask(4), 0x0000000Fu);
    LAB_CHECK_EQ(make_mask(8), 0x000000FFu);
    LAB_CHECK_EQ(make_mask(16), 0x0000FFFFu);
    LAB_CHECK_EQ(make_mask(31), 0x7FFFFFFFu);
    LAB_CHECK_EQ(make_mask(32), 0xFFFFFFFFu);  // <- 这一条就是那个 UB 陷阱

    // 2. insert_field
    LAB_CHECK_EQ(insert_field(0x00000000u, 0, 1, 1u), 0x00000001u);
    LAB_CHECK_EQ(insert_field(0x00000000u, 8, 8, 0xABu), 0x0000AB00u);
    LAB_CHECK_EQ(insert_field(0xFFFFFFFFu, 4, 4, 0u), 0xFFFFFF0Fu);
    LAB_CHECK_EQ(insert_field(0x12345678u, 0, 8, 0xFFu), 0x123456FFu);
    LAB_CHECK_EQ(insert_field(0x12345678u, 16, 8, 0xAAu), 0x12AA5678u);
    LAB_CHECK_EQ(insert_field(0x00000000u, 0, 4, 0xFFu), 0x0000000Fu);
    LAB_CHECK_EQ(insert_field(0xFFFFFFFFu, 0, 32, 0xDEADBEEFu), 0xDEADBEEFu);
    LAB_CHECK_EQ(insert_field(0x12345678u, 28, 4, 0x0Au), 0xA2345678u);

    // 3. extract_field
    LAB_CHECK_EQ(extract_field(0x12345678u, 0, 8), 0x00000078u);
    LAB_CHECK_EQ(extract_field(0x12345678u, 8, 8), 0x00000056u);
    LAB_CHECK_EQ(extract_field(0x12345678u, 24, 8), 0x00000012u);
    LAB_CHECK_EQ(extract_field(0x12345678u, 4, 8), 0x00000067u);
    LAB_CHECK_EQ(extract_field(0xFFFFFFFFu, 31, 1), 0x00000001u);
    LAB_CHECK_EQ(extract_field(0xFFFFFFFFu, 0, 32), 0xFFFFFFFFu);
    LAB_CHECK_EQ(extract_field(0x00000000u, 8, 8), 0x00000000u);

    // insert 和 extract 互为逆运算
    {
        const std::uint32_t original = 0xDEADBEEFu;
        const std::uint32_t written  = insert_field(original, 8, 8, 0x5Au);
        LAB_CHECK_EQ(extract_field(written, 8, 8), 0x0000005Au);
        LAB_CHECK_EQ(extract_field(written, 0, 8), 0x000000EFu);
    }

    // 4. popcount32
    LAB_CHECK_EQ(popcount32(0x00000000u), 0u);
    LAB_CHECK_EQ(popcount32(0x00000001u), 1u);
    LAB_CHECK_EQ(popcount32(0x80000001u), 2u);
    LAB_CHECK_EQ(popcount32(0x0000FFFFu), 16u);
    LAB_CHECK_EQ(popcount32(0x55555555u), 16u);
    LAB_CHECK_EQ(popcount32(0xFFFFFFFFu), 32u);
    LAB_CHECK_EQ(popcount32(0x0000000Fu), 4u);

    // 5. clamp_i32
    LAB_CHECK_EQ(clamp_i32(5, 0, 10), 5);
    LAB_CHECK_EQ(clamp_i32(-5, 0, 10), 0);
    LAB_CHECK_EQ(clamp_i32(50, 0, 10), 10);
    LAB_CHECK_EQ(clamp_i32(0, 0, 10), 0);
    LAB_CHECK_EQ(clamp_i32(10, 0, 10), 10);
    LAB_CHECK_EQ(clamp_i32(-2147483647 - 1, 0, 10), 0);
    LAB_CHECK_EQ(clamp_i32(2147483647, 0, 10), 10);

    // 6. sat_add_i16
    LAB_CHECK_EQ(sat_add_i16(100, 200), 300);
    LAB_CHECK_EQ(sat_add_i16(0, 0), 0);
    LAB_CHECK_EQ(sat_add_i16(-100, -200), -300);
    LAB_CHECK_EQ(sat_add_i16(32767, 1), 32767);
    LAB_CHECK_EQ(sat_add_i16(30000, 30000), 32767);
    LAB_CHECK_EQ(sat_add_i16(-32768, -1), -32768);
    LAB_CHECK_EQ(sat_add_i16(-32768, 32767), -1);
    LAB_CHECK_EQ(sat_add_i16(32767, -32768), -1);

    // 7. q15_mul
    LAB_CHECK_EQ(q15_mul(16384, 16384), 8192);
    LAB_CHECK_EQ(q15_mul(16384, 0), 0);
    LAB_CHECK_EQ(q15_mul(32767, 16384), 16384);
    LAB_CHECK_EQ(q15_mul(-16384, 16384), -8192);
    LAB_CHECK_EQ(q15_mul(32767, 32767), 32766);

    return LAB_REPORT("第 01 课");
}
