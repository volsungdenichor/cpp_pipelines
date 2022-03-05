#pragma once

#include <cpp_pipelines/view_interface.hpp>

namespace cpp_pipelines
{
struct owning_fn
{
    template <class Range>
    struct view
    {
        Range range;

        using iterator = iterator_t<Range>;

        constexpr iterator begin() const
        {
            return std::begin(range);
        }

        constexpr iterator end() const
        {
            return std::end(range);
        }
    };

    template <class Range>
    constexpr auto operator()(Range range) const
    {
        return view_interface{ view<Range>{ std::move(range) } };
    }
};

struct ref_fn
{
    template <class Range>
    struct view
    {
        Range* range;

        using iterator = iterator_t<Range>;

        constexpr iterator begin() const
        {
            return std::begin(*range);
        }

        constexpr iterator end() const
        {
            return std::end(*range);
        }
    };

    template <class Range>
    constexpr auto operator()(Range& range) const
    {
        return view_interface{ view<Range>{ std::addressof(range) } };
    }
};

static constexpr inline auto owning = owning_fn{};
static constexpr inline auto ref = ref_fn{};

struct all_fn
{
    template <class Range>
    constexpr auto operator()(Range&& range) const
    {
        if constexpr (is_view_interface<std::decay_t<Range>>::value)
        {
            return std::forward<Range>(range);
        }
        else if constexpr (std::is_lvalue_reference_v<Range>)
        {
            return ref(std::forward<Range>(range));
        }
        else
        {
            return owning(std::forward<Range>(range));
        }
    }
};

static constexpr inline auto all = all_fn{};

}  // namespace cpp_pipelines
