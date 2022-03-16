#pragma once

#include <cpp_pipelines/subrange.hpp>
#include <iterator>

namespace cpp_pipelines::seq
{
namespace detail
{
template <class T>
struct istream_fn
{
    constexpr auto operator()(std::istream& is) const
    {
        return subrange{
            std::istream_iterator<T>{ is },
            std::istream_iterator<T>{}
        };
    }
};
}  // namespace detail

template <class T>
static constexpr inline auto istream = detail::istream_fn<T>{};
}  // namespace cpp_pipelines::seq