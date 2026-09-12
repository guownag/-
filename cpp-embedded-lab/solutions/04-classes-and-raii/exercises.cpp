//
// 第 04 课参考答案。
//

#include "exercises.h"

namespace lab04 {

namespace {
TestHw g_hw;
}  // namespace

TestHw& hw() {
    return g_hw;
}

void hw_reset() {
    g_hw = TestHw{};
}

// --- 1. CriticalSection ----------------------------------------------------
CriticalSection::CriticalSection() {
    // 只有从"没关"变成"关"的那一次才真的关中断。
    if (hw().irq_nesting == 0) {
        ++hw().irq_disable_calls;
    }
    ++hw().irq_nesting;
}

CriticalSection::~CriticalSection() {
    --hw().irq_nesting;
    // 只有回到 0 的时候才真的开中断 —— 这是嵌套能正确工作的关键。
    if (hw().irq_nesting == 0) {
        ++hw().irq_enable_calls;
    }
}

// --- 2. ChipSelect --------------------------------------------------------
ChipSelect::ChipSelect() {
    ++hw().cs_low_calls;
    hw().cs_active = true;
}

ChipSelect::~ChipSelect() {
    ++hw().cs_high_calls;
    hw().cs_active = false;
}

// --- 3. LifetimeCounter ---------------------------------------------------
std::int32_t LifetimeCounter::alive       = 0;
std::int32_t LifetimeCounter::constructed = 0;
std::int32_t LifetimeCounter::copied      = 0;
std::int32_t LifetimeCounter::destroyed   = 0;

LifetimeCounter::LifetimeCounter() {
    ++constructed;
    ++alive;
}

LifetimeCounter::LifetimeCounter(const LifetimeCounter& other) {
    (void)other;          // 没有成员要抄，只是统计
    ++copied;
    ++alive;              // 新对象诞生了
}

LifetimeCounter& LifetimeCounter::operator=(const LifetimeCounter& other) {
    if (this != &other) {
        ++copied;         // 赋值不产生新对象，所以 alive 不变
    }
    return *this;
}

LifetimeCounter::~LifetimeCounter() {
    ++destroyed;
    --alive;
}

void LifetimeCounter::reset_stats() {
    alive       = 0;
    constructed = 0;
    copied      = 0;
    destroyed   = 0;
}

// --- 4. MovingAverage -----------------------------------------------------
MovingAverage::MovingAverage(std::size_t window)
    : m_buf{}, m_sum(0), m_window(1u), m_count(0u), m_index(0u) {
    std::size_t w = window;
    if (w == 0u) {
        w = 1u;
    }
    if (w > kMaxWindow) {
        w = kMaxWindow;
    }
    m_window = w;
}

void MovingAverage::push(std::int32_t sample) {
    if (m_count < m_window) {
        // 缓冲还没满：直接放进去
        m_buf[m_index] = sample;
        m_sum += sample;
        ++m_count;
    } else {
        // 已经满了：先把被覆盖的旧值从和里减掉，再放入新值
        m_sum -= m_buf[m_index];
        m_buf[m_index] = sample;
        m_sum += sample;
    }

    // 写指针往前走一格，到末尾就绕回开头
    m_index = (m_index + 1u) % m_window;
}

std::int32_t MovingAverage::value() const {
    if (m_count == 0u) {
        return 0;         // 还没样本，不能除以 0
    }
    return m_sum / static_cast<std::int32_t>(m_count);
}

std::size_t MovingAverage::count() const {
    return m_count;
}

std::size_t MovingAverage::window() const {
    return m_window;
}

}  // namespace lab04
