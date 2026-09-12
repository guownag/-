//
// 第 03 课验收测试。不要改。
//

#include <cstddef>
#include <cstdint>

#include "exercises.h"
#include "lab_check.hpp"
#include "linkage_demo.h"

using namespace lab03;

// ------------------------- 测试用的辅助函数和计数 -------------------------
namespace {

std::uint32_t g_a_calls = 0;
std::uint32_t g_b_calls = 0;
std::int32_t  g_last_id = -1;
std::int32_t  g_last_value = 0;
std::int32_t  g_sum = 0;

void handler_a(std::uint8_t id, std::int32_t value) {
    ++g_a_calls;
    g_last_id = static_cast<std::int32_t>(id);
    g_last_value = value;
    g_sum += value;
}

void handler_b(std::uint8_t id, std::int32_t value) {
    (void)id;
    ++g_b_calls;
    g_sum += value * 2;
}

void reset_counters() {
    g_a_calls = 0;
    g_b_calls = 0;
    g_last_id = -1;
    g_last_value = 0;
    g_sum = 0;
}

bool is_negative(std::int32_t v) { return v < 0; }
bool is_odd(std::int32_t v) { return (v & 1) != 0; }
bool is_greater_than_100(std::int32_t v) { return v > 100; }

}  // namespace

