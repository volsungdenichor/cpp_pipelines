

#include <cpp_pipelines/functions.hpp>
#include <cpp_pipelines/macros.hpp>
#include <cpp_pipelines/seq.hpp>
#include <cpp_pipelines/tpl.hpp>

#include "test_utils.hpp"

using namespace cpp_pipelines;
using namespace std::string_literals;
using namespace std::string_view_literals;

auto fibonacci()
{
    using state_type = std::pair<int, int>;
    return seq::unfold_infinite(state_type{ 1, 1 }, [](state_type state) -> std::pair<int, state_type> {
        const auto [a, b] = state;
        return std::pair{ a, state_type{ b, a + b } };
    });
}

TEST_CASE("seq::transform", "[seq][transform]")
{
    REQUIRE_THAT((std::vector<std::string>{} >>= seq::transform(&std::string::size)), EqualsRange(std::vector<std::size_t>{}));
    REQUIRE_THAT((std::vector{ "Mercury"s, "Venus"s, "Earth"s, "Mars"s } >>= seq::transform(&std::string::size)), EqualsRange(std::vector<std::size_t>{ 7, 5, 5, 4 }));
    REQUIRE_THAT(("Jupiter"s >>= seq::transform(LIFT(std::toupper))), EqualsRange("JUPITER"s));
}

TEST_CASE("seq::transform_maybe", "[seq][transform_maybe]")
{
    static const auto even = [](int v) -> std::optional<int> {
        return v % 2 == 0
                   ? std::optional{ v }
                   : std::nullopt;
    };
    REQUIRE_THAT((std::vector<int>{} >>= seq::transform_maybe(even)), EqualsRange(std::vector<int>{}));
    REQUIRE_THAT((std::vector{ 1, 2, 3, 4, 5, 6, 8 } >>= seq::transform_maybe(even)), EqualsRange(std::vector{ 2, 4, 6, 8 }));
}

TEST_CASE("seq::transform_maybe - function returning pointer", "[seq][transform_maybe]")
{
    static const auto is_empty = [](const std::string& v) -> const std::string* {
        return !v.empty() ? std::addressof(v) : nullptr;
    };

    REQUIRE_THAT((std::vector{ "A"s, ""s, "Xyz"s, ""s, "B"s } >>= seq::transform_maybe(is_empty)), EqualsRange(std::vector{ "A"s, "Xyz"s, "B"s }));
}

TEST_CASE("seq::filter", "[seq][filter]")
{
    static const auto is_even = [](int v) -> bool {
        return v % 2 == 0;
    };
    REQUIRE_THAT((std::vector<int>{} >>= seq::filter(is_even)), EqualsRange(std::vector<int>{}));
    REQUIRE_THAT((std::vector{ 1, 2, 3, 4, 5, 6, 8 } >>= seq::filter(is_even)), EqualsRange(std::vector{ 2, 4, 6, 8 }));
}

TEST_CASE("seq::filter - reverse iterator", "[seq][filter]")
{
    static const auto is_even = [](int v) -> bool {
        return v % 2 == 0;
    };
    REQUIRE_THAT((std::vector<int>{} >>= seq::filter(is_even)), EqualsRange(std::vector<int>{}));
    REQUIRE_THAT((std::vector{ 1, 2, 3, 4, 5, 6, 8 } >>= seq::reverse >>= seq::filter(is_even)), EqualsRange(std::vector{ 8, 6, 4, 2 }));
}

#if 0
TEST_CASE("seq::unique", "[seq][unique]")
{
    REQUIRE((""s >>= seq::unique >>= seq::to_string) == ""s);
    REQUIRE(("123344445"s >>= seq::unique >>= seq::to_string) == "12345"s);
}

TEST_CASE("seq::unique_by_key", "[seq][unique][unique_by_key][by_key]")
{
    static const auto to_lower = LIFT(std::tolower);
    REQUIRE((""s >>= seq::unique_by_key(to_lower) >>= seq::to_string) == ""s);
    REQUIRE(("AaACDdDe"s >>= seq::unique_by_key(to_lower) >>= seq::to_string) == "ACDe"s);
}

