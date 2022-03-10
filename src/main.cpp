#include <cmath>
#include <cpp_pipelines/algorithm.hpp>
#include <cpp_pipelines/debug.hpp>
#include <cpp_pipelines/functions.hpp>
#include <cpp_pipelines/opt.hpp>
#include <cpp_pipelines/output.hpp>
#include <cpp_pipelines/predicates.hpp>
#include <cpp_pipelines/seq.hpp>
#include <cpp_pipelines/tap.hpp>
#include <forward_list>
#include <iostream>
#include <locale>
#include <optional>
#include <sstream>
#include <variant>

template <class T>
struct parse_fn
{
    std::optional<T> operator()(std::string text) const
    {
        std::stringstream ss{ std::move(text) };
        T result;
        ss >> result;
        return ss
                   ? std::optional{ std::move(result) }
                   : std::nullopt;
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

template <class T>
void print(const T& item)
{
    std::cout << item << " [" << demangle(typeid(item).name()) << "] [" << std::addressof(item) << "]" << std::endl;
}

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

auto linspace(float start, float stop, int n)
{
    using namespace cpp_pipelines;
    return seq::iota(n) >>= seq::transform([=](int _) { return start + (stop - start) * _ / (n - 1); });
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
        Person{ "Celina", 24 },
        Person{ "Daria", -1 },
        Person{ "Ewa", 64 },

    };

    static const auto is_vowel = p::any('a', 'e', 'i', 'o', 'u', 'y');
    const auto pred = fn(&Person::name, seq::front, lowercase, is_vowel);

    const auto pipe = fn()
        >>= seq::enumerate
        >>= seq::drop_last(2)
        >>= seq::reverse
        >>= seq::maybe_front
        >>= opt::transform(fn(get_element<1>, &Person::name))
        >>= opt::value
        >>= seq::slice(1, 3);

    algorithm::copy(
        persons >>= pipe,
        ostream_iterator{ std::cout, "\n" });
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