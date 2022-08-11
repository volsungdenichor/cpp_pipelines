
#include <cmath>
#include <cpp_pipelines/algorithm.hpp>
#include <cpp_pipelines/container_utils.hpp>
#include <cpp_pipelines/debug.hpp>
#include <cpp_pipelines/functions.hpp>
#include <cpp_pipelines/integer_types.hpp>
#include <cpp_pipelines/iterable.hpp>
#include <cpp_pipelines/log.hpp>
#include <cpp_pipelines/macros.hpp>
#include <cpp_pipelines/map.hpp>
#include <cpp_pipelines/operators.hpp>
#include <cpp_pipelines/opt.hpp>
#include <cpp_pipelines/output.hpp>
#include <cpp_pipelines/pattern_matching.hpp>
#include <cpp_pipelines/predicates.hpp>
#include <cpp_pipelines/res.hpp>
#include <cpp_pipelines/scope_functions.hpp>
#include <cpp_pipelines/semiregular.hpp>
#include <cpp_pipelines/seq.hpp>
#include <cpp_pipelines/set.hpp>
#include <cpp_pipelines/source_location.hpp>
#include <cpp_pipelines/sub.hpp>
#include <cpp_pipelines/tpl.hpp>
#include <cpp_pipelines/var.hpp>
#include <forward_list>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <locale>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <typeindex>
#include <variant>

using namespace std::string_literals;
using namespace std::string_view_literals;

struct parse_error
{
    std::string text;
};

inline std::ostream& operator<<(std::ostream& os, const parse_error& item)
{
    return os << "parse_error: cannot parse '" << item.text << "'";
}

template <class T>
struct parse_fn
{
    cpp_pipelines::result<T, parse_error> operator()(std::string_view text) const
    {
        const auto s = std::string{ text };
        std::stringstream ss{ s };
        T result;
        ss >> result;
        if (ss)
            return std::move(result);
        else
            return cpp_pipelines::error(parse_error{ s });
    }
};

template <class T>
static constexpr inline auto parse = parse_fn<T>{};

struct square_root_fn
{
    struct domain_error
    {
    };

    using error_type = domain_error;

    cpp_pipelines::result<double, error_type> operator()(double x) const
    {
        if (x < 0.0)
            return cpp_pipelines::error(domain_error{});
        return std::sqrt(x);
    }
};

inline std::ostream& operator<<(std::ostream& os, const square_root_fn::error_type& item)
{
    return os << "square_root::domain_error";
}

static constexpr inline auto square_root = square_root_fn{};

enum class Sex
{
    male,
    female
};

struct Life
{
    int birth = 0;
    int death = 0;

    friend std::ostream& operator<<(std::ostream& os, const Life& item)
    {
        return os << item.birth << " - " << item.death;
    }
};

struct Person
{
    std::string first_name;
    std::string last_name;
    Life life;
    Sex sex;

    friend std::ostream& operator<<(std::ostream& os, const Person& item)
    {
        return os << item.last_name << ", " << item.first_name << " (" << item.life << ")";
    }
};

auto zero_padded(std::ptrdiff_t n) -> cpp_pipelines::ostream_manipulator
{
    return [=](std::ostream& os) { os << std::setw(n) << std::setfill('0'); };
}

constexpr auto pythagorean_triples()
{
    using namespace cpp_pipelines;
    return seq::iota(1)
           >>= seq::transform_join([](int z) {
                   return seq::iota(1, z + 1)
                          >>= seq::transform_join([=](int x) {
                                  return seq::iota(x, z + 1)
                                         >>= seq::transform([=](int y) { return std::tuple{ x, y, z }; })
                                         >>= seq::filter([](int x, int y, int z) { return x * x + y * y == z * z; });
                              });
               });
}

constexpr auto fibonacci_series()
{
    using namespace cpp_pipelines;
    return seq::unfold_infinite(std::pair{ 1, 1 }, [](const auto& pair) {
        const auto [a, b] = pair;
        return std::pair{ a, std::pair{ b, a + b } };
    });
}

