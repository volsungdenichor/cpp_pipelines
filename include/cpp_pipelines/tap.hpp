#pragma once

#include <cpp_pipelines/pipeline.hpp>

namespace cpp_pipelines
{
namespace detail
{
struct inspect_fn
{
    template <class Func>
    struct impl
    {
        Func func;

        template <class T>
        constexpr decltype(auto) operator()(T&& item) const
        {
            invoke(func, item);
            return to_return_type(std::forward<T>(item));
        }
    };

    template <class Func>
    constexpr auto operator()(Func func) const
    {
        return make_pipeline(impl<Func>{ std::move(func) });
    }
};
}  // namespace detail

static constexpr inline auto inspect = detail::inspect_fn{};
static constexpr inline auto tap = inspect;

}  // namespace cpp_pipelines