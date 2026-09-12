//
// 模块的实现文件。
//
// 注意两件事：
//
//   1. 头文件里只有声明，实现在这里。这样别的 .cpp 能调用它，
//      但链接阶段只会看到这三个导出的符号。
//
//   2. 计数器放进【匿名命名空间】，于是它只在本文件可见，
//      链接器根本不知道它的存在。别人想改也改不了。
//

#include "linkage_demo.h"

namespace {

// 匿名命名空间：仅本编译单元可见。
// 相当于 C 里的 `static std::uint32_t g_counter = 0;`，
// 但 C++ 更推荐这个写法。
//
// 它是个纯 POD 全局变量，初值 0 直接烧进数据段，
// 不涉及任何"运行时的构造函数"，所以没有静态初始化顺序问题。
std::uint32_t g_counter = 0u;

}  // namespace

namespace lab03 {

std::uint32_t counter_next() {
    ++g_counter;
    return g_counter;
}

std::uint32_t counter_peek() {
    return g_counter;
}

void counter_reset() {
    g_counter = 0u;
}

}  // namespace lab03
