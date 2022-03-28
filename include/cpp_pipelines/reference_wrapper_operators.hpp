#pragma once

#include <functional>

#define CPP_PIPELINES_DEFINE_OP(op)                                                \
    template <class L, class R>                                                    \
    constexpr bool operator op(reference_wrapper<L> lhs, reference_wrapper<R> rhs) \
    {                                                                              \
        return lhs.get() op rhs.get();                                             \
    }                                                                              \
                                                                                   \
    template <class L, class R>                                                    \
    constexpr bool operator op(reference_wrapper<L> lhs, const R& rhs)             \
    {                                                                              \
        return lhs.get() op rhs;                                                   \
    }                                                                              \
                                                                                   \
    template <class L, class R>                                                    \
    constexpr bool operator op(const L& lhs, reference_wrapper<R> rhs)             \
    {                                                                              \
        return lhs op rhs.get();                                                   \
    }

namespace std
{
CPP_PIPELINES_DEFINE_OP(==)
CPP_PIPELINES_DEFINE_OP(!=)
CPP_PIPELINES_DEFINE_OP(<)
CPP_PIPELINES_DEFINE_OP(<=)
CPP_PIPELINES_DEFINE_OP(>)
CPP_PIPELINES_DEFINE_OP(>=)
}  // namespace std

#undef CPP_PIPELINES_DEFINE_OP
