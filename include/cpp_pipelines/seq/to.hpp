#pragma once

#include <cpp_pipelines/pipeline.hpp>

namespace cpp_pipelines::seq
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

template <template <class> class Container>
static constexpr inline auto to = make_pipeline(to_fn<Container>{});

}  // namespace cpp_pipelines::seq