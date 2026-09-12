#pragma once
//
// 第 05 课参考答案（完整版头文件）。
//

#include <array>
#include <cstddef>
#include <cstdint>

namespace lab05 {

// --- 1. clamp_value --------------------------------------------------------
template <typename T>
T clamp_value(T v, T lo, T hi) {
    if (v < lo) { return lo; }
    if (v > hi) { return hi; }
    return v;
}

// --- 2. array_count --------------------------------------------------------
template <typename T, std::size_t N>
constexpr std::size_t array_count(const T (&data)[N]) {
    (void)data;          // 参数只是用来推导 N 的，不需要真的读它
    return N;
}

// --- 3. bit_mask -----------------------------------------------------------
constexpr std::uint32_t bit_mask(unsigned n) {
    if (n >= 32u) {
        return 0xFFFFFFFFu;
    }
    if (n == 0u) {
        return 0u;
    }
    return (1u << n) - 1u;
}

// 编译期自检：把这几行取消注释，就能亲眼看到"编译期检查"。
// static_assert(bit_mask(0u)  == 0x00000000u, "bit_mask(0) 错了");
// static_assert(bit_mask(8u)  == 0x000000FFu, "bit_mask(8) 错了");
// static_assert(bit_mask(32u) == 0xFFFFFFFFu, "bit_mask(32) 错了");

// --- 4. FixedVector --------------------------------------------------------
template <typename T, std::size_t N>
class FixedVector {
    static_assert(N > 0u, "FixedVector 的容量必须大于 0");

public:
    FixedVector()
        : m_data{}, m_size(0u) {
    }

    std::size_t size() const     { return m_size; }
    std::size_t capacity() const { return N; }
    bool empty() const           { return m_size == 0u; }
    bool full() const            { return m_size >= N; }

    bool push_back(const T& value) {
        if (m_size >= N) {
            return false;
        }
        m_data[m_size] = value;
        ++m_size;
        return true;
    }

    void clear() {
        m_size = 0u;
    }

    T& operator[](std::size_t index) {
        return m_data[index];
    }

    const T& operator[](std::size_t index) const {
        return m_data[index];
    }

private:
    std::array<T, N> m_data;
    std::size_t      m_size;
};

// --- 5. array_max ----------------------------------------------------------
template <typename T, std::size_t N>
constexpr T array_max(const T (&data)[N]) {
    static_assert(N > 0u, "array_max 需要至少一个元素");

    T best = data[0];
    for (std::size_t i = 1u; i < N; ++i) {
        if (data[i] > best) {
            best = data[i];
        }
    }
    return best;
}

// --- 对比样例：非模板函数，定义在 exercises.cpp 里 --------------------------
std::uint32_t truncate_to_u8(std::uint32_t value);

}  // namespace lab05
