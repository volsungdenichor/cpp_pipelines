#pragma once

#include <utility>

namespace cpp_pipelines
{
template <class T>
class out
{
public:
    constexpr explicit out(T& item)
        : _item{ std::addressof(item) }
    {
    }

    constexpr out(const out&) = default;

    template <class U>
    constexpr out& operator=(U&& value)
    {
        get() = std::forward<U>(value);
        return *this;
    }

    constexpr T& get()
    {
        return *_item;
    }

private:
    T* _item;
};
}  // namespace cpp_pipelines