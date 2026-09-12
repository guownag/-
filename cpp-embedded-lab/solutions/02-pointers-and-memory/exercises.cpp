//
// 第 02 课参考答案。
//

#include "exercises.h"

namespace lab02 {

namespace {

// 16 位的辅助函数。协议里 16 位字段也很常见，自己写一份不难。
void put_u16_le(std::uint8_t* p, std::uint16_t v) {
    p[0] = static_cast<std::uint8_t>(v & 0xFFu);
    p[1] = static_cast<std::uint8_t>((v >> 8) & 0xFFu);
}

std::uint16_t get_u16_le(const std::uint8_t* p) {
    return static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(p[0]) |
        static_cast<std::uint16_t>(static_cast<std::uint16_t>(p[1]) << 8));
}

}  // namespace

void reverse_in_place(std::int32_t* data, std::size_t n) {
    // n < 2 时不用动；顺便挡住 nullptr，避免空指针解引用。
    if (data == nullptr || n < 2u) {
        return;
    }
    std::size_t i = 0;
    std::size_t j = n - 1u;
    while (i < j) {
        const std::int32_t tmp = data[i];
        data[i] = data[j];
        data[j] = tmp;
        ++i;
        --j;
    }
}

std::uint32_t read_u32_le(const std::uint8_t* p) {
    return static_cast<std::uint32_t>(p[0]) |
           (static_cast<std::uint32_t>(p[1]) << 8) |
           (static_cast<std::uint32_t>(p[2]) << 16) |
           (static_cast<std::uint32_t>(p[3]) << 24);
}

std::uint32_t read_u32_be(const std::uint8_t* p) {
    return (static_cast<std::uint32_t>(p[0]) << 24) |
           (static_cast<std::uint32_t>(p[1]) << 16) |
           (static_cast<std::uint32_t>(p[2]) << 8) |
           static_cast<std::uint32_t>(p[3]);
}

void write_u32_le(std::uint8_t* p, std::uint32_t v) {
    p[0] = static_cast<std::uint8_t>(v & 0xFFu);
    p[1] = static_cast<std::uint8_t>((v >> 8) & 0xFFu);
    p[2] = static_cast<std::uint8_t>((v >> 16) & 0xFFu);
    p[3] = static_cast<std::uint8_t>((v >> 24) & 0xFFu);
}

void write_u32_be(std::uint8_t* p, std::uint32_t v) {
    p[0] = static_cast<std::uint8_t>((v >> 24) & 0xFFu);
    p[1] = static_cast<std::uint8_t>((v >> 16) & 0xFFu);
    p[2] = static_cast<std::uint8_t>((v >> 8) & 0xFFu);
    p[3] = static_cast<std::uint8_t>(v & 0xFFu);
}

bool is_little_endian() {
    // 取一个已知值的变量的首字节。小端时最低有效字节在最前面。
    const std::uint16_t probe = 1u;
    const std::uint8_t* first = reinterpret_cast<const std::uint8_t*>(&probe);
    return first[0] == 1u;
}

const std::uint8_t* find_byte(const std::uint8_t* first,
                              const std::uint8_t* last,
                              std::uint8_t value) {
    for (const std::uint8_t* p = first; p != last; ++p) {
        if (*p == value) {
            return p;
        }
    }
    return nullptr;
}

void reg_set_bits(volatile std::uint32_t* reg, std::uint32_t mask) {
    *reg |= mask;
}

void reg_clear_bits(volatile std::uint32_t* reg, std::uint32_t mask) {
    *reg &= ~mask;
}

void reg_toggle_bits(volatile std::uint32_t* reg, std::uint32_t mask) {
    *reg ^= mask;
}

std::size_t pack_frame(const SensorFrame& f, std::uint8_t* out, std::size_t cap) {
    // 先检查容量再写任何东西 —— 半途失败会留下半个包，比直接不写更糟。
    if (out == nullptr || cap < kFrameWireSize) {
        return 0;
    }

    put_u16_le(out + 0, f.id);
    put_u16_le(out + 2, static_cast<std::uint16_t>(f.temperature_q8));
    write_u32_le(out + 4, f.timestamp_ms);
    out[8] = f.crc;

    return kFrameWireSize;
}

bool unpack_frame(const std::uint8_t* in, std::size_t len, SensorFrame* out) {
    if (in == nullptr || out == nullptr || len < kFrameWireSize) {
        return false;
    }

    out->id             = get_u16_le(in + 0);
    out->temperature_q8 = static_cast<std::int16_t>(get_u16_le(in + 2));
    out->timestamp_ms   = read_u32_le(in + 4);
    out->crc            = in[8];

    return true;
}

}  // namespace lab02
