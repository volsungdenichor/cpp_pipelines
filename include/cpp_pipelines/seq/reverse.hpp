#pragma once

#include <cpp_pipelines/pipeline.hpp>
#include <cpp_pipelines/seq/views.hpp>
#include <cpp_pipelines/subrange.hpp>

namespace cpp_pipelines::seq
{
struct reverse_fn
{
    template <class Iter>
    constexpr auto make_reverse(Iter iter) const
    {
        return std::make_reverse_iterator(iter);
    }

    template <class Iter>
    constexpr auto make_reverse(std::reverse_iterator<Iter> iter) const
    {
        return iter.base();
    }

    struct impl
    {
        template <class Range>
        constexpr auto operator()(Range&& range) const
        {
            return subrange{ make_reverse_iterator(std::end(range)), make_reverse_iterator(std::begin(range)) };
        }
    };

    constexpr auto operator()() const
    {
        return make_pipeline(impl{});
    }
};

static constexpr inline auto reverse = reverse_fn{};

}  // namespace cpp_pipelines::seq