const std::vector<Person> persons = {
    { "Adam", "Asnyk", { 1838, 1897 }, Sex::male },
    { "Krzysztof Kamil", "Baczyński", { 1921, 1944 }, Sex::male },
    { "Stanisław", "Barańczak", { 1946, 2014 }, Sex::male },
    { "Miron", "Białoszewski", { 1922, 1983 }, Sex::male },
    { "Tadeusz", "Boy-Żeleński", { 1874, 1941 }, Sex::male },
    { "Władysław", "Broniewski", { 1897, 1962 }, Sex::male },
    { "Jan", "Brzechwa", { 1898, 1966 }, Sex::male },
    { "Andrzej", "Bursa", { 1932, 1957 }, Sex::male },
    { "Józef", "Czechowicz", { 1903, 1939 }, Sex::male },
    { "Aleksander", "Fredro", { 1793, 1876 }, Sex::male },
    { "Tadeusz", "Gajcy", { 1922, 1944 }, Sex::male },
    { "Konstanty Ildefons", "Gałczyński", { 1905, 1953 }, Sex::male },
    { "Stanisław", "Grochowiak", { 1934, 1976 }, Sex::male },
    { "Marian", "Hemar", { 1901, 1972 }, Sex::male },
    { "Zbigniew", "Herbert", { 1924, 1998 }, Sex::male },
    { "Jarosław", "Iwaszkiewicz", { 1894, 1980 }, Sex::male },
    { "Bruno", "Jasieński", { 1901, 1938 }, Sex::male },
    { "Mieczysław", "Jastrun", { 1903, 1983 }, Sex::male },
    { "Anna", "Kamieńska", { 1920, 1986 }, Sex::female },
    { "Franciszek", "Karpiński", { 1741, 1825 }, Sex::male },
    { "Jan", "Kasprowicz", { 1860, 1936 }, Sex::male },
    { "Jan", "Kochanowski", { 1530, 1584 }, Sex::male },
    { "Maria", "Konopnicka", { 1842, 1910 }, Sex::female },
    { "Ignacy", "Krasicki", { 1735, 1801 }, Sex::male },
    { "Zygmunt", "Krasiński", { 1812, 1859 }, Sex::male },
    { "Jalu", "Kurek", { 1904, 1983 }, Sex::male },
    { "Jan", "Lechoń", { 1899, 1956 }, Sex::male },
    { "Bolesław", "Leśmian", { 1877, 1937 }, Sex::male },
    { "Antoni", "Malczewski", { 1793, 1826 }, Sex::male },
    { "Tadeusz", "Miciński", { 1873, 1918 }, Sex::male },
    { "Adam", "Mickiewicz", { 1798, 1855 }, Sex::male },
    { "Czesław", "Miłosz", { 1911, 2004 }, Sex::male },
    { "Stanisław", "Młodożeniec", { 1895, 1959 }, Sex::male },
    { "Jan Andrzej", "Morsztyn", { 1621, 1693 }, Sex::male },
    { "Zbigniew", "Morsztyn", { 1628, 1689 }, Sex::male },
    { "Daniel", "Naborowski", { 1573, 1640 }, Sex::male },
    { "Adam", "Naruszewicz", { 1733, 1796 }, Sex::male },
    { "Julian Ursyn", "Niemcewicz", { 1758, 1841 }, Sex::male },
    { "Cyprian Kamil", "Norwid", { 1821, 1883 }, Sex::male },
    { "Władysław", "Orkan", { 1875, 1930 }, Sex::male },
    { "Agnieszka", "Osiecka", { 1936, 1997 }, Sex::female },
    { "Leon", "Pasternak", { 1910, 1969 }, Sex::male },
    { "Maria", "Pawlikowska-Jasnorzewska", { 1891, 1945 }, Sex::female },
    { "Wincenty", "Pol", { 1807, 1872 }, Sex::male },
    { "Halina", "Poświatowska", { 1935, 1967 }, Sex::female },
    { "Wacław", "Potocki", { 1621, 1696 }, Sex::male },
    { "Kazimierz", "Tetmajer", { 1865, 1940 }, Sex::male },
    { "Zenon", "Przesmycki", { 1861, 1944 }, Sex::male },
    { "Jeremi", "Przybora", { 1915, 2004 }, Sex::male },
    { "Mikołaj", "Rej", { 1505, 1569 }, Sex::male },
    { "Tadeusz", "Różewicz", { 1921, 2014 }, Sex::male },
    { "Jarosław Marek", "Rymkiewicz", { 1935, 2022 }, Sex::male },
    { "Antoni", "Słonimski", { 1895, 1976 }, Sex::male },
    { "Juliusz", "Słowacki", { 1809, 1849 }, Sex::male },
    { "Edward", "Stachura", { 1937, 1979 }, Sex::male },
    { "Leopold", "Staff", { 1878, 1957 }, Sex::male },
    { "Władysław", "Syrokomla", { 1823, 1862 }, Sex::male },
    { "Włodzimierz", "Szymanowicz", { 1946, 1967 }, Sex::male },
    { "Wisława", "Szymborska", { 1923, 2012 }, Sex::female },
    { "Julian", "Tuwim", { 1894, 1953 }, Sex::male },
    { "Jan", "Twardowski", { 1915, 2006 }, Sex::male },
    { "Kornel", "Ujejski", { 1823, 1897 }, Sex::male },
    { "Aleksander", "Wat", { 1900, 1967 }, Sex::male },
    { "Adam", "Ważyk", { 1905, 1982 }, Sex::male },
    { "Kazimierz", "Wierzyński", { 1894, 1969 }, Sex::male },
    { "Stanisław Ignacy", "Witkiewicz", { 1885, 1939 }, Sex::male },
    { "Stefan", "Witwicki", { 1801, 1847 }, Sex::male },
    { "Rafał", "Wojaczek", { 1945, 1971 }, Sex::male },
    { "Stanisław", "Wyspiański", { 1869, 1907 }, Sex::male },
    { "Adam", "Zagajewski", { 1945, 2021 }, Sex::male },
    { "Emil", "Zegadłowicz", { 1888, 1941 }, Sex::male },
    { "Narcyza", "Żmichowska", { 1819, 1876 }, Sex::female },
    { "Jerzy", "Żuławski", { 1874, 1915 }, Sex::male },
};

