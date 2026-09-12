#pragma once
//
// lab_check —— 极简测试框架
//
// 设计目标：
//   * 只用 <cstdint> / <cstdio>，不依赖第三方库，主机和 MCU 上都能跑；
//   * 断言失败时把 实际值/期望值 打出来，方便自己找错；
//   * 全部 inline，不产生额外的编译单元。
//
// 用法见 lessons/*/tests.cpp。
//

#include <cstdint>
#include <cstdio>
#include <type_traits>

namespace lab {

struct Stats {
    int checks   = 0;
    int failures = 0;
};

inline Stats& stats() {
    static Stats s;
    return s;
}

// ------------------------- 值打印（只在失败时用到） -------------------------
inline void print_value(bool v)               { std::printf("%s", v ? "true" : "false"); }
inline void print_value(char v)               { std::printf("'%c'", v); }
inline void print_value(signed char v)        { std::printf("%d", static_cast<int>(v)); }
inline void print_value(unsigned char v)      { std::printf("%u", static_cast<unsigned>(v)); }
inline void print_value(short v)              { std::printf("%d", static_cast<int>(v)); }
inline void print_value(unsigned short v)     { std::printf("%u", static_cast<unsigned>(v)); }
inline void print_value(int v)                { std::printf("%d", v); }
inline void print_value(unsigned int v)       { std::printf("%u", v); }
inline void print_value(long v)               { std::printf("%ld", v); }
inline void print_value(unsigned long v)      { std::printf("%lu", v); }
inline void print_value(long long v)          { std::printf("%lld", v); }
inline void print_value(unsigned long long v) { std::printf("%llu", v); }
inline void print_value(float v)              { std::printf("%g", static_cast<double>(v)); }
inline void print_value(double v)             { std::printf("%g", v); }
inline void print_value(char* v)              { std::printf("\"%s\"", v ? v : "(null)"); }
inline void print_value(const char* v)        { std::printf("\"%s\"", v ? v : "(null)"); }
inline void print_value(const void* v)        { std::printf("%p", v); }
inline void print_value(const volatile void*) { std::printf("<?>"); }

// 有作用域的枚举（enum class）不会隐式转 int，单独走这一条。
template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
void print_value(T v) {
    std::printf("%lld", static_cast<long long>(v));
}

inline void print_value(...) { std::printf("<?>"); }

// ------------------------------- 断言实现 -------------------------------
inline void check_true(bool ok, const char* expr, const char* file, int line) {
    ++stats().checks;
    if (!ok) {
        ++stats().failures;
        std::printf("  %s:%d\n    断言失败: %s\n", file, line, expr);
    }
}

template <typename A, typename B>
void check_eq(const A& actual, const B& expected, const char* expr, const char* file, int line) {
    ++stats().checks;
    if (!(actual == expected)) {
        ++stats().failures;
        std::printf("  %s:%d\n    断言失败: %s\n      实际值 = ", file, line, expr);
        print_value(actual);
        std::printf("\n      期望值 = ");
        print_value(expected);
        std::printf("\n");
    }
}

inline void check_near(double actual, double expected, double tol,
                       const char* expr, const char* file, int line) {
    ++stats().checks;
    double d = actual - expected;
    if (d < 0) { d = -d; }
    if (!(d <= tol)) {
        ++stats().failures;
        std::printf("  %s:%d\n    断言失败: %s\n      实际值 = %g\n      期望值 = %g (容差 %g)\n",
                    file, line, expr, actual, expected, tol);
    }
}

// 打印结果并给出返回值：全对返回 0，有错返回 1（这样 ctest 能认出失败）。
inline int report(const char* suite) {
    const Stats& s = stats();
    if (s.failures == 0) {
        std::printf("[通过] %s    (%d 项检查全部通过)\n", suite, s.checks);
    } else {
        std::printf("[未通过] %s    (%d 项检查中 %d 项失败)\n", suite, s.checks, s.failures);
    }
    std::fflush(stdout);
    return s.failures == 0 ? 0 : 1;
}

}  // namespace lab

#define LAB_CHECK(cond)         ::lab::check_true((cond), #cond, __FILE__, __LINE__)
#define LAB_CHECK_EQ(a, e)      ::lab::check_eq((a), (e), #a " == " #e, __FILE__, __LINE__)
#define LAB_CHECK_NEAR(a, e, t) ::lab::check_near((double)(a), (double)(e), (double)(t), #a " ~= " #e, __FILE__, __LINE__)
#define LAB_REPORT(suite)       ::lab::report(suite)

