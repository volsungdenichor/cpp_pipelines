#pragma once

#include <cpp_pipelines/functions.hpp>
#include <cpp_pipelines/seq/access.hpp>
#include <cpp_pipelines/seq/generate.hpp>
#include <cpp_pipelines/seq/transform.hpp>
#include <cpp_pipelines/subrange.hpp>

namespace cpp_pipelines::map
{
namespace detail
{
struct equal_range_fn
{
    template <class K>
    struct impl
    {
        K key;

        template <class Map>
        constexpr auto operator()(Map& map) const
        {
            return subrange{ map.equal_range(key) } >>= seq::transform(get_value);
        }
    };

    template <class K>
    constexpr auto operator()(K key) const
    {
        return make_pipeline(impl<K>{ std::move(key) });
    }
};

struct maybe_at
{
    template <class K>
    constexpr auto operator()(K key) const
    {
        return equal_range_fn{}(std::move(key)) >>= seq::maybe_front;
    }
};

struct keys_fn
{
    template <class Map>
    struct generator
    {
        Map* map;
        mutable typename Map::const_iterator it;

        constexpr generator(Map& map)
            : map{ std::addressof(map) }
            , it{ this->map->begin() }
        {
        }

        constexpr auto operator()() const -> const typename Map::key_type*
        {
            if (it != map->end())
            {
                auto& res = it->first;
                it = map->upper_bound(it->first);
                return std::addressof(res);
            }
            return nullptr;
        }
    };

    template <class Map>
    constexpr auto operator()(Map& map) const
    {
        return seq::generate(generator<Map>{ map });
    }
};

struct items_fn
{
    template <class Map>
    constexpr auto operator()(Map& map) const
    {
        return keys_fn{}(map) >>= seq::transform([&](const auto& key) { return std::pair{ key, map >>= equal_range_fn{}(key) }; });
    }
};

}  // namespace detail
constexpr inline auto equal_range = detail::equal_range_fn{};
constexpr inline auto maybe_at = detail::maybe_at{};
constexpr inline auto keys = make_pipeline(detail::keys_fn{});
constexpr inline auto items = make_pipeline(detail::items_fn{});
}  // namespace cpp_pipelines::map