TEST_CASE("seq::unique_by", "[seq][unique][unique_by][by]")
{
    const auto cmp = [](char lhs, char rhs) {
        return std::abs(std::tolower(lhs) - std::tolower(rhs)) <= 1;
    };
    REQUIRE((""s >>= seq::unique_by(cmp) >>= seq::to_string) == ""s);
    REQUIRE(("AaACDdFhH"s >>= seq::unique_by(cmp) >>= seq::to_string) == "ACFh"s);
}
#endif

TEST_CASE("seq::chunk", "[seq][chunk]")
{
    REQUIRE_THAT((""s >>= seq::chunk(3) >>= seq::transform(seq::to_string)), EqualsRange(std::vector<std::string>{}));
    REQUIRE_THAT(("12"s >>= seq::chunk(3) >>= seq::transform(seq::to_string)), EqualsRange(std::vector{ "12"s }));
    REQUIRE_THAT(("1234"s >>= seq::chunk(3) >>= seq::transform(seq::to_string)), EqualsRange(std::vector{ "123"s, "4"s }));
    REQUIRE_THAT(("123456"s >>= seq::chunk(3) >>= seq::transform(seq::to_string)), EqualsRange(std::vector{ "123"s, "456"s }));
}

TEST_CASE("seq::slide", "[seq][slide]")
{
    REQUIRE_THAT((""s >>= seq::slide(3) >>= seq::transform(seq::to_string)), EqualsRange(std::vector<std::string>{}));
    REQUIRE_THAT(("12"s >>= seq::slide(3) >>= seq::transform(seq::to_string)), EqualsRange(std::vector{ "12"s, "2"s }));
    REQUIRE_THAT(("1234"s >>= seq::slide(3) >>= seq::transform(seq::to_string)), EqualsRange(std::vector{ "123"s, "234"s, "34"s, "4"s }));
}

TEST_CASE("seq::chunk_by", "[seq][chunk][chunk_by][by]")
{
    static const auto cmp = [](char lhs, char rhs) {
        return std::tolower(lhs) == std::tolower(rhs);
    };
    REQUIRE_THAT((""s >>= seq::chunk_by(cmp) >>= seq::transform(seq::to_string)), EqualsRange(std::vector<std::string>{}));
    REQUIRE_THAT(("AaabBbcccDDD"s >>= seq::chunk_by(cmp) >>= seq::transform(seq::to_string)), EqualsRange(std::vector{ "Aaa"s, "bBb"s, "ccc"s, "DDD"s }));
}

TEST_CASE("seq::chunk_by_key", "[seq][chunk][chunk_by_key][by_key]")
{
    static const auto to_lower = LIFT(std::tolower);
    REQUIRE_THAT((""s >>= seq::chunk_by_key(to_lower) >>= seq::transform(seq::to_string)), EqualsRange(std::vector<std::string>{}));
    REQUIRE_THAT(("AaACDdDe"s >>= seq::chunk_by_key(to_lower) >>= seq::transform(seq::to_string)), EqualsRange(std::vector{ "AaA"s, "C"s, "DdD"s, "e"s }));
}

TEST_CASE("seq::split_when", "[seq][split][split_when]")
{
    static const auto is_delimiter = [](char ch) {
        static const std::string delimiters = ".,;:!?";
        return std::isspace(ch) || delimiters.find(ch) != std::string::npos;
    };
    REQUIRE_THAT((""s >>= seq::split_when(is_delimiter) >>= seq::transform(seq::to_string)), EqualsRange(std::vector<std::string>{}));
    REQUIRE_THAT(("abc"s >>= seq::split_when(is_delimiter) >>= seq::transform(seq::to_string)), EqualsRange(std::vector{ "abc"s }));
    REQUIRE_THAT(("abc def,ghi jkl; mno"s >>= seq::split_when(is_delimiter) >>= seq::transform(seq::to_string)), EqualsRange(std::vector{ "abc"s, "def"s, "ghi"s, "jkl"s, ""s, "mno"s }));
}

