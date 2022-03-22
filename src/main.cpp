#include <cmath>
#include <cpp_pipelines/algorithm.hpp>
#include <cpp_pipelines/debug.hpp>
#include <cpp_pipelines/functions.hpp>
#include <cpp_pipelines/macros.hpp>
#include <cpp_pipelines/map.hpp>
#include <cpp_pipelines/opt.hpp>
#include <cpp_pipelines/output.hpp>
#include <cpp_pipelines/predicates.hpp>
#include <cpp_pipelines/res.hpp>
#include <cpp_pipelines/semiregular.hpp>
#include <cpp_pipelines/seq.hpp>
#include <cpp_pipelines/source_location.hpp>
#include <cpp_pipelines/sub.hpp>
#include <cpp_pipelines/tap.hpp>
#include <cpp_pipelines/var.hpp>
#include <forward_list>
#include <fstream>
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
    std::vector<std::string> children;

    friend std::ostream& operator<<(std::ostream& os, const Person& item)
    {
        using namespace cpp_pipelines;
        return os << "Person{ name=" << item.name << " age=" << item.age << " children=[" << delimit(item.children, ", ") << "] }";
    }
};

auto zero_padded(std::ptrdiff_t n) -> cpp_pipelines::ostream_manipulator
{
    return [=](std::ostream& os) { os << std::setw(n) << std::setfill('0'); };
}

template <class T>
std::string build_type_name()
{
    using TR = std::remove_reference_t<T>;
    std::string r = demangle(typeid(TR).name());
    if (std::is_const_v<TR>)
        r += " const";
    if (std::is_volatile_v<TR>)
        r += " volatile";
    if (std::is_lvalue_reference_v<T>)
        r += "&";
    else if (std::is_rvalue_reference_v<T>)
        r += "&&";
    return r;
}

using namespace cpp_pipelines;

std::ostream& operator<<(std::ostream& os, const std::forward_iterator_tag)
{
    return os << "forward_iterator_tag";
}

std::ostream& operator<<(std::ostream& os, const std::input_iterator_tag)
{
    return os << "input_iterator_tag";
}

std::ostream& operator<<(std::ostream& os, const std::bidirectional_iterator_tag)
{
    return os << "bidirectional_iterator_tag";
}

std::ostream& operator<<(std::ostream& os, const std::random_access_iterator_tag)
{
    return os << "random_access_iterator_tag";
}

template <class T>
auto yield_if(bool condition, T item)
{
    return condition
               ? std::vector<T>{ std::move(item) }
               : std::vector<T>{};
}

struct transform_y
{
    int x, z;

    auto operator()(int y) const
    {
        return yield_if(x * x + y * y == z * z, std::tuple{ x, y, z });
    }
};

struct transform_x
{
    int z;

    auto operator()(int x) const
    {
        return seq::iota(x, z + 1) >>= seq::transform_join(transform_y{ x, z }) >>= seq::collect;
    }
};

struct transform_z
{
    auto operator()(int z) const
    {
        return seq::iota(1, z + 1) >>= seq::transform_join(transform_x{ z }) >>= seq::collect;
    }
};

const auto triples = seq::iota(1, std::numeric_limits<int>::max()) >>= seq::transform_join(transform_z{});

template <class T>
std::string_view type_name()
{
    static const std::string name = build_type_name<T>();
    return name;
}

void run()
{
    using namespace std::string_literals;
    using namespace cpp_pipelines;
    namespace p = cpp_pipelines::predicates;
    using p::__;

    std::vector<Person> persons{
        Person{ "Adam", 10, { "A1", "A2", "A3" } },
        Person{ "Bartek", 13 },
        Person{ "-23", 13 },
        Person{ "Celina", 24 },
        Person{ "542", 24 },
        Person{ "Daria", -1 },
        Person{ "Ewa", 64, { "E1" } },
        Person{ "912", 24 },
        Person{ "Helena", 24 },
        Person{ "irena", 49 },
    };

<<<<<<< Updated upstream
    const auto triples = seq::iota(1, std::numeric_limits<int>::max())
        >>= seq::transform_join([](int z)
        {
            return seq::iota(1, z + 1)
                >>= seq::transform_join([=](int x)
                {
                    return seq::iota(x, z + 1)
                        >>= seq::transform_maybe([=](int y)
                        {
                            return x * x + y * y == z * z
                                ? std::optional{std::tuple{x, y, z }}
                                : std::nullopt;
                        });
                });
        });

    triples
        >>= seq::enumerate
        >>= seq::transform([](auto i, const auto& tuple)
        {
            auto [x, y, z] = tuple;
            return str("[", i, "] ", x, ", ", y, ", ", z);
        })
        >>= seq::take(10)
=======
    triples
        >>= seq::take(30)
>>>>>>> Stashed changes
        >>= seq::copy(ostream_iterator{ std::cout, "\n" });
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