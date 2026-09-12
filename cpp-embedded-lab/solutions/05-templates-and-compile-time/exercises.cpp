//
// 第 05 课参考答案（.cpp 部分）。
//

#include "exercises.h"

namespace lab05 {

std::uint32_t truncate_to_u8(std::uint32_t value) {
    return value & 0xFFu;
}

}  // namespace lab05
