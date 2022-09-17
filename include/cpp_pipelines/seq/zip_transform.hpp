#pragma once

#include <cpp_pipelines/pipeline.hpp>
#include <cpp_pipelines/seq/views.hpp>

namespace cpp_pipelines::seq
{
namespace detail
{
struct zip_transform_fn
{
    template <class Func, class... Ranges>
    struct view
    {
        Func func;
        std::tuple<Ranges...> ranges;
        using index_seq = std::index_sequence_for<Ranges...>;

        constexpr view(Func func, std::tuple<Ranges...> ranges)
            : func{ std::move(func) }
            , ranges{ std::move(ranges) }
        {
        }

        struct iter
        {
            const view* parent;
            std::tuple<iterator_t<Ranges>...> its;

            constexpr iter() = default;

            constexpr iter(const view* parent, std::tuple<iterator_t<Ranges>...> its)
                : parent{ parent }
                , its{ its }
            {
            }

            constexpr decltype(auto) deref() const
            {
                return to_return_type(call(index_seq{}));
            }

            constexpr void inc()
            {
                inc(index_seq{});
            }

            constexpr bool is_equal(const iter& other) const
            {
                return is_equal(other, index_seq{});
            }

        private:
            template <std::size_t... I>
            constexpr decltype(auto) call(std::index_sequence<I...>) const
            {
                return invoke(parent->func, *std::get<I>(its)...);
            }

            template <std::size_t... I>
            constexpr void inc(std::index_sequence<I...>)
            {
                (..., ++std::get<I>(its));
            }

            template <std::size_t... I>
            bool is_equal(const iter& other, std::index_sequence<I...>) const
            {
                return (... || (std::get<I>(its) == std::get<I>(other.its)));
            }
        };

        constexpr auto begin() const
        {
            return begin(std::index_sequence_for<Ranges...>{});
        }

        constexpr auto end() const
        {
            return end(std::index_sequence_for<Ranges...>{});
        }

    private:
        template <std::size_t... I>
        constexpr auto begin(std::index_sequence<I...>) const
        {
            return iterator_interface{ iter{ this, std::tuple{ std::begin(std::get<I>(ranges))... } } };
        }

        template <std::size_t... I>
        constexpr auto end(std::index_sequence<I...>) const
        {
            return iterator_interface{ iter{ this, std::tuple{ std::end(std::get<I>(ranges))... } } };
        }
    };

    template <class Func>
    struct impl
    {
        Func func;

        template <class... Ranges>
        constexpr auto operator()(Ranges&&... ranges) const
        {
            return view_interface{ view{ func, std::tuple{ all(std::forward<Ranges>(ranges))... } } };
        }
    };

    template <class Func>
    constexpr inline auto operator()(Func func) const
    {
        return make_pipeline(impl<Func>{ std::move(func) });
    }
};

}  // namespace detail

static constexpr inline auto zip_transform = detail::zip_transform_fn{};

}  // namespace cpp_pipelines::seq