#pragma once

#include <cpp_pipelines/functions.hpp>
#include <cpp_pipelines/pipeline.hpp>
#include <map>
#include <unordered_map>

namespace cpp_pipelines::seq
{
namespace detail
{
template <template <class, class> class Map>
struct to_map_fn
{
    template <class Range>
    constexpr auto operator()(Range&& range) const
    {
        using key_type = std::decay_t<decltype(invoke(get_key, *std::begin(range)))>;
        using value_type = std::decay_t<decltype(invoke(get_value, *std::begin(range)))>;
        return Map<key_type, value_type>{ std::forward<Range>(range) };
    }
};

}  // namespace detail

template <template <class, class> class Map>
static constexpr inline auto to_map_as = make_pipeline(detail::to_map_fn<Map>{});

static constexpr inline auto to_map = to_map_as<std::map>;
static constexpr inline auto to_multimap = to_map_as<std::multimap>;

static constexpr inline auto to_unordered_map = to_map_as<std::unordered_map>;
static constexpr inline auto to_unordered_multimap = to_map_as<std::unordered_multimap>;

}  // namespace cpp_pipelines::seq