auto split(std::string_view text, char delim = ',') -> std::optional<std::pair<std::string_view, std::string_view>>
{
    using namespace cpp_pipelines;
    const auto s = text
        >>= seq::split_on_element(delim)
        >>= seq::transform(cast<std::string_view>)
        >>= seq::take(2)
        >>= seq::to<std::vector>;
    return s.size() == 2
               ? std::optional{ std::pair{ s[0], s[1] } }
               : std::nullopt;
}

cpp_pipelines::log::value_wrapper<int> calculate_odd(int x)
{
    return {
        x * 100,
        { "Calculating for odd", "Multiplying by 100" }
    };
}

cpp_pipelines::log::value_wrapper<int> calculate_even(int x)
{
    return {
        x * 1000,
        { "Calculating for even", "Multiplying by 1000" }
    };
}

cpp_pipelines::log::value_wrapper<int> calculate(int x)
{
    using namespace cpp_pipelines;
    if (x % 2 == 0)
    {
        return x
               >>= log::lift
               >>= log::append_logs({ "EVEN", "xxx", "yyy" })
               >>= log::and_then(&calculate_even);
    }
    else
    {
        return x >>= log::lift >>= log::and_then(&calculate_odd);
    }
}

void run()
{
    using namespace cpp_pipelines;
    using namespace cpp_pipelines::pattern_matching;
    namespace p = cpp_pipelines::predicates;

    const auto handler = [](const log::log_t& item) { std::cout << "|" << item << "\n"; };

    const auto logged = calculate_even(10)
        >>= log::and_then([](int x) { return log::lift(x + 1) >>= log::append_logs({ "Lambda" }); });
    const auto res = logged
        // >>= log::flush(handler)
        >>= log::value;
    std::cout << "res=" << res << std::endl;
}

int main()
{
    try
    {
        run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "exception: " << e.what() << '\n';
    }
}