#pragma once

#include <cpp_pipelines/pipeline.hpp>
#include <memory>
#include <optional>

namespace cpp_pipelines
{
template <class T>
using optional_ref = std::optional<std::reference_wrapper<T>>;

namespace detail
{
struct to_optional_ref_fn
{
    template <class T>
    constexpr auto operator()(optional_ref<T> ref) const -> optional_ref<T>
    {
        return ref;
    }

    template <class T>
    constexpr auto operator()(T* ptr) const -> optional_ref<T>
    {
        return ptr
                   ? std::optional{ std::ref(*ptr) }
                   : std::nullopt;
    }

    template <class T>
    constexpr auto operator()(const std::unique_ptr<T>& ptr) const -> optional_ref<T>
    {
        return (*this)(ptr.get());
    }

    template <class T>
    constexpr auto operator()(const std::shared_ptr<T>& ptr) const -> optional_ref<T>
    {
        return (*this)(ptr.get());
    }
};

}  // namespace detail

static constexpr inline auto to_optional_ref = fn(detail::to_optional_ref_fn{});

}  // namespace cpp_pipelines