#pragma once

#include <cpp_pipelines/pipeline.hpp>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace cpp_pipelines
{
namespace detail
{
struct format_error : std::runtime_error
{
    format_error(std::string message) : std::runtime_error{ std::move(message) }
    {
    }
};
struct format_fn
{
    struct impl
    {
        using argument_extractor = std::function<void(std::ostream&, int, std::string_view)>;

        std::string_view fmt;

        template <class... Args>
        std::string operator()(const Args&... args) const
        {
            std::stringstream ss;

            const argument_extractor arg_extractor
                = [&](std::ostream& os, int index, std::string_view f) { write_args(os, index, f, args...); };

            do_format(ss, fmt, 0, arg_extractor);
            return ss.str();
        }

        template <class T>
        void write_arg(std::ostream& os, std::string_view fmt, const T& item) const
        {
            apply_format_spec(os, fmt);
            os << item;
        }

        void write_args(std::ostream& os, int index, std::string_view fmt) const
        {
            throw format_error{ "format: invalid argument index" };
        }

        template <class T, class... Args>
        void write_args(std::ostream& os, int index, std::string_view fmt, const T& arg, const Args&... args) const
        {
            if (index == 0)
                write_arg(os, fmt, arg);
            else
                write_args(os, index - 1, fmt, args...);
        }

        void format_text(std::ostream& os, std::string_view txt) const
        {
            os << txt;
        }

        static std::string_view make_string_view(std::string_view::iterator b, std::string_view::iterator e)
        {
            if (b < e)
                return { std::addressof(*b), std::string_view::size_type(e - b) };
            else
                return {};
        }

        static int parse_int(std::string_view txt)
        {
            int result = 0;
            for (char c : txt)
            {
                result = result * 10 + (c - '0');
            }
            return result;
        }

        void format_arg(std::ostream& os, std::string_view fmt, int arg_index, const argument_extractor& arg_extractor) const
        {
            const auto colon = std::find(std::begin(fmt), std::end(fmt), ':');
            const auto index_part = make_string_view(std::begin(fmt), colon);
            const auto fmt_part = make_string_view(colon != std::end(fmt) ? colon + 1 : colon, std::end(fmt));

            const auto actual_index = !index_part.empty() ? parse_int(index_part) : arg_index;

            arg_extractor(os, actual_index, fmt_part);
        }

        void do_format(std::ostream& os, std::string_view fmt, int arg_index, const argument_extractor& arg_extractor) const
        {
            const auto bracket = std::find_if(std::begin(fmt), std::end(fmt), [](char c) { return c == '{' || c == '}'; });
            if (bracket == std::end(fmt))
            {
                return format_text(os, fmt);
            }
            else if (bracket + 1 != std::end(fmt) && bracket[0] == bracket[1])
            {
                format_text(os, make_string_view(std::begin(fmt), bracket + 1));
                return do_format(os, make_string_view(bracket + 2, std::end(fmt)), arg_index, arg_extractor);
            }
            else if (bracket[0] == '{')
            {
                const auto closing_bracket = std::find(bracket + 1, std::end(fmt), '}');
                if (closing_bracket == std::end(fmt))
                {
                    throw format_error{ "format: unclosed bracket" };
                }
                format_text(os, make_string_view(std::begin(fmt), bracket));
                format_arg(os, make_string_view(bracket + 1, closing_bracket), arg_index, arg_extractor);
                return do_format(os, make_string_view(closing_bracket + 1, std::end(fmt)), arg_index + 1, arg_extractor);
            }
            throw format_error{ "format: unexpected opening bracket" };
        }

        void apply_format_spec(std::ostream& os, std::string_view fmt) const
        {
        }
    };

    auto operator()(std::string_view fmt) const
    {
        return fn(impl{ fmt });
    }
};

}  // namespace detail

using detail::format_error;
static constexpr inline auto format = detail::format_fn{};
}  // namespace cpp_pipelines