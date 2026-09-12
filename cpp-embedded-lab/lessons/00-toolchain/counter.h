#pragma once
//
// 一个"声明"的样例：
// 头文件里只放函数声明、类型、常量，不放函数定义（函数体）。
// 因为头文件会被多个 .cpp 包含，定义放这里就会违反 ODR。
//

#include <cstdint>

// 计数器状态。用 struct 把相关的东西打包，而不是散落成多个全局变量。
struct Counter {
    uint32_t value;
    uint32_t step;
};

// 初始化：把 value 清零、step 设为 1。
void counter_init(Counter& c);

// 走一步，返回值是走完之后的值。
uint32_t counter_next(Counter& c);

// 复位。
void counter_reset(Counter& c);