TEST_CASE("seq::split_on_element", "[seq][split][split_on_element]")
{
    REQUIRE_THAT((""s >>= seq::split_on_element(',') >>= seq::transform(seq::to_string)), EqualsRange(std::vector<std::string>{}));
    REQUIRE_THAT(("abc"s >>= seq::split_on_element(',') >>= seq::transform(seq::to_string)), EqualsRange(std::vector{ "abc"s }));
    REQUIRE_THAT(("abc,def,ghi,jkl,mno"s >>= seq::split_on_element(',') >>= seq::transform(seq::to_string)), EqualsRange(std::vector{ "abc"s, "def"s, "ghi"s, "jkl"s, "mno"s }));
}

TEST_CASE("seq::split_on_subrange", "[seq][split][split_on_subrange]")
{
    REQUIRE_THAT((""s >>= seq::split_on_subrange(", "sv) >>= seq::transform(seq::to_string)), EqualsRange(std::vector<std::string>{}));
    REQUIRE_THAT(("abc"s >>= seq::split_on_subrange(", "sv) >>= seq::transform(seq::to_string)), EqualsRange(std::vector{ "abc"s }));
    REQUIRE_THAT(("abc, def, ghi, jkl, mno"s >>= seq::split_on_subrange(", "sv) >>= seq::transform(seq::to_string)), EqualsRange(std::vector{ "abc"s, "def"s, "ghi"s, "jkl"s, "mno"s }));
}

TEST_CASE("seq::zip", "[seq][zip]")
{
    REQUIRE_THAT(
        (std::tuple{ std::vector{ 1, 2, 3 }, "ABC"s } |= seq::zip),
        (EqualsRange(std::vector{ std::tuple{ 1, 'A' }, std::tuple{ 2, 'B' }, std::tuple{ 3, 'C' } })));
}

TEST_CASE("seq::zip_transform", "[seq][zip][zip_transform][transform]")
{
    const auto func = [](int a, char b, char c) {
        return str(a, " ", b, " ", c);
    };
    REQUIRE_THAT((std::tuple{ std::vector{ 1, 2, 3 }, "ABC"s, "12345"s } |= seq::zip_transform(func)), EqualsRange(std::vector{ "1 A 1"s, "2 B 2"s, "3 C 3"s }));
}

TEST_CASE("seq::enumerate", "[seq][enumerate]")
{
    REQUIRE_THAT(("ABCD"s >>= seq::enumerate), EqualsRange(std::vector{ std::pair{ 0, 'A' }, std::pair{ 1, 'B' }, std::pair{ 2, 'C' }, std::pair{ 3, 'D' } }));
}

TEST_CASE("seq::transform_join", "[seq][transform_join][join][transform]")
{
    const auto func = [](char c) {
        return std::string(2, std::toupper(c));
    };
    REQUIRE_THAT(("abcd"s >>= seq::transform_join(func)), EqualsRange("AABBCCDD"s));
}

TEST_CASE("seq::join", "[seq][join]")
{
    REQUIRE_THAT((std::vector{ "Alpha"s, "Beta"s, "Gamma"s } >>= seq::join), EqualsRange("AlphaBetaGamma"s));
}

TEST_CASE("seq::join_with", "[seq][join_with][join]")
{
    REQUIRE_THAT((std::vector{ "Alpha"s, "Beta"s, "Gamma"s } >>= seq::join_with(", "sv)), EqualsRange("Alpha, Beta, Gamma"s));
}

TEST_CASE("seq::intersperse", "[seq][intersperse]")
{
    REQUIRE_THAT((std::vector{ 10, 11, 12 } >>= seq::intersperse(-1)), EqualsRange(std::vector{ 10, -1, 11, -1, 12 }));
}

