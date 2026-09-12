#pragma once
//
// 第 03 课的第二个模块。
//
// 这个头文件只有三个函数的声明，它们实现在 linkage_demo.cpp 里。
// 模块内部用一个"别人看不见"的计数器记状态 —— 这就是本课要讲的
// 【内部链接】。翻到 linkage_demo.cpp 看它是怎么藏的。
//

#include <cstdint>

namespace lab03 {

// 计数器加一，返回加完之后的值。
std::uint32_t counter_next();

// 看一眼当前值，不改变它。
std::uint32_t counter_peek();

// 归零。
void counter_reset();

}  // namespace lab03

