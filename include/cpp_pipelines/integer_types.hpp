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
#define CPP_PIPELINES_DEFINE_INTEGER(t)                  \
    constexpr t operator""_##t(unsigned long long int v) \
    {                                                    \
        return static_cast<t>(v);                        \
    }

CPP_PIPELINES_DEFINE_INTEGER(i8)
CPP_PIPELINES_DEFINE_INTEGER(i16)
CPP_PIPELINES_DEFINE_INTEGER(i32)
CPP_PIPELINES_DEFINE_INTEGER(i64)

CPP_PIPELINES_DEFINE_INTEGER(u8)
CPP_PIPELINES_DEFINE_INTEGER(u16)
CPP_PIPELINES_DEFINE_INTEGER(u32)
CPP_PIPELINES_DEFINE_INTEGER(u64)

#undef CPP_PIPELINES_DEFINE_INTEGER

}  // namespace literals

}  // namespace cpp_pipelines