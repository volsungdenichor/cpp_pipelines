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
    cpp_pipelines::result<T, parse_error> operator()(std::string text) const
    {
        std::stringstream ss{ text };
        T result;
        ss >> result;
        if (ss)
            return std::move(result);
        else
            return cpp_pipelines::error(parse_error{ text });
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

enum class Sex
{
    male,
    female
};

struct Person
{
    std::string first_name;
    std::string last_name;
    int birth = 0;
    int death = 0;
    Sex sex;

    friend std::ostream& operator<<(std::ostream& os, const Person& item)
    {
        return os << item.last_name << ", " << item.first_name << " (" << item.birth << " - " << item.death << ")";
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

const std::vector<Person> persons = {
    { "Adam", "Asnyk", 1838, 1897, Sex::male },
    { "Krzysztof Kamil", "Baczyński", 1921, 1944, Sex::male },
    { "Stanisław", "Barańczak", 1946, 2014, Sex::male },
    { "Miron", "Białoszewski", 1922, 1983, Sex::male },
    { "Tadeusz", "Boy-Żeleński", 1874, 1941, Sex::male },
    { "Władysław", "Broniewski", 1897, 1962, Sex::male },
    { "Jan", "Brzechwa", 1898, 1966, Sex::male },
    { "Andrzej", "Bursa", 1932, 1957, Sex::male },
    { "Józef", "Czechowicz", 1903, 1939, Sex::male },
    { "Aleksander", "Fredro", 1793, 1876, Sex::male },
    { "Tadeusz", "Gajcy", 1922, 1944, Sex::male },
    { "Konstanty Ildefons", "Gałczyński", 1905, 1953, Sex::male },
    { "Stanisław", "Grochowiak", 1934, 1976, Sex::male },
    { "Marian", "Hemar", 1901, 1972, Sex::male },
    { "Zbigniew", "Herbert", 1924, 1998, Sex::male },
    { "Jarosław", "Iwaszkiewicz", 1894, 1980, Sex::male },
    { "Bruno", "Jasieński", 1901, 1938, Sex::male },
    { "Mieczysław", "Jastrun", 1903, 1983, Sex::male },
    { "Anna", "Kamieńska", 1920, 1986, Sex::female },
    { "Franciszek", "Karpiński", 1741, 1825, Sex::male },
    { "Jan", "Kasprowicz", 1860, 1936, Sex::male },
    { "Jan", "Kochanowski", 1530, 1584, Sex::male },
    { "Maria", "Konopnicka", 1842, 1910, Sex::female },
    { "Ignacy", "Krasicki", 1735, 1801, Sex::male },
    { "Zygmunt", "Krasiński", 1812, 1859, Sex::male },
    { "Jalu", "Kurek", 1904, 1983, Sex::male },
    { "Jan", "Lechoń", 1899, 1956, Sex::male },
    { "Bolesław", "Leśmian", 1877, 1937, Sex::male },
    { "Antoni", "Malczewski", 1793, 1826, Sex::male },
    { "Tadeusz", "Miciński", 1873, 1918, Sex::male },
    { "Adam", "Mickiewicz", 1798, 1855, Sex::male },
    { "Czesław", "Miłosz", 1911, 2004, Sex::male },
    { "Stanisław", "Młodożeniec", 1895, 1959, Sex::male },
    { "Jan Andrzej", "Morsztyn", 1621, 1693, Sex::male },
    { "Zbigniew", "Morsztyn", 1628, 1689, Sex::male },
    { "Daniel", "Naborowski", 1573, 1640, Sex::male },
    { "Adam", "Naruszewicz", 1733, 1796, Sex::male },
    { "Julian Ursyn", "Niemcewicz", 1758, 1841, Sex::male },
    { "Cyprian Kamil", "Norwid", 1821, 1883, Sex::male },
    { "Władysław", "Orkan", 1875, 1930, Sex::male },
    { "Agnieszka", "Osiecka", 1936, 1997, Sex::female },
    { "Leon", "Pasternak", 1910, 1969, Sex::male },
    { "Maria", "Pawlikowska-Jasnorzewska", 1891, 1945, Sex::female },
    { "Wincenty", "Pol", 1807, 1872, Sex::male },
    { "Halina", "Poświatowska", 1935, 1967, Sex::female },
    { "Wacław", "Potocki", 1621, 1696, Sex::male },
    { "Kazimierz", "Tetmajer", 1865, 1940, Sex::male },
    { "Zenon", "Przesmycki", 1861, 1944, Sex::male },
    { "Jeremi", "Przybora", 1915, 2004, Sex::male },
    { "Mikołaj", "Rej", 1505, 1569, Sex::male },
    { "Tadeusz", "Różewicz", 1921, 2014, Sex::male },
    { "Jarosław Marek", "Rymkiewicz", 1935, 2022, Sex::male },
    { "Antoni", "Słonimski", 1895, 1976, Sex::male },
    { "Juliusz", "Słowacki", 1809, 1849, Sex::male },
    { "Edward", "Stachura", 1937, 1979, Sex::male },
    { "Leopold", "Staff", 1878, 1957, Sex::male },
    { "Władysław", "Syrokomla", 1823, 1862, Sex::male },
    { "Włodzimierz", "Szymanowicz", 1946, 1967, Sex::male },
    { "Wisława", "Szymborska", 1923, 2012, Sex::female },
    { "Julian", "Tuwim", 1894, 1953, Sex::male },
    { "Jan", "Twardowski", 1915, 2006, Sex::male },
    { "Kornel", "Ujejski", 1823, 1897, Sex::male },
    { "Aleksander", "Wat", 1900, 1967, Sex::male },
    { "Adam", "Ważyk", 1905, 1982, Sex::male },
    { "Kazimierz", "Wierzyński", 1894, 1969, Sex::male },
    { "Stanisław Ignacy", "Witkiewicz", 1885, 1939, Sex::male },
    { "Stefan", "Witwicki", 1801, 1847, Sex::male },
    { "Rafał", "Wojaczek", 1945, 1971, Sex::male },
    { "Stanisław", "Wyspiański", 1869, 1907, Sex::male },
    { "Adam", "Zagajewski", 1945, 2021, Sex::male },
    { "Emil", "Zegadłowicz", 1888, 1941, Sex::male },
    { "Narcyza", "Żmichowska", 1819, 1876, Sex::female },
    { "Jerzy", "Żuławski", 1874, 1915, Sex::male },
};

class event_aggregator
{
public:
    using sub_id = int;
    struct context
    {
        context(sub_id id, int counter)
            : id{ id }
            , counter{ counter }
            , pending_unsubscription_{ false }
        {
        }

        void unsubscribe()
        {
            pending_unsubscription_ = true;
        }

        bool is_pending_unsubscription() const
        {
            return pending_unsubscription_;
        }

        const sub_id id;
        const int counter;

    private:
        bool pending_unsubscription_;
    };
    template <class E>
    using event_handler = std::function<void(context&, const E&)>;

    event_aggregator() = default;
    event_aggregator(const event_aggregator&) = delete;
    event_aggregator(event_aggregator&&) = default;

    template <class E>
    sub_id subscribe(event_handler<E> event_handler)
    {
        sub_id id = acquire_id();
        handler_base handler = [event_handler](context& ctx, const void* e) {
            event_handler(ctx, *static_cast<const E*>(e));
        };

        _map.emplace(get_type<E>(), sub_info{ id, std::move(handler), 0 });
        return id;
    }

    template <class E>
    void publish(const E& event)
    {
        const void* e = static_cast<const void*>(&event);
        auto [begin, end] = _map.equal_range(get_type<E>());

        std::vector<sub_id> unsubscribed;
        unsubscribed.reserve(std::distance(begin, end));

        std::for_each(begin, end, [this, e, &unsubscribed](auto& entry) {
            sub_info& sub_info = entry.second;
            context ctx{ sub_info.id, ++sub_info.counter };
            sub_info.handler(ctx, e);
            if (ctx.is_pending_unsubscription())
            {
                unsubscribed.push_back(sub_info.id);
            }
        });

        for (sub_id id : unsubscribed)
        {
            unsubscribe(id);
        }
    }

    void unsubscribe(sub_id id)
    {
        for (auto it = _map.begin(), end = _map.end(); it != end;)
        {
            if (it->second.id == id)
            {
                it = _map.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

private:
    using handler_base = std::function<void(context&, const void*)>;

    template <class T>
    static std::type_index get_type()
    {
        return { typeid(T) };
    }

    sub_id acquire_id()
    {
        return _next_id++;
    }

    struct sub_info
    {
        sub_id id;
        handler_base handler;
        int counter;
    };

    std::multimap<std::type_index, sub_info> _map;
    sub_id _next_id = 1;
};

struct A
{
    int x;
};

void run()
{
    using namespace cpp_pipelines;
    namespace p = cpp_pipelines::predicates;
    using p::__;

    event_aggregator e{};
    e.subscribe<A>([](event_aggregator::context& ctx, const A& e) {
        if (ctx.counter == 2)
            ctx.unsubscribe();
        std::cout << e.x << std::endl;
    });

    e.subscribe<A>([](event_aggregator::context& ctx, const A& e) {
        std::cout << "    " << e.x << std::endl;
    });

    e.publish(A{ 3 });
    e.publish(A{ 5 });
    e.publish(A{ 7 });
    e.publish(A{ 9 });
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