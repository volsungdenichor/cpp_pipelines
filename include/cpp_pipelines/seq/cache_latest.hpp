#pragma once

#include <cpp_pipelines/seq/views.hpp>

namespace cpp_pipelines::seq
{
struct cache_latest_fn
{
    template <class Range>
    struct view
    {
        Range range;

        constexpr view(Range range)
            : range{ std::move(range) }
        {
        }

        struct iter
        {
            using inner_iterator = iterator_t<Range>;
            using reference = iter_reference_t<inner_iterator>;
            using cache_type = std::conditional_t<
                std::is_lvalue_reference_v<reference>,
                std::add_pointer_t<reference>,
                std::optional<reference>>;

            inner_iterator it;
            mutable cache_type cache;

            constexpr reference deref() const
            {
                if (!cache)
                {
                    if constexpr (std::is_lvalue_reference_v<reference>)
                    {
                        cache = std::addressof(*it);
                    }
                    else
                    {
                        cache = *it;
                    }
                }
                return *cache;
            }

            void inc()
            {
                ++it;
                cache = cache_type{};
            }

            constexpr bool is_equal(const iter& other) const
            {
                return it == other.it;
            }
        };

        using iterator = iterator_interface<iter>;

        constexpr iterator begin() const
        {
            return { iter{ std::begin(range) } };
        }

        constexpr iterator end() const
        {
            return { iter{ std::end(range) } };
        }
    };

    struct impl
    {
        template <class Range>
        constexpr auto operator()(Range&& range) const
        {
            return view_interface{ view{ all(std::forward<Range>(range)) } };
        }
    };

    constexpr auto
    operator()() const
    {
        return make_pipeline(impl{});
    }
};

static constexpr inline auto cache_latest = cache_latest_fn{};
}  // namespace cpp_pipelines::seq