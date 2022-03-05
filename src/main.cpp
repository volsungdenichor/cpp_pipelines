#include <cmath>
#include <cpp_pipelines/algorithm.hpp>
#include <cpp_pipelines/debug.hpp>
#include <cpp_pipelines/functions.hpp>
#include <cpp_pipelines/opt.hpp>
#include <cpp_pipelines/seq.hpp>
#include <cpp_pipelines/tap.hpp>
#include <forward_list>
#include <iostream>
#include <locale>
#include <optional>
#include <sstream>
#include <variant>

struct str_fn
{
    template <class... Args>
    std::string operator()(const Args&... args) const
    {
        std::ostringstream ss;
        (ss << ... << args);
        return std::move(ss).str();
    }
};

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

static constexpr inline auto str = str_fn{};

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

struct cout
{
    std::string_view prefix = "";
    std::string_view suffix = "";

    template <class T>
    std::monostate operator()(const T& item) const
    {
        std::cout << prefix << item << suffix << std::endl;
        return {};
    }
};

struct decorate_string
{
    std::string_view prefix = "";
    std::string_view suffix = "";

    template <class T>
    std::string operator()(const T& item) const
    {
        return str(prefix, item, suffix);
    }
};

struct Person
{
    std::string name;
    int age;

    friend std::ostream& operator<<(std::ostream& os, const Person& item)
    {
        return os << "Person{ name=" << item.name << " age=" << item.age << " }";
    }
};

#define ITER(cat)                                   \
    std::ostream& operator<<(std::ostream& os, cat) \
    {                                               \
        return os << #cat;                          \
    }

ITER(std::input_iterator_tag)
ITER(std::forward_iterator_tag)
ITER(std::bidirectional_iterator_tag)
ITER(std::random_access_iterator_tag)

void run()
{
    using namespace std::string_literals;
    using namespace cpp_pipelines;

    std::vector<Person> persons{
        Person{ "Adam", 10 },
        Person{ "Bartek", 13 },
    };

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