TEST_CASE("seq::adjacent_transform", "[seq][adjacent_transform]")
{
    REQUIRE_THAT((std::vector{ 1, 2, 3, 4, 5 } >>= seq::adjacent_transform<3>([](int a, int b, int c) { return a + b + c; })), EqualsRange(std::vector{ 6, 9, 12 }));
}

TEST_CASE("seq::pairwise_transform", "[seq][adjacent_transform]")
{
    REQUIRE_THAT((std::vector{ 1, 2, 3, 4, 5 } >>= seq::pairwise_transform(std::plus<>{})), EqualsRange(std::vector{ 3, 5, 7, 9 }));
}

TEST_CASE("seq::generate", "[seq][generate]")
{
    const auto f = [cur = 2]() mutable -> std::optional<std::string> {
        if (cur > 100)
            return std::nullopt;
        auto result = str(cur);
        cur *= 2;
        return result;
    };
    REQUIRE_THAT(seq::generate(f), EqualsRange(std::vector{ "2"s, "4"s, "8"s, "16"s, "32"s, "64"s }));
}

TEST_CASE("seq::generate_infinite", "[seq][generate_infinite][generate]")
{
    const auto f = [cur = 2]() mutable -> std::string {
        auto result = str(cur);
        cur *= 2;
        return result;
    };
    REQUIRE_THAT(seq::generate_infinite(f) >>= seq::take(8), EqualsRange(std::vector{ "2"s, "4"s, "8"s, "16"s, "32"s, "64"s, "128"s, "256"s }));
}

TEST_CASE("seq::unfold", "[seq][unfold][generate]")
{
    const auto f = [](int cur) -> std::optional<std::pair<std::string, int>> {
        if (cur > 100)
            return std::nullopt;
        return std::pair{ str(cur), cur * 2 };
    };
    REQUIRE_THAT(seq::unfold(1, f), EqualsRange(std::vector{ "1"s, "2"s, "4"s, "8"s, "16"s, "32"s, "64"s }));
}

TEST_CASE("seq::unfold_infinite", "[seq][unfold_infinite][generate]")
{
    const auto f = [](int cur) -> std::pair<std::string, int> {
        return std::pair{ str(cur), cur * 2 };
    };
    REQUIRE_THAT(seq::unfold_infinite(1, f) >>= seq::take(8), EqualsRange(std::vector{ "1"s, "2"s, "4"s, "8"s, "16"s, "32"s, "64"s, "128"s }));
}

TEST_CASE("seq::take", "[seq][slice]")
{
    REQUIRE_THAT((std::vector{ 1, 2, 3, 4, 5, 6 } >>= seq::take(3)), EqualsRange(std::vector{ 1, 2, 3 }));
}

TEST_CASE("seq::take_while", "[seq][slice]")
{
    REQUIRE_THAT((std::vector{ 1, 2, 3, 4, 5, 6 } >>= seq::take_while([](int x) { return x < 4; })), EqualsRange(std::vector{ 1, 2, 3 }));
}

TEST_CASE("seq::take_last", "[seq][slice]")
{
    REQUIRE_THAT((std::vector{ 1, 2, 3, 4, 5, 6 } >>= seq::take_last(3)), EqualsRange(std::vector{ 4, 5, 6 }));
}

TEST_CASE("seq::drop", "[seq][slice]")
{
    REQUIRE_THAT((std::vector{ 1, 2, 3, 4, 5, 6 } >>= seq::drop(3)), EqualsRange(std::vector{ 4, 5, 6 }));
}

TEST_CASE("seq::drop_while", "[seq][slice]")
{
    REQUIRE_THAT((std::vector{ 1, 2, 3, 4, 5, 6 } >>= seq::drop_while([](int x) { return x < 5; })), EqualsRange(std::vector{ 5, 6 }));
}

TEST_CASE("seq::drop_last", "[seq][slice]")
{
    REQUIRE_THAT((std::vector{ 1, 2, 3, 4, 5, 6 } >>= seq::drop_last(3)), EqualsRange(std::vector{ 1, 2, 3 }));
}