int main() {
    std::printf("=== 第 03 课 · 函数、头文件与编译单元 ===\n");

    // -----------------------------------------------------------------------
    // 1. 函数指针
    // -----------------------------------------------------------------------
    LAB_CHECK_EQ(op_add(3, 4), 7);
    LAB_CHECK_EQ(op_add(-3, 4), 1);
    LAB_CHECK_EQ(op_sub(10, 4), 6);
    LAB_CHECK_EQ(op_sub(4, 10), -6);
    LAB_CHECK_EQ(op_mul(6, 7), 42);
    LAB_CHECK_EQ(op_mul(-6, 7), -42);
    LAB_CHECK_EQ(op_max(3, 9), 9);
    LAB_CHECK_EQ(op_max(9, 3), 9);
    LAB_CHECK_EQ(op_max(5, 5), 5);

    // 同一个 apply_binop，传不同的函数进去，行为完全不同
    LAB_CHECK_EQ(apply_binop(op_add, 10, 3), 13);
    LAB_CHECK_EQ(apply_binop(op_sub, 10, 3), 7);
    LAB_CHECK_EQ(apply_binop(op_mul, 10, 3), 30);
    LAB_CHECK_EQ(apply_binop(op_max, 10, 3), 10);

    // 先把函数存进变量再传进去，效果一样
    {
        BinOp fn = op_add;              // 不需要写 &op_add
        LAB_CHECK_EQ(apply_binop(fn, 1, 2), 3);
        BinOp fn2 = &op_sub;
        LAB_CHECK_EQ(apply_binop(fn2, 1, 2), -1);
    }

    // 空函数指针不能崩
    LAB_CHECK_EQ(apply_binop(nullptr, 1, 2), 0);

    // -----------------------------------------------------------------------
    // 2. find_first_if
    // -----------------------------------------------------------------------
    {
        const std::int32_t data[6] = {4, 7, -2, 9, -5, 12};

        LAB_CHECK(find_first_if(data, data + 6, is_negative) == data + 2);
        LAB_CHECK(find_first_if(data, data + 6, is_odd) == data + 1);
        LAB_CHECK(find_first_if(data, data + 6, is_greater_than_100) == nullptr);

        // 空区间
        LAB_CHECK(find_first_if(data, data, is_odd) == nullptr);

        // last 是开区间：data[2] 是负数，但区间只到 data+2 就取不到
        LAB_CHECK(find_first_if(data, data + 2, is_negative) == nullptr);

        // 空函数指针不能崩
        LAB_CHECK(find_first_if(data, data + 6, nullptr) == nullptr);
    }

    // -----------------------------------------------------------------------
    // 3. 事件回调注册表
    // -----------------------------------------------------------------------
    event_reset();
    LAB_CHECK_EQ(event_handler_count(), 0u);
    LAB_CHECK_EQ(event_emit(1, 100), 0u);      // 一个都没注册，不能崩

    reset_counters();
    LAB_CHECK(event_subscribe(handler_a));
    LAB_CHECK_EQ(event_handler_count(), 1u);
    LAB_CHECK_EQ(event_emit(7, 42), 1u);
    LAB_CHECK_EQ(g_a_calls, 1u);
    LAB_CHECK_EQ(g_last_id, 7);
    LAB_CHECK_EQ(g_last_value, 42);
    LAB_CHECK_EQ(g_sum, 42);

    // 注册第二个，两个都会被调用
    reset_counters();
    LAB_CHECK(event_subscribe(handler_b));
    LAB_CHECK_EQ(event_handler_count(), 2u);
    LAB_CHECK_EQ(event_emit(3, 10), 2u);
    LAB_CHECK_EQ(g_a_calls, 1u);
    LAB_CHECK_EQ(g_b_calls, 1u);
    LAB_CHECK_EQ(g_sum, 30);                   // a 加 10，b 加 20

    // 容量上限：注册满 8 个之后，第 9 个必须被拒绝
    event_reset();
    for (std::size_t i = 0; i < kMaxHandlers; ++i) {
        LAB_CHECK(event_subscribe(handler_a));
    }
    LAB_CHECK_EQ(event_handler_count(), kMaxHandlers);
    LAB_CHECK(!event_subscribe(handler_b));     // 第 9 个，拒绝
    LAB_CHECK_EQ(event_handler_count(), kMaxHandlers);

    // 被拒绝之后，前面 8 个必须完好无损
    reset_counters();
    LAB_CHECK_EQ(event_emit(1, 1), kMaxHandlers);
    LAB_CHECK_EQ(g_a_calls, kMaxHandlers);
    LAB_CHECK_EQ(g_b_calls, 0u);

    // reset 之后应该彻底干净
    event_reset();
    LAB_CHECK_EQ(event_handler_count(), 0u);
    LAB_CHECK_EQ(event_emit(1, 1), 0u);

    // -----------------------------------------------------------------------
    // 4. safe_divide —— 输出参数
    // -----------------------------------------------------------------------
    {
        std::int32_t result = 12345;               // 故意给个"旧值"

        LAB_CHECK(safe_divide(10, 2, &result));
        LAB_CHECK_EQ(result, 5);

        LAB_CHECK(safe_divide(-9, 3, &result));
        LAB_CHECK_EQ(result, -3);

        // 除数为 0：返回 false，而且 result 必须保持原样
        result = 12345;
        LAB_CHECK(!safe_divide(10, 0, &result));
        LAB_CHECK_EQ(result, 12345);

        // 输出指针为空：返回 false，不能崩
        LAB_CHECK(!safe_divide(10, 2, nullptr));

        // 两个都坏：同样不能崩
        LAB_CHECK(!safe_divide(10, 0, nullptr));
    }

    // -----------------------------------------------------------------------
    // 5. min_max —— 一次遍历出两个结果
    // -----------------------------------------------------------------------
    {
        const std::int32_t data[6] = {4, -7, 12, 0, -3, 9};
        std::int32_t lo = 777;
        std::int32_t hi = 777;

        LAB_CHECK(min_max(data, 6, &lo, &hi));
        LAB_CHECK_EQ(lo, -7);
        LAB_CHECK_EQ(hi, 12);

        // 只比较大小，不能把中间的值漏掉
        const std::int32_t data2[3] = {5, 5, 5};
        lo = 0;
        hi = 0;
        LAB_CHECK(min_max(data2, 3, &lo, &hi));
        LAB_CHECK_EQ(lo, 5);
        LAB_CHECK_EQ(hi, 5);

        // 只有一个元素
        const std::int32_t one[1] = {-42};
        lo = 0;
        hi = 0;
        LAB_CHECK(min_max(one, 1, &lo, &hi));
        LAB_CHECK_EQ(lo, -42);
        LAB_CHECK_EQ(hi, -42);

        // 全部是负数
        const std::int32_t neg[3] = {-5, -20, -1};
        lo = 0;
        hi = 0;
        LAB_CHECK(min_max(neg, 3, &lo, &hi));
        LAB_CHECK_EQ(lo, -20);
        LAB_CHECK_EQ(hi, -1);

        // 失败的情况：一个字节都不能写
        lo = 111;
        hi = 222;
        LAB_CHECK(!min_max(data, 0, &lo, &hi));      // n == 0
        LAB_CHECK_EQ(lo, 111);
        LAB_CHECK_EQ(hi, 222);

        LAB_CHECK(!min_max(nullptr, 6, &lo, &hi));   // data 为空
        LAB_CHECK_EQ(lo, 111);
        LAB_CHECK_EQ(hi, 222);

        LAB_CHECK(!min_max(data, 6, nullptr, &hi));  // 输出指针为空
        LAB_CHECK(!min_max(data, 6, &lo, nullptr));
        LAB_CHECK_EQ(lo, 111);
        LAB_CHECK_EQ(hi, 222);
    }

    // -----------------------------------------------------------------------
    // 6. extern "C"
    // -----------------------------------------------------------------------
    LAB_CHECK_EQ(lab03_c_add(3, 4), 7);
    LAB_CHECK_EQ(lab03_c_add(-10, 4), -6);

    // -----------------------------------------------------------------------
    // 7. 模块与内部链接（实现在 linkage_demo.cpp 里）
    // -----------------------------------------------------------------------
    //
    // 这个计数器藏在 linkage_demo.cpp 的匿名命名空间里，外部看不见。
    // 但通过这三个函数，行为是完全可以验证的。
    counter_reset();
    LAB_CHECK_EQ(counter_peek(), 0u);
    LAB_CHECK_EQ(counter_next(), 1u);
    LAB_CHECK_EQ(counter_next(), 2u);
    LAB_CHECK_EQ(counter_next(), 3u);
    LAB_CHECK_EQ(counter_peek(), 3u);       // peek 不改变状态
    LAB_CHECK_EQ(counter_peek(), 3u);
    counter_reset();
    LAB_CHECK_EQ(counter_peek(), 0u);
    LAB_CHECK_EQ(counter_next(), 1u);

    return LAB_REPORT("第 03 课");
}
