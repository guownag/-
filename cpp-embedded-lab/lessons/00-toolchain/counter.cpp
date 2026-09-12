//
// 一个"定义"的样例。
// 每个非 inline 的函数，在整个程序里只能有这一个定义。
//

#include "counter.h"

// 匿名命名空间里的东西只在本编译单元（本 .cpp）可见，
// 不会导出成链接符号。嵌入式里"内部状态"都这么写。
namespace {

// 全局变量的初始化。注意：C++ 静态对象的构造函数会在 main 之前运行，
// 具体顺序跨编译单元是不确定的 —— 这就是"静态初始化顺序问题"，
// 第 04 课会专门讲。这里用一个纯 POD 常量，没有这个问题。
constexpr uint32_t kInitialStep = 1;

}  // namespace

void counter_init(Counter& c) {
    c.value = 0;
    c.step  = kInitialStep;
}

uint32_t counter_next(Counter& c) {
    c.value += c.step;
    return c.value;
}

void counter_reset(Counter& c) {
    c.value = 0;
}

