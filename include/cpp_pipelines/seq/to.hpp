#pragma once

#include <cpp_pipelines/functions.hpp>
#include <cpp_pipelines/pipeline.hpp>

namespace cpp_pipelines::seq
{
namespace detail
{
template <template <class> class Container>
struct to_fn
{
    template <class Range>
    constexpr auto operator()(Range&& range) const -> Container<range_value_t<Range>>
    {
        return range;
    }
};

}  // namespace detail

template <template <class> class Container>
static constexpr inline auto to = fn(detail::to_fn<Container>{});

template <class Container>
static constexpr inline auto as = fn(cast<Container>);

static constexpr inline auto to_vector = to<std::vector>;
static constexpr inline auto to_string = as<std::string>;

}  // namespace cpp_pipelines::seq