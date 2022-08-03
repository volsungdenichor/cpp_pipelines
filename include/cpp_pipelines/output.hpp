#pragma once

#include <cpp_pipelines/type_traits.hpp>
#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string_view>

namespace cpp_pipelines
{
struct ostream_manipulator : public std::function<void(std::ostream&)>
{
    using base_type = std::function<void(std::ostream&)>;
    using base_type::base_type;

    friend std::ostream& operator<<(std::ostream& os, const ostream_manipulator& item)
    {
        item(os);
        return os;
    }
};

struct ostream_iterator : std::iterator<std::output_iterator_tag, void, void, void, void>
{
    std::ostream* os;
    std::string_view separator;

    ostream_iterator(std::ostream& os, std::string_view separator = {})
        : os{ &os }
        , separator{ separator }
    {
    }

    ostream_iterator& operator*()
    {
        return *this;
    }

    ostream_iterator& operator++()
    {
        return *this;
    }

    ostream_iterator& operator++(int)
    {
        return *this;
    }

    template <class T>
    ostream_iterator& operator=(const T& item)
    {
        *os << item << separator;
        return *this;
    }
};

namespace detail
{
struct delimit_fn
{
    template <class Range>
    auto operator()(Range&& range, std::string_view separator = {}) const -> ostream_manipulator
    {
        return impl(std::begin(range), std::end(range), separator);
    }

private:
    template <class Iter>
    auto impl(Iter begin, Iter end, std::string_view separator = {}) const -> ostream_manipulator
    {
        return { [b = begin, e = end, s = separator, first = true](std::ostream& os) mutable {
            for (auto it = b; it != e; ++it)
            {
                if (!first)
                    os << s;
                os << *it;
                first = false;
            }
        } };
    }
};

struct write_fn
{
    template <class... Args>
    std::ostream& operator()(std::ostream& os, const Args&... args) const
    {
        (os << ... << args);
        return os;
    }
};

struct str_fn
{
    template <class... Args>
    std::string operator()(const Args&... args) const
    {
        std::stringstream ss;
        write_fn{}(ss, args...);
        return std::move(ss).str();
    }
};

struct safe_print_fn
{
    template <class T>
    constexpr auto operator()(const T& item) const -> ostream_manipulator
    {
        if constexpr (has_ostream_operator<T>::value)
        {
            return { [&](std::ostream& os) { os << item; } };
        }
        else if constexpr (is_range<T>::value)
        {
            return {
                [&](std::ostream& os) {
                    os << "[";
                    const auto b = std::begin(item);
                    const auto e = std::end(item);
                    bool first = true;
                    for (auto it = b; it != e; ++it)
                    {
                        if (!first)
                            os << ", ";
                        os << (*this)(*it);
                        first = false;
                    }
                    os << "]";
                }
            };
        }

        return { [](std::ostream& os) { os << "???"; } };
    }
};

struct ostream_arg_fn
{
    template <class... Args>
    auto operator()(const Args&... args) const -> ostream_manipulator
    {
        return [&](std::ostream& os) { (os << ... << args); };
    }
};

struct ostream_fill_fn
{
    auto operator()(char ch) const -> ostream_manipulator
    {
        return [=](std::ostream& os) { os << std::setfill(ch); };
    }

    auto operator()(char ch, std::ptrdiff_t n) const -> ostream_manipulator
    {
        return [=](std::ostream& os) { os << std::setfill(ch) << std::setw(n); };
    }
};

struct ostream_zero_fill_fn
{
    auto operator()(std::ptrdiff_t n) const -> ostream_manipulator
    {
        return [=](std::ostream& os) { os << std::setfill('0') << std::setw(n); };
    }
};

}  // namespace detail

static constexpr inline auto delimit = detail::delimit_fn{};
static constexpr inline auto write = detail::write_fn{};
static constexpr inline auto str = detail::str_fn{};
static constexpr inline auto safe_print = detail::safe_print_fn{};
static constexpr inline auto ostream_arg = detail::ostream_arg_fn{};
static constexpr inline auto ostream_fill = detail::ostream_fill_fn{};
static constexpr inline auto ostream_zero_fill = detail::ostream_zero_fill_fn{};

struct cout
{
    std::string_view prefix = "";
    std::string_view suffix = "";

    template <class T>
    const T& operator()(const T& item) const
    {
        std::cout << prefix << item << suffix << std::endl;
        return item;
    }
};

}  // namespace cpp_pipelines
