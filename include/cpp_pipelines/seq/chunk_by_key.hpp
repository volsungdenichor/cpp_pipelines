#pragma once

#include <cpp_pipelines/iter_utils.hpp>
#include <cpp_pipelines/pipeline.hpp>
#include <cpp_pipelines/seq/chunk_base.hpp>

namespace cpp_pipelines::seq
{
namespace detail
{
struct chunk_by_key_fn
{
    template <class Func>
    struct policy
    {
        semiregular<Func> func;

        template <class Iter>
        constexpr subrange<Iter> operator()(subrange<Iter> sub) const
        {
            const auto& key = invoke(func, *std::begin(sub));
            const auto b = advance_while(std::begin(sub), [&](const auto& x) { return invoke(func, x) == key; }, std::end(sub));
            return subrange{b, b};
        }
    };

    template <class Func, class Range>
    struct view : public chunk_view_base<policy<Func>, Range>
    {
        using base_type = chunk_view_base<policy<Func>, Range>;

        constexpr view(Func func, Range range)
            : base_type{policy<Func>{std::move(func)}, std::move(range)}
        {
        }
    };

    template <class Func>
    struct impl
    {
        Func func;

        template <class Range>
        constexpr auto operator()(Range&& range) const
        {
            return view_interface{ view{ func, all(std::forward<Range>(range)) } };
        }
    };

    template <class Func>
    constexpr auto operator()(Func func) const
    {
        return make_pipeline(impl<Func>{ std::move(func) });
    }
};

}  // namespace detail

static constexpr inline auto chunk_by_key = detail::chunk_by_key_fn{};
}  // namespace cpp_pipelines::seq