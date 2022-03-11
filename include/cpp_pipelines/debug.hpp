#pragma once
#include <iostream>
#include <optional>
#include <string>
#include <tuple>

#ifdef __GNUG__
#include <cxxabi.h>

#include <cstdlib>
#include <memory>

inline std::string demangle(const char* name)
{
    int status = -4;
    std::unique_ptr<char, void (*)(void*)> res{ abi::__cxa_demangle(name, NULL, NULL, &status), std::free };
    return (status == 0) ? res.get() : name;
}

#else

// does nothing if not g++
inline std::string demangle(const char* name)
{
    return name;
}

#endif

namespace std
{
template <class T>
ostream& operator<<(ostream& os, const optional<T>& item)
{
    if (item)
        os << "some{" << *item << "}";
    else
        os << "none";
    return os;
}

template <class T>
ostream& operator<<(ostream& os, const reference_wrapper<T>& item)
{
    return os << item.get();
}

template <class... Args>
ostream& operator<<(ostream& os, const tuple<Args...>& item)
{
    os << "(";
    std::apply([&os](const auto&... args) { auto n = 0u; ((os << args << (++n != sizeof...(args) ? ", " : "")), ...); }, item);
    os << ")";
    return os;
}
}  // namespace std