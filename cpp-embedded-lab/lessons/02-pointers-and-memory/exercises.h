#pragma once
//
// 第 02 课 · 指针、引用与内存布局
//
// 契约文件，不要改。
//

#include <cstddef>
#include <cstdint>

namespace lab02 {

// 1. 就地把数组倒过来
//
// data 指向 n 个 int32_t。不允许申请新内存，只能用指针/下标交换元素。
// 这是双指针（头尾夹逼）的经典练习，也是写 DMA 缓冲区翻转时的手感训练。
//
// n == 0 或 1 时什么都不做。
void reverse_in_place(std::int32_t* data, std::size_t n);

// 2. 端序（字节序）
//
// 协议里多字节整数的字节顺序是定死的，跟你的 CPU 无关。
// 所以你永远不能说"直接 memcpy 到结构体里就行" —— 那样换个大端芯片就全错。
//
// read_u32_le: 从 p 读 4 个字节，按小端解释。
//   小端 = 最低有效字节在前。0x12345678 在内存里是 78 56 34 12。
// read_u32_be: 按大端解释（网络字节序，很多工业协议用它）。
//
// 要求：用字节移位拼出来，不要用 memcpy，不要依赖平台端序，
//      也不要假设 p 是 4 字节对齐的。
std::uint32_t read_u32_le(const std::uint8_t* p);
std::uint32_t read_u32_be(const std::uint8_t* p);

void write_u32_le(std::uint8_t* p, std::uint32_t v);
void write_u32_be(std::uint8_t* p, std::uint32_t v);

// 3. 运行时判断当前平台的端序
//
// 返回 true 表示小端（x86 / ARM Cortex-M 默认都是小端）。
// 提示：取一个已知值的变量的第一个字节，看它是高位还是低位。
bool is_little_endian();

// 4. 在内存区间里找字节（半开区间 [first, last)）
//
// 找到就返回指向它的指针，没找到返回 nullptr。
//
// 为什么要传 last 而不是长度？这是 C++ 标准库的迭代器风格，
// 在嵌入式里也很实用：你可以直接传 DMA 缓冲区的起止地址。
const std::uint8_t* find_byte(const std::uint8_t* first,
                              const std::uint8_t* last,
                              std::uint8_t value);

// 5. 寄存器位操作（volatile 的正确用法）
//
// 这三个函数直接操作"硬件寄存器"。用 volatile 指针调用它们，
// 保证编译器不会把读写优化掉或合并 —— 硬件寄存器有副作用，
// 读一次和读两次的结果可能不一样。
//
// 注意参数是 `volatile std::uint32_t*`，不是 `std::uint32_t*`。
// 不要用 const_cast 强行转换来绕过类型系统。
void reg_set_bits(volatile std::uint32_t* reg, std::uint32_t mask);
void reg_clear_bits(volatile std::uint32_t* reg, std::uint32_t mask);
void reg_toggle_bits(volatile std::uint32_t* reg, std::uint32_t mask);

// 6. 结构体布局与协议序列化
//
// 内存里的结构体布局 ≠ 协议里的字节布局。原因有两个：
//   * 对齐填充（padding）：编译器会在成员之间塞空洞，让每个成员对齐到自己的边界
//   * 端序：多字节整数在内存里的顺序取决于 CPU
//
// SensorFrame 是"内存里"的表示，成员顺序是自然的（可读性优先）。
struct SensorFrame {
    std::uint16_t id;
    std::int16_t  temperature_q8;   // 实际温度 = 值 / 256.0
    std::uint32_t timestamp_ms;
    std::uint8_t  crc;
};

// 协议里规定的线上格式（固定 9 字节，全部小端）：
//   偏移 0..1 : id             uint16 LE
//   偏移 2..3 : temperature    int16  LE
//   偏移 4..7 : timestamp_ms   uint32 LE
//   偏移 8    : crc            uint8
//
// pack_frame: 把 f 序列化到 out（最多 cap 字节）。
//   成功返回写出的字节数（9），容量不够返回 0 且不写任何东西。
//
// unpack_frame: 从 in（长度 len）反序列化到 *out。
//   成功返回 true；len < 9 返回 false。
//
// 要求：out / in 不保证对齐，所以不能直接对它们做 *reinterpret_cast<uint16_t*>(p)。
//      用第 2 题的读写函数逐字段处理。
constexpr std::size_t kFrameWireSize = 9;

std::size_t pack_frame(const SensorFrame& f, std::uint8_t* out, std::size_t cap);
bool unpack_frame(const std::uint8_t* in, std::size_t len, SensorFrame* out);

}  // namespace lab02
