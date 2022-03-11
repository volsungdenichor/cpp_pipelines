#pragma once

#include <cpp_pipelines/pipeline.hpp>
#include <variant>

namespace cpp_pipelines::var
{
template <class... Ts>
struct overloaded : Ts...
{
    using Ts::operator()...;
};

template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

struct match_fn
{
    template <class Func>
    struct impl
    {
        Func func;

        template <class T>
        constexpr decltype(auto) operator()(T&& item) const
        {
            return std::visit(func, std::forward<T>(item));
        }
    };

    template <class... Funcs>
    constexpr auto operator()(Funcs... funcs) const
    {
        auto f = overloaded{ std::move(funcs)... };
        return make_pipeline(impl<decltype(f)>{ std::move(f) });
    }
};

static constexpr inline auto match = match_fn{};

}  // namespace cpp_pipelines::var