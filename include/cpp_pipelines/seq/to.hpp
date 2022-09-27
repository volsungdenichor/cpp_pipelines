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
static constexpr inline auto to = make_pipeline(detail::to_fn<Container>{});

template <class Container>
static constexpr inline auto as = make_pipeline(cast<Container>);

static constexpr inline auto to_vector = to<std::vector>;
static constexpr inline auto to_string = as<std::string>;
static constexpr inline auto collect = to_vector;

}  // namespace cpp_pipelines::seq