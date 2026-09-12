#pragma once
//
// 第 03 课 · 函数、头文件与编译单元
//
// 契约文件，不要改。
//
#include <cstddef>
#include <cstdint>

namespace lab03 {

// ---------------------------------------------------------------------------
// 1. 函数指针：把"做什么"当成参数传进来
// ---------------------------------------------------------------------------

// BinOp 是一个函数指针类型：指向"收两个 int32_t，返回一个 int32_t"的函数。
using BinOp = std::int32_t (*)(std::int32_t, std::int32_t);

std::int32_t op_add(std::int32_t a, std::int32_t b);
std::int32_t op_sub(std::int32_t a, std::int32_t b);
std::int32_t op_mul(std::int32_t a, std::int32_t b);
std::int32_t op_max(std::int32_t a, std::int32_t b);

// 把 op 指向的函数作用到 a 和 b 上。
std::int32_t apply_binop(BinOp op, std::int32_t a, std::int32_t b);

// ---------------------------------------------------------------------------
// 2. 判断回调
// ---------------------------------------------------------------------------

using Predicate = bool (*)(std::int32_t);

// 在 [first, last) 里找第一个满足 pred 的元素，返回它的地址；没有返回 nullptr。
const std::int32_t* find_first_if(const std::int32_t* first,
                                  const std::int32_t* last,
                                  Predicate pred);

// ---------------------------------------------------------------------------
// 3. 事件回调注册表 —— 嵌入式事件驱动的核心模式
// ---------------------------------------------------------------------------

using EventHandler = void (*)(std::uint8_t event_id, std::int32_t value);

constexpr std::size_t kMaxHandlers = 8u;

// 清空所有注册，回到刚启动的状态。
void event_reset();

// 注册一个回调。表满了返回 false，否则返回 true。
bool event_subscribe(EventHandler handler);

// 当前注册了几个回调。
std::size_t event_handler_count();

// 把事件广播给所有已注册的回调，返回实际调用了几次。
std::size_t event_emit(std::uint8_t event_id, std::int32_t value);

// ---------------------------------------------------------------------------
// 4. 输出参数：用指针把结果"带出来"
// ---------------------------------------------------------------------------

// 安全的整数除法。b == 0 或 out == nullptr 时返回 false，且不写 *out。
bool safe_divide(std::int32_t a, std::int32_t b, std::int32_t* out);

// 一次遍历同时求出最小值和最大值。
// n == 0、out_min/out_max 为 nullptr 时返回 false，且不写输出。
bool min_max(const std::int32_t* data,
             std::size_t n,
             std::int32_t* out_min,
             std::int32_t* out_max);

}  // namespace lab03

// ---------------------------------------------------------------------------
// 5. C 互操作：按 C 的规则命名，方便 C 代码调用
// ---------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

std::int32_t lab03_c_add(std::int32_t a, std::int32_t b);

#ifdef __cplusplus
}
#endif

