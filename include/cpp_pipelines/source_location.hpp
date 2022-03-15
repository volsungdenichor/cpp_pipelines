#pragma once

#include <string_view>

namespace cpp_pipelines
{

class source_location
{
public:
    source_location() = default;

    source_location(std::string_view file_name, std::uint32_t line, std::string_view function_name)
        : _file_name{file_name}
        , _line{line}
        , _function_name{function_name}
    {
    }

    source_location(const source_location&) = default;
    source_location(source_location&&) = default;

    std::string_view file_name() const noexcept
    {
        return _file_name;
    }

    std::string_view function_name() const noexcept
    {
        return _function_name;
    }

    std::uint32_t line() const noexcept
    {
        return _line;
    }

    friend std::ostream& operator<<(std::ostream& os, const source_location& item)
    {
        return os << item.file_name() << "(" << item.line() << "): " << item.function_name();
    }

private:
    std::string_view _file_name;
    std::uint32_t _line;
    std::string_view _function_name;
};

} // namespace cpp_pipelines

#define SOURCE_LOCATION ::cpp_pipelines::source_location{__FILE__, __LINE__, __PRETTY_FUNCTION__}