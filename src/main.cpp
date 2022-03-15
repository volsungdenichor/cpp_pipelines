#include <cmath>
#include <cpp_pipelines/algorithm.hpp>
#include <cpp_pipelines/debug.hpp>
#include <cpp_pipelines/functions.hpp>
#include <cpp_pipelines/opt.hpp>
#include <cpp_pipelines/output.hpp>
#include <cpp_pipelines/predicates.hpp>
#include <cpp_pipelines/res.hpp>
#include <cpp_pipelines/seq.hpp>
#include <cpp_pipelines/sub.hpp>
#include <cpp_pipelines/tap.hpp>
#include <cpp_pipelines/var.hpp>
#include <forward_list>
#include <iomanip>
#include <iostream>
#include <locale>
#include <map>
#include <optional>
#include <sstream>
#include <variant>

struct parse_error
{
};

inline std::ostream& operator<<(std::ostream& os, const parse_error&)
{
    return os << "parse_error";
}
template <class T>
struct parse_fn
{
    cpp_pipelines::result<T, parse_error> operator()(std::string text) const
    {
        std::stringstream ss{ std::move(text) };
        T result;
        ss >> result;
        if (ss)
            return std::move(result);
        else
            return cpp_pipelines::error(parse_error{});
    }
};

template <class T>
static constexpr inline auto parse = parse_fn<T>{};

std::optional<double> square_root(double x)
{
    return x >= 0.0
               ? std::optional{ std::sqrt(x) }
               : std::nullopt;
}

struct print
{
    std::string prefix = {};

    template <class T>
    void operator()(const T& item) const
    {
        std::cout << prefix << item << " [" << demangle(typeid(item).name()) << "] [" << std::addressof(item) << "]" << std::endl;
    }
};

struct decorate_string
{
    std::string_view prefix = "";
    std::string_view suffix = "";

    template <class T>
    std::string operator()(const T& item) const
    {
        return cpp_pipelines::str(prefix, item, suffix);
    }
};

template <class Func>
struct transform_string_fn
{
    Func func;

    std::string operator()(std::string text) const
    {
        return text >>= cpp_pipelines::seq::transform(std::ref(*this));
    }

    char operator()(char ch) const
    {
        return func(ch);
    }
};

template <class Func>
transform_string_fn(Func) -> transform_string_fn<Func>;

static constexpr inline auto uppercase = transform_string_fn{ [](char ch) { return std::toupper(ch); } };
static constexpr inline auto lowercase = transform_string_fn{ [](char ch) { return std::tolower(ch); } };

struct Person
{
    std::string name;
    int age;

    friend std::ostream& operator<<(std::ostream& os, const Person& item)
    {
        return os << "Person{ name=" << item.name << " age=" << item.age << " }";
    }
};

constexpr auto linspace(float start, float stop, int n)
{
    using namespace cpp_pipelines;
    return seq::iota(n) >>= seq::transform([=](int _) { return start + (stop - start) * _ / (n - 1); });
}

auto zero_padded(std::ptrdiff_t n) -> cpp_pipelines::ostream_manipulator
{
    return [=](std::ostream& os) { os << std::setw(n) << std::setfill('0'); };
}

void print_text(cpp_pipelines::const_view<std::string> txt)
{
    using namespace cpp_pipelines;
    std::cout << "\"";
    txt >>= seq::copy(ostream_iterator{ std::cout, "" });
    std::cout << "\"";
    std::cout << std::endl;
}

void run()
{
    using namespace std::string_literals;
    using namespace cpp_pipelines;
    namespace p = cpp_pipelines::predicates;
    using p::__;

    std::vector<Person> persons{
        Person{ "Adam", 10 },
        Person{ "Bartek", 13 },
        Person{ "-23", 13 },
        Person{ "Celina", 24 },
        Person{ "542", 24 },
        Person{ "Daria", -1 },
        Person{ "Ewa", 64 },
        Person{ "912", 24 },
        Person{ "Helena", 24 },
    };

    std::multimap<int, std::string> map = persons >>= seq::transform([](const Person& p) { return std::pair{ p.age, p.name }; });

    if (auto v = map >>= seq::map_maybe_at(24))
    {
        std::cout << v << " " << demangle(typeid(v).name()) << std::endl;
    }
    else
    {
        std::cout << "-" << std::endl;
    }
}

int main()
{
    try
    {
        run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}