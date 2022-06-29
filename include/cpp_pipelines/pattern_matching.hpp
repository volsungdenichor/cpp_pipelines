#pragma once

#include <cpp_pipelines/pipeline.hpp>
#include <cpp_pipelines/predicates.hpp>
#include <cpp_pipelines/type_traits.hpp>
#include <memory>
#include <variant>

namespace cpp_pipelines::pattern_matching
{
struct no_match : std::runtime_error
{
    no_match()
        : std::runtime_error{ "pattern_matching: no match" }
    {
    }
};

namespace detail
{
using predicates::detail::predicate_interface;

template <class Pred, class Func>
struct matcher_t
{
    Pred pred;
    Func func;
};

struct when_fn
{
    template <class Pred>
    struct impl
    {
        Pred pred;

        template <class Func>
        constexpr auto then(Func func) &&
        {
            return matcher_t<Pred, Func>{ std::move(pred), std::move(func) };
        }

        template <class Func>
        constexpr auto operator|=(Func func) &&
        {
            return (std::move(*this)).then(std::move(func));
        }
    };

    template <class Pred>
    constexpr auto operator()(Pred pred) const
    {
        return (*this)(predicate_interface<Pred>{ std::move(pred) });
    }

    template <class Impl>
    constexpr auto operator()(predicate_interface<Impl> pred) const
    {
        return impl<predicate_interface<Impl>>{ std::move(pred) };
    }
};

template <class Func, class... Args>
constexpr decltype(auto) eval(Func&& func, Args&&... args)
{
    if constexpr (std::is_invocable_v<Func, Args...>)
    {
        return std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);
    }
    else if constexpr (std::is_invocable_v<Func>)
    {
        return std::invoke(std::forward<Func>(func));
    }
    else
    {
        return std::forward<Func>(func);
    }
}

struct match_fn
{
    template <class... Matchers>
    struct impl
    {
        std::tuple<Matchers...> matchers;

        template <class T>
        constexpr auto operator()(const T& item) const
        {
            return call<0>(item);
        }

        template <std::size_t I, class T>
        constexpr auto call(const T& item) const
        {
            const auto& matcher = std::get<I>(matchers);
            if constexpr (I + 1 < sizeof...(Matchers))
            {
                return matcher.pred.template is(item)
                           ? eval(matcher.func, matcher.pred.template as(item))
                           : call<I + 1>(item);
            }
            else
            {
                return matcher.pred.template is(item)
                           ? eval(matcher.func, matcher.pred.template as(item))
                           : throw no_match{};
            }
        }
    };

    template <class... Matchers>
    constexpr auto operator()(Matchers... matchers) const
    {
        return make_pipeline(impl<Matchers...>{ std::tuple<Matchers...>{ std::move(matchers)... } });
    }
};

}  // namespace detail

static constexpr inline auto match = detail::match_fn{};
static constexpr inline auto when = detail::when_fn{};

#if 1
static constexpr inline auto switch_ = match;
static constexpr inline auto case_ = when;
#endif


}  // namespace cpp_pipelines::pattern_matching
