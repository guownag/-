#pragma once
//
// 第 04 课 · 类与 RAII 生命周期
//
// 契约文件，不要改。
//
#include <cstddef>
#include <cstdint>

namespace lab04 {

// ---------------------------------------------------------------------------
// 一个极简的"硬件模型"，测试用它来观察副作用。
//
// 真实的 MCU 上这些会变成寄存器操作；这里用几个计数器代替，
// 这样在 PC 上也能验证"中断到底关了没有"。
// ---------------------------------------------------------------------------
struct TestHw {
    std::uint32_t irq_disable_calls = 0;
    std::uint32_t irq_enable_calls  = 0;
    std::int32_t  irq_nesting       = 0;    // > 0 表示中断正被关着
    std::uint32_t cs_low_calls      = 0;
    std::uint32_t cs_high_calls     = 0;
    bool          cs_active         = false;
};

// 全局唯一的硬件实例。实现已经给你写好了（见 exercises.cpp），直接用。
TestHw& hw();
void    hw_reset();

// ---------------------------------------------------------------------------
// 1. RAII 临界区
//
// 构造时关中断，析构时开中断。支持嵌套：
// 只有最外层进入时才真的关，只有最外层离开时才真的开。
// ---------------------------------------------------------------------------
class CriticalSection {
public:
    CriticalSection();
    ~CriticalSection();

    // 禁止拷贝 —— 否则两个对象会各自"开一次中断"，计数就乱了。
    CriticalSection(const CriticalSection&)            = delete;
    CriticalSection& operator=(const CriticalSection&) = delete;
};

// ---------------------------------------------------------------------------
// 2. RAII 片选
//
// 构造时把片选拉低（选中器件），析构时拉高（释放）。
// 这是 SPI 通信的标准写法。
// ---------------------------------------------------------------------------
class ChipSelect {
public:
    ChipSelect();
    ~ChipSelect();

    ChipSelect(const ChipSelect&)            = delete;
    ChipSelect& operator=(const ChipSelect&) = delete;
};

// ---------------------------------------------------------------------------
// 3. 生命周期计数器
//
// 统计构造函数、拷贝、析构各被调用了多少次，以及当前活着几个对象。
// 这个类没什么实际用途，它的价值是让你【看见】编译器什么时候
// 悄悄调用了拷贝构造或析构。
// ---------------------------------------------------------------------------
struct LifetimeCounter {
    static std::int32_t alive;        // 当前活着的对象数
    static std::int32_t constructed;  // 默认构造次数
    static std::int32_t copied;       // 拷贝构造 + 拷贝赋值次数
    static std::int32_t destroyed;    // 析构次数

    LifetimeCounter();
    LifetimeCounter(const LifetimeCounter& other);
    LifetimeCounter& operator=(const LifetimeCounter& other);
    ~LifetimeCounter();

    static void reset_stats();
};

// ---------------------------------------------------------------------------
// 4. 滑动平均滤波器
//
// 嵌入式里最常见的数字滤波之一：保留最近 N 个采样，输出它们的平均值。
// 内部用环形缓冲实现，固定容量，不做任何动态分配。
//
// window 会被限制在 [1, kMaxWindow] 之间。
// ---------------------------------------------------------------------------
class MovingAverage {
public:
    static constexpr std::size_t kMaxWindow = 32u;

    explicit MovingAverage(std::size_t window);

    void push(std::int32_t sample);

    std::int32_t value() const;   // 当前平均值；还没有样本时返回 0
    std::size_t  count() const;   // 已经收了多少个样本（最多 window 个）
    std::size_t  window() const;  // 实际生效的窗口大小

private:
    std::int32_t m_buf[kMaxWindow];
    std::int32_t m_sum;
    std::size_t  m_window;
    std::size_t  m_count;
    std::size_t  m_index;
};

}  // namespace lab04

