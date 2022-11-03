#pragma once

#include <cpp_pipelines/functions.hpp>
#include <cpp_pipelines/seq/access.hpp>
#include <cpp_pipelines/seq/generate.hpp>
#include <cpp_pipelines/seq/to_map.hpp>
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
            return subrange{ map.equal_range(key) };
        }
    };

    template <class K>
    constexpr auto operator()(K key) const
    {
        return make_pipeline(impl<K>{ std::move(key) });
    }
};

struct values_at_fn
{
    static constexpr inline auto equal_range = equal_range_fn{};

    template <class K>
    constexpr auto operator()(K key) const
    {
        return equal_range(std::move(key)) |= seq::transform(get_value);
    }
};

struct maybe_at_fn
{
    static constexpr inline auto values_at = values_at_fn{};

    template <class K>
    constexpr auto operator()(K key) const
    {
        return values_at(std::move(key)) |= seq::maybe_front;
    }
};

struct at_fn
{
    static constexpr inline auto values_at = values_at_fn{};

    template <class K>
    constexpr auto operator()(K key) const
    {
        return values_at(std::move(key)) |= seq::front;
    }
};

struct keys_fn
{
    template <class T>
    using has_upper_bound = decltype(std::declval<T&>().upper_bound(std::declval<typename T::key_type&>()));

    template <class Map>
    struct generator
    {
        const Map* map;
        mutable typename Map::const_iterator it;

        constexpr generator(const Map& map)
            : map{ std::addressof(map) }
            , it{ this->map->begin() }
        {
        }

        constexpr auto operator()() const -> const typename Map::key_type*
        {
            if (it != map->end())
            {
                auto& res = it->first;
                if constexpr (is_detected_v<has_upper_bound, Map>)
                {
                    it = map->upper_bound(it->first);
                }
                else
                {
                    while (it != map->end() && it->first == res)
                    {
                        ++it;
                    }
                }

                return std::addressof(res);
            }
            return nullptr;
        }
    };

    template <class Map>
    constexpr auto operator()(Map& map) const
    {
        return seq::generate(generator{ map });
    }
};

struct items_fn
{
    static constexpr inline auto values_at = values_at_fn{};

    template <class Map>
    constexpr auto operator()(Map& map) const
    {
        return keys_fn{}(map) |= seq::transform([&](const auto& key) { return std::pair{ key, map |= values_at(key) }; });
    }
};

template <template <class, class> class Map>
struct group_by_as_fn
{
    template <class Key, class Value>
    struct impl
    {
        Key key;
        Value value;

        template <class Range>
        constexpr auto operator()(Range&& range) const
        {
            return std::forward<Range>(range) |= seq::transform(make_pair(key, value)) |= seq::to_map_as<Map>;
        }
    };

    template <class Key, class Value>
    constexpr auto operator()(Key key, Value value) const
    {
        return make_pipeline(impl<Key, Value>{ std::move(key), std::move(value) });
    }
};

}  // namespace detail
constexpr inline auto equal_range = detail::equal_range_fn{};
constexpr inline auto values_at = detail::values_at_fn{};
constexpr inline auto at = detail::at_fn{};
constexpr inline auto maybe_at = detail::maybe_at_fn{};
constexpr inline auto keys = make_pipeline(detail::keys_fn{});
constexpr inline auto items = make_pipeline(detail::items_fn{});

template <template <class, class> class Map>
constexpr inline auto group_by_as = detail::group_by_as_fn<Map>{};
}  // namespace cpp_pipelines::map