TEST_CASE("seq::stride", "[seq][slice]")
{
    REQUIRE_THAT((fibonacci() >>= seq::stride(3) >>= seq::take(5)), EqualsRange(std::vector{ 1, 3, 13, 55, 233 }));
}

TEST_CASE("seq::reverse", "[seq][reverse]")
{
    REQUIRE_THAT((std::vector{ 1, 2, 3, 4, 5 } >>= seq::reverse), EqualsRange(std::vector{ 5, 4, 3, 2, 1 }));
}

TEST_CASE("seq::getlines", "[seq][getlines")
{
    std::string text = "Mercury\nVenus\nEarth\nMars\nJupiter\nSaturn\nUranus\nNeptune";
    std::stringstream ss{ text };
    REQUIRE_THAT(seq::getlines(ss), EqualsRange(std::vector{ "Mercury"s, "Venus"s, "Earth"s, "Mars"s, "Jupiter"s, "Saturn"s, "Uranus"s, "Neptune"s }));
}

TEST_CASE("seq::accumulate", "[seq][accumulate]")
{
    REQUIRE((seq::range(1, 101) >>= seq::accumulate(std::plus<>{})) == 5050);
}

TEST_CASE("seq access - non-empty range", "[seq]")
{
    const auto v = std::vector{ 10, 12, 14, 16 };
    REQUIRE((v >>= seq::front) == 10);
    REQUIRE((v >>= seq::maybe_front) == 10);
    REQUIRE((v >>= seq::back) == 16);
    REQUIRE((v >>= seq::maybe_back) == 16);
    REQUIRE((v >>= seq::at(2)) == 14);
    REQUIRE((v >>= seq::maybe_at(2)) == 14);
}

TEST_CASE("seq access - empty range", "[seq]")
{
    const auto v = std::vector<int>{};
    REQUIRE((v >>= seq::maybe_front) == std::nullopt);
    REQUIRE((v >>= seq::maybe_back) == std::nullopt);
    REQUIRE((v >>= seq::maybe_at(2)) == std::nullopt);
    REQUIRE_THROWS(v >>= seq::front);
    REQUIRE_THROWS(v >>= seq::back);
    REQUIRE_THROWS(v >>= seq::at(2));
}

TEST_CASE("seq::all_of", "[seq]")
{
    REQUIRE((std::vector{ 1, 2, 3, 4, 5 } >>= seq::all_of([](int x) { return x < 10; })) == true);
    REQUIRE((std::vector{ 1, 2, 3, 44, 5 } >>= seq::all_of([](int x) { return x < 10; })) == false);
}

TEST_CASE("seq::any_of", "[seq]")
{
    REQUIRE((std::vector{ 1, 2, 3, 4, 5 } >>= seq::any_of([](int x) { return x < 10; })) == true);
    REQUIRE((std::vector{ 1, 2, 3, 44, 5 } >>= seq::any_of([](int x) { return x < 10; })) == true);
    REQUIRE((std::vector{ 10, 12, 13, 44, 15 } >>= seq::any_of([](int x) { return x < 10; })) == false);
}

TEST_CASE("seq::none_of", "[seq]")
{
    REQUIRE((std::vector{ 1, 2, 3, 4, 5 } >>= seq::none_of([](int x) { return x < 10; })) == false);
    REQUIRE((std::vector{ 1, 2, 3, 44, 5 } >>= seq::none_of([](int x) { return x < 10; })) == false);
    REQUIRE((std::vector{ 10, 12, 13, 44, 15 } >>= seq::none_of([](int x) { return x < 10; })) == true);
}

TEST_CASE("seq::concat", "[seq]")
{
    REQUIRE_THAT(seq::concat(std::vector{ 1, 2, 3 }, seq::range(100, 105)), EqualsRange(std::vector{ 1, 2, 3, 100, 101, 102, 103, 104 }));
}
