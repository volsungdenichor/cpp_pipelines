#pragma once

#include <cpp_pipelines/pipeline.hpp>
#include <cpp_pipelines/seq/views.hpp>

namespace cpp_pipelines::seq
{
struct transform_zip_fn
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

        using iterator = iterator_interface<iter>;

        constexpr iterator begin() const
        {
            return begin(std::index_sequence_for<Ranges...>{});
        }

        constexpr iterator end() const
        {
            return end(std::index_sequence_for<Ranges...>{});
        }

        template <std::size_t... I>
        constexpr iterator begin(std::index_sequence<I...>) const
        {
            return { iter{ this, std::tuple{ std::begin(std::get<I>(ranges))... } } };
        }

        template <std::size_t... I>
        constexpr iterator end(std::index_sequence<I...>) const
        {
            return { iter{ this, std::tuple{ std::end(std::get<I>(ranges))... } } };
        }
    };

    template <class Func, class... Ranges>
    constexpr inline auto operator()(Func func, Ranges&&... ranges) const
    {
        return view_interface{ view{ std::move(func), std::tuple{ all(std::forward<Ranges>(ranges))... } } };
    }
};

static constexpr inline auto transform_zip = transform_zip_fn{};

}  // namespace cpp_pipelines::seq