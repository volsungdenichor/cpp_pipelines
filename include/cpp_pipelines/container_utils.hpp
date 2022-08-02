#pragma once

#include <cpp_pipelines/algorithm.hpp>

namespace cpp_pipelines
{
namespace detail
{
template <class T, class = std::void_t<>>
struct is_associative_container : std::false_type
{
};

template <class T>
struct is_associative_container<T, std::void_t<typename T::mapped_type>> : std::true_type
{
};

struct erase_if_fn
{
    template <class Container, class Pred>
    void operator()(Container& container, Pred pred) const
    {
        if constexpr (is_associative_container<Container>::value)
        {
            auto b = std::begin(container);
            auto e = std::end(container);
            while (b != e)
            {
                if (invoke(pred, *b))
                {
                    container.erase(b++);
                }
                else
                {
                    ++b;
                }
            }
        }
        else
        {
            const auto e = std::end(container);
            container.erase(algorithm::remove_if(container, pred), e);
        }
    }
};
}  // namespace detail
static constexpr inline auto erase_if = detail::erase_if_fn{};
}  // namespace cpp_pipelines