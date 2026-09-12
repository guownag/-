//
// 第 04 课验收测试。不要改。
//

#include <cstddef>
#include <cstdint>

#include "exercises.h"
#include "lab_check.hpp"

using namespace lab04;

int main() {
    std::printf("=== 第 04 课 · 类与 RAII 生命周期 ===\n");

    // -----------------------------------------------------------------------
    // 1. CriticalSection —— 进入作用域关中断，离开开中断
    // -----------------------------------------------------------------------
    hw_reset();
    LAB_CHECK_EQ(hw().irq_nesting, 0);
    LAB_CHECK_EQ(hw().irq_disable_calls, 0u);
    {
        CriticalSection cs;
        LAB_CHECK_EQ(hw().irq_nesting, 1);
        LAB_CHECK_EQ(hw().irq_disable_calls, 1u);
        LAB_CHECK_EQ(hw().irq_enable_calls, 0u);   // 还没离开，不能开
    }
    LAB_CHECK_EQ(hw().irq_nesting, 0);
    LAB_CHECK_EQ(hw().irq_enable_calls, 1u);

    // 嵌套：只有最外层真的关和开
    hw_reset();
    {
        CriticalSection outer;
        LAB_CHECK_EQ(hw().irq_nesting, 1);
        LAB_CHECK_EQ(hw().irq_disable_calls, 1u);

        {
            CriticalSection inner;
            LAB_CHECK_EQ(hw().irq_nesting, 2);
        }
        LAB_CHECK_EQ(hw().irq_nesting, 1);
        LAB_CHECK_EQ(hw().irq_enable_calls, 0u);   // 内层离开，不该开中断

        {
            CriticalSection inner2;
            LAB_CHECK_EQ(hw().irq_nesting, 2);
        }
        LAB_CHECK_EQ(hw().irq_nesting, 1);
    }
    LAB_CHECK_EQ(hw().irq_nesting, 0);
    LAB_CHECK_EQ(hw().irq_disable_calls, 1u);      // 三次进入，只关了一次
    LAB_CHECK_EQ(hw().irq_enable_calls, 1u);       // 三次离开，只开了一次

    // 顺序：先创建的后析构，计数必须配平
    hw_reset();
    {
        CriticalSection a;
        CriticalSection b;
        LAB_CHECK_EQ(hw().irq_nesting, 2);
        LAB_CHECK_EQ(hw().irq_disable_calls, 1u);  // 只关一次
    }
    LAB_CHECK_EQ(hw().irq_nesting, 0);
    LAB_CHECK_EQ(hw().irq_enable_calls, 1u);       // 只开一次

    // -----------------------------------------------------------------------
    // 2. ChipSelect —— 构造拉低，析构拉高
    // -----------------------------------------------------------------------
    hw_reset();
    LAB_CHECK(!hw().cs_active);
    {
        ChipSelect cs;
        LAB_CHECK(hw().cs_active);                 // 作用域内：片选有效
        LAB_CHECK_EQ(hw().cs_low_calls, 1u);
        LAB_CHECK_EQ(hw().cs_high_calls, 0u);
    }
    LAB_CHECK(!hw().cs_active);                    // 离开后自动释放
    LAB_CHECK_EQ(hw().cs_high_calls, 1u);

    // 连续两次通信，每次都是完整的一对
    hw_reset();
    for (int i = 0; i < 3; ++i) {
        ChipSelect cs;
        LAB_CHECK(hw().cs_active);
    }
    LAB_CHECK(!hw().cs_active);
    LAB_CHECK_EQ(hw().cs_low_calls, 3u);
    LAB_CHECK_EQ(hw().cs_high_calls, 3u);

    // -----------------------------------------------------------------------
    // 3. LifetimeCounter —— 看清拷贝和析构
    // -----------------------------------------------------------------------
    LifetimeCounter::reset_stats();
    LAB_CHECK_EQ(LifetimeCounter::alive, 0);
    {
        LifetimeCounter a;                         // 默认构造
        LAB_CHECK_EQ(LifetimeCounter::constructed, 1);
        LAB_CHECK_EQ(LifetimeCounter::alive, 1);

        LifetimeCounter b = a;                     // 拷贝构造
        LAB_CHECK_EQ(LifetimeCounter::copied, 1);
        LAB_CHECK_EQ(LifetimeCounter::alive, 2);

        LifetimeCounter c;                         // 默认构造
        c = a;                                     // 拷贝赋值：没有新对象
        LAB_CHECK_EQ(LifetimeCounter::constructed, 2);
        LAB_CHECK_EQ(LifetimeCounter::copied, 2);
        LAB_CHECK_EQ(LifetimeCounter::alive, 3);
    }
    LAB_CHECK_EQ(LifetimeCounter::destroyed, 3);   // 三个对象全部析构
    LAB_CHECK_EQ(LifetimeCounter::alive, 0);

    // 再开一个作用域，alive 从 0 重新开始
    {
        LifetimeCounter d;
        LifetimeCounter e;
        LAB_CHECK_EQ(LifetimeCounter::alive, 2);
    }
    LAB_CHECK_EQ(LifetimeCounter::alive, 0);

    // -----------------------------------------------------------------------
    // 4. MovingAverage
    // -----------------------------------------------------------------------
    {
        MovingAverage avg(4u);
        LAB_CHECK_EQ(avg.window(), 4u);
        LAB_CHECK_EQ(avg.count(), 0u);
        LAB_CHECK_EQ(avg.value(), 0);        // 还没样本，不能除以 0

        avg.push(10);
        LAB_CHECK_EQ(avg.count(), 1u);
        LAB_CHECK_EQ(avg.value(), 10);

        avg.push(20);
        LAB_CHECK_EQ(avg.count(), 2u);
        LAB_CHECK_EQ(avg.value(), 15);

        avg.push(30);
        LAB_CHECK_EQ(avg.value(), 20);

        avg.push(40);
        LAB_CHECK_EQ(avg.count(), 4u);
        LAB_CHECK_EQ(avg.value(), 25);       // (10+20+30+40)/4

        // 缓冲满了，最老的 10 被挤出去
        avg.push(50);
        LAB_CHECK_EQ(avg.count(), 4u);       // 数量不再增长
        LAB_CHECK_EQ(avg.value(), 35);       // (20+30+40+50)/4

        avg.push(60);
        LAB_CHECK_EQ(avg.value(), 45);       // (30+40+50+60)/4
    }

    // 窗口大小的钳位
    {
        MovingAverage big(1000u);            // 超过上限
        LAB_CHECK_EQ(big.window(), MovingAverage::kMaxWindow);

        MovingAverage tiny(0u);              // 0 不合法
        LAB_CHECK_EQ(tiny.window(), 1u);
        tiny.push(7);
        LAB_CHECK_EQ(tiny.value(), 7);
    }

    // 窗口大小为 1：每次都等于最后一个样本
    {
        MovingAverage one(1u);
        LAB_CHECK_EQ(one.window(), 1u);
        for (std::int32_t v = 1; v <= 5; ++v) {
            one.push(v);
            LAB_CHECK_EQ(one.value(), v);
        }
        LAB_CHECK_EQ(one.count(), 1u);
    }

    // 负数样本
    {
        MovingAverage neg(2u);
        neg.push(-10);
        neg.push(-20);
        LAB_CHECK_EQ(neg.value(), -15);
        neg.push(0);
        LAB_CHECK_EQ(neg.value(), -10);      // (-20 + 0) / 2
    }

    return LAB_REPORT("第 04 课");
}
