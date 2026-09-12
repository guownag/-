//
// 第 03 课参考答案。
//

#include "exercises.h"

// 文件级状态：匿名命名空间，链接器看不见。
namespace {

lab03::EventHandler g_handlers[lab03::kMaxHandlers] = {};
std::size_t         g_handler_count = 0u;

}  // namespace

namespace lab03 {

std::int32_t op_add(std::int32_t a, std::int32_t b) { return a + b; }
std::int32_t op_sub(std::int32_t a, std::int32_t b) { return a - b; }
std::int32_t op_mul(std::int32_t a, std::int32_t b) { return a * b; }

std::int32_t op_max(std::int32_t a, std::int32_t b) {
    return (a > b) ? a : b;
}

std::int32_t apply_binop(BinOp op, std::int32_t a, std::int32_t b) {
    if (op == nullptr) {
        return 0;
    }
    return op(a, b);        // 和调用普通函数一模一样
}

const std::int32_t* find_first_if(const std::int32_t* first,
                                  const std::int32_t* last,
                                  Predicate pred) {
    if (pred == nullptr) {
        return nullptr;
    }
    for (const std::int32_t* p = first; p != last; ++p) {
        if (pred(*p)) {
            return p;
        }
    }
    return nullptr;
}

void event_reset() {
    // 只需要把计数归零。数组里残留的旧指针无所谓 ——
    // g_handler_count 是唯一的事实来源，超过它的位置不会被读。
    g_handler_count = 0u;
}

bool event_subscribe(EventHandler handler) {
    if (handler == nullptr) {
        return false;
    }
    if (g_handler_count >= kMaxHandlers) {   // 先检查容量，再写
        return false;
    }
    g_handlers[g_handler_count] = handler;
    ++g_handler_count;
    return true;
}

std::size_t event_handler_count() {
    return g_handler_count;
}

std::size_t event_emit(std::uint8_t event_id, std::int32_t value) {
    std::size_t called = 0u;
    for (std::size_t i = 0u; i < g_handler_count; ++i) {
        g_handlers[i](event_id, value);
        ++called;
    }
    return called;
}

bool safe_divide(std::int32_t a, std::int32_t b, std::int32_t* out) {
    if (out == nullptr) {
        return false;
    }
    if (b == 0) {
        return false;
    }
    *out = a / b;
    return true;
}

bool min_max(const std::int32_t* data,
             std::size_t n,
             std::int32_t* out_min,
             std::int32_t* out_max) {
    if (data == nullptr || out_min == nullptr || out_max == nullptr) {
        return false;
    }
    if (n == 0u) {
        return false;
    }

    // 用第一个元素初始化，而不是用哨兵值 —— 永远正确。
    std::int32_t lo = data[0];
    std::int32_t hi = data[0];

    for (std::size_t i = 1u; i < n; ++i) {
        if (data[i] < lo) { lo = data[i]; }
        if (data[i] > hi) { hi = data[i]; }
    }

    *out_min = lo;
    *out_max = hi;
    return true;
}

}  // namespace lab03

extern "C" std::int32_t lab03_c_add(std::int32_t a, std::int32_t b) {
    return a + b;
}
