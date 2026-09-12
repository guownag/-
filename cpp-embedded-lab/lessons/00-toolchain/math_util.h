#pragma once
//
// 这个头文件演示"同时给 C 和 C++ 用"的写法。
// 嵌入式里你调用厂家 HAL 库、RTOS 的 API 时，天天看到这个模式。
//

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// 饱和加法：结果超过 int32_t 表示范围时，钳到边界而不是回绕。
// 嵌入式里大量使用这种"饱和"语义，因为回绕导致的错误极难排查。
int32_t math_sat_add_i32(int32_t a, int32_t b);

// 整数绝对值（不用 <cstdlib> 的 abs，避免某些工具链上的重载歧义）。
uint32_t math_abs_i32(int32_t v);

#ifdef __cplusplus
}  // extern "C"
#endif

