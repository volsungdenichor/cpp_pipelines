#pragma once

#include <cpp_pipelines/invoke.hpp>
#include <optional>

namespace cpp_pipelines
{
template <class Func>
class semiregular
{
private:
    static constexpr bool is_default_constructible = std::is_default_constructible_v<Func>;

    using impl_type = std::conditional_t<is_default_constructible, Func, std::optional<Func>>;

public:
    constexpr semiregular() = default;

    constexpr semiregular(Func func)
        : _func{ std::move(func) }
    {
    }

    constexpr semiregular(const semiregular&) = default;

    constexpr semiregular(semiregular&&) = default;

    constexpr semiregular& operator=(semiregular other)
    {
        if constexpr (std::is_assignable_v<impl_type, impl_type>)
        {
            _func = std::move(other._func);
        }
        else
        {
            _func.~impl_type();
            new (&_func) impl_type{ std::move(other._func) };
        }
        return *this;
    }

    template <class... Args>
    constexpr decltype(auto) operator()(Args&&... args) const
    {
        return invoke(get(), std::forward<Args>(args)...);
    }

    template <class... Args>
    constexpr decltype(auto) operator()(Args&&... args)
    {
        return invoke(get(), std::forward<Args>(args)...);
    }

private:
    constexpr decltype(auto) get() const
    {
        if constexpr (is_default_constructible)
            return _func;
        else
            return *_func;
    }

    constexpr decltype(auto) get()
    {
        if constexpr (is_default_constructible)
            return _func;
        else
            return *_func;
    }

    mutable impl_type _func;
};
}  // namespace cpp_pipelines