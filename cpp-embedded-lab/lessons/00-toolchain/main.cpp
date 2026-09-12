//
// 第 00 课的主程序。
//
// 看重点不在它干了什么，而在它是怎么被构建出来的：
//   * counter.h 只有声明，实现在 counter.cpp
//   * math_util.h 用 extern "C" 包了一层，C/C++ 都能用
//   * 本文件自己不知道任何实现细节，全靠链接器把三块 .obj 拼起来
//

#include <cstdio>

#include "counter.h"
#include "math_util.h"

int main() {
    std::printf("=== 第 00 课：工具链 ===\n\n");

    std::printf("编译期信息：\n");
    std::printf("  __cplusplus = %ldL   (201703 = C++17)\n", static_cast<long>(__cplusplus));
    std::printf("  sizeof(void*) = %zu 字节  (%s)\n",
                sizeof(void*), sizeof(void*) == 4 ? "32 位目标" : "64 位主机");
    std::printf("  sizeof(int32_t) = %zu 字节\n", sizeof(int32_t));
    std::printf("  sizeof(int)     = %zu 字节  <-- 注意它和平台有关，所以别用裸 int\n\n",
                sizeof(int));

    Counter c;
    counter_init(c);
    std::printf("Counter: init -> %u\n", c.value);
    for (int i = 0; i < 3; ++i) {
        std::printf("Counter: next -> %u\n", counter_next(c));
    }
    counter_reset(c);
    std::printf("Counter: reset -> %u\n\n", c.value);

    std::printf("饱和加法（extern \"C\" 函数）：\n");
    std::printf("  100 + 200               = %d\n", math_sat_add_i32(100, 200));
    std::printf("  INT32_MAX + 1           = %d   <-- 钳到上界，而不是回绕\n",
                math_sat_add_i32(2147483647, 1));
    std::printf("  INT32_MIN + (-1)        = %d\n", math_sat_add_i32(-2147483647 - 1, -1));
    std::printf("  abs(INT32_MIN)          = %u\n", math_abs_i32(-2147483647 - 1));

    return 0;
}

