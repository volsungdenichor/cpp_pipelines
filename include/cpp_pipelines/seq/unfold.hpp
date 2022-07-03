#pragma once

#include <cpp_pipelines/seq/generate.hpp>

namespace cpp_pipelines::seq
{
namespace detail
{
struct unfold_fn
{
    template <class T, class Func>
    constexpr auto operator()(T init, Func func) const
    {
        using result_type = std::decay_t<decltype(std::get<0>(*func(init)))>;

        return generate([=, state = std::move(init)]() mutable -> std::optional<result_type> {
            if (auto func_result = invoke(func, state))
            {
                auto [result, new_state] = *std::move(func_result);
                state = std::move(new_state);
                return std::optional{ std::move(result) };
            }
            else
            {
                return std::nullopt;
            }
        });
    }
};

struct unfold_infinite_fn
{
    template <class T, class Func>
    constexpr auto operator()(T init, Func func) const
    {
        return generate([=, state = std::move(init)]() mutable {
            auto func_result = invoke(func, state);
            auto [result, new_state] = std::move(func_result);
            state = std::move(new_state);
            return std::optional{ std::move(result) };
        });
    }
};

}  // namespace detail

static constexpr inline auto unfold = detail::unfold_fn{};
static constexpr inline auto unfold_infinite = detail::unfold_infinite_fn{};
}  // namespace cpp_pipelines::seq