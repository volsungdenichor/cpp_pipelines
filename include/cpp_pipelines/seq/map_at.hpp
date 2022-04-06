#pragma once

#include <cpp_pipelines/functions.hpp>
#include <cpp_pipelines/seq/access.hpp>
#include <cpp_pipelines/seq/generate.hpp>
#include <cpp_pipelines/seq/transform.hpp>
#include <cpp_pipelines/subrange.hpp>

namespace cpp_pipelines::seq
{
namespace detail
{
struct map_at_fn
{
    template <class K>
    struct impl
    {
        K key;

        template <class Map>
        constexpr auto operator()(Map& map) const
        {
            return subrange{ map.equal_range(key) } >>= transform(get_value);
        }
    };

    template <class K>
    constexpr auto operator()(K key) const
    {
        return make_pipeline(impl<K>{ std::move(key) });
    }
};

struct map_maybe_at
{
    template <class K>
    constexpr auto operator()(K key) const
    {
        return map_at_fn{}(std::move(key)) >>= maybe_front;
    }
};

struct map_keys_fn
{
    template <class Map>
    constexpr auto operator()(Map& map) const
    {
        return generate([it = map.begin(), &map]() mutable -> const typename Map::key_type*
        {
            if (it != map.end())
            {
                auto& res = it->first;
                it = map.upper_bound(it->first);
                return std::addressof(res);
            }
            return nullptr;
        });
    }
};

struct map_items_fn
{
    template <class Map>
    constexpr auto operator()(Map& map) const
    {
        return map_keys_fn{}(map) >>= seq::transform([&](const auto& key) { return std::pair{key, map >>= map_at_fn{}(key)}; });
    }
};

}  // namespace detail
constexpr inline auto map_at = detail::map_at_fn{};
constexpr inline auto map_maybe_at = detail::map_maybe_at{};
constexpr inline auto map_keys = make_pipeline(detail::map_keys_fn{});
constexpr inline auto map_items = make_pipeline(detail::map_items_fn{});
}  // namespace cpp_pipelines::seq