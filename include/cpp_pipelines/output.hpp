#pragma once

#include <functional>
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
    template <class Range, class>
    auto operator()(Range&& range, std::string_view separator = {}) const -> ostream_manipulator
    {
        return impl(std::begin(range), std::end(range), separator);
    }

private:
    template <class Iter>
    auto impl(Iter begin, Iter end, std::string_view separator = {}) const -> ostream_manipulator
    {
        return { [=](std::ostream& os) {
            for (auto it = begin; it != end; ++it)
            {
                if (it != begin)
                    os << separator;
                os << *it;
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

}  // namespace detail

static constexpr inline auto delimit = detail::delimit_fn{};
static constexpr inline auto write = detail::write_fn{};
static constexpr inline auto str = detail::str_fn{};

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
