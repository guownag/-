#include "math_util.h"

#include <limits>

extern "C" int32_t math_sat_add_i32(int32_t a, int32_t b) {
    // 注意：不能写 if (a + b > INT32_MAX) —— a+b 本身就已经溢出了，
    // 有符号整数溢出在 C++ 里是未定义行为(UB)，优化器可以让你的判断完全失效。
    // 正确做法是先转成更宽的类型，或者用"同号才可能溢出"的逻辑判断。
    const int64_t sum = static_cast<int64_t>(a) + static_cast<int64_t>(b);

    if (sum > static_cast<int64_t>(std::numeric_limits<int32_t>::max())) {
        return std::numeric_limits<int32_t>::max();
    }
    if (sum < static_cast<int64_t>(std::numeric_limits<int32_t>::min())) {
        return std::numeric_limits<int32_t>::min();
    }
    return static_cast<int32_t>(sum);
}

extern "C" uint32_t math_abs_i32(int32_t v) {
    // 为什么不能直接 return (uint32_t)(v < 0 ? -v : v);
    // 因为 v == INT32_MIN 时 -v 就已经溢出（UB）了。
    if (v >= 0) {
        return static_cast<uint32_t>(v);
    }
    // 先转成 int64_t 再取负，安全。
    return static_cast<uint32_t>(-static_cast<int64_t>(v));
}

