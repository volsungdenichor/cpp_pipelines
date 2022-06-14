#pragma once

#include <cstdint>

namespace cpp_pipelines
{
using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

namespace literals
{
constexpr i8 operator""_i8(unsigned long long int v)
{
    return static_cast<i8>(v);
}

constexpr i16 operator""_i16(unsigned long long int v)
{
    return static_cast<i16>(v);
}

constexpr i32 operator""_i32(unsigned long long int v)
{
    return static_cast<i32>(v);
}

constexpr i64 operator""_i64(unsigned long long int v)
{
    return static_cast<i64>(v);
}

constexpr u8 operator""_u8(unsigned long long int v)
{
    return static_cast<u8>(v);
}

constexpr u16 operator""_u16(unsigned long long int v)
{
    return static_cast<u16>(v);
}

constexpr u32 operator""_u32(unsigned long long int v)
{
    return static_cast<u32>(v);
}

constexpr u64 operator""_u64(unsigned long long int v)
{
    return static_cast<u64>(v);
}
}  // namespace literals

}  // namespace cpp_pipelines