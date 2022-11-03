#include <cpp_pipelines/map.hpp>
#include <cpp_pipelines/output.hpp>

#include "test_utils.hpp"

using namespace cpp_pipelines;
using namespace std::string_literals;

TEST_CASE("map::group_by_as", "[map]")
{
    const auto to_upper = [](std::string text) -> std::string {
        std::transform(text.begin(), text.end(), text.begin(), [](char ch) { return std::toupper(ch); });
        return text;
    };
    const auto result = std::vector{ "Anne"s, "Adam"s, "Bart"s, "Ben"s, "Adrian"s } |= map::group_by_as<std::unordered_multimap>([](const std::string& x) -> char { return std::tolower(x[0]); }, to_upper);
    REQUIRE(result == (std::unordered_multimap<char, std::string>{ { 'b', "BEN"s }, { 'b', "BART"s }, { 'a', "ADRIAN"s }, { 'a', "ADAM"s }, { 'a', "ANNE"s } }));
}

TEST_CASE("map::equal_range")
{
    std::map<int, int> m = { { 1, 2 }, { 2, 4 }, { 3, 9 } };
    REQUIRE_THAT(m |= map::equal_range(2), EqualsRange(std::vector<std::pair<int, int>>{ { 2, 4 } }));
    REQUIRE_THAT(m |= map::equal_range(5), EqualsRange(std::vector<std::pair<int, int>>{}));
}

TEST_CASE("map::values_at")
{
    std::map<int, int> m = { { 1, 2 }, { 2, 4 }, { 3, 9 } };
    REQUIRE_THAT(m |= map::values_at(2), EqualsRange(std::vector{ 4 }));
    REQUIRE_THAT(m |= map::values_at(5), EqualsRange(std::vector<int>{}));
}

TEST_CASE("map::maybe_at")
{
    std::map<int, int> m = { { 1, 2 }, { 2, 4 }, { 3, 9 } };
    REQUIRE((m |= map::maybe_at(2)) == 4);
    REQUIRE((m |= map::maybe_at(5)) == std::nullopt);
}

TEST_CASE("map::at")
{
    std::map<int, int> m = { { 1, 2 }, { 2, 4 }, { 3, 9 } };
    REQUIRE((m |= map::at(2)) == 4);
    REQUIRE_THROWS(m |= map::at(5));
}

TEST_CASE("map::keys")
{
    std::multimap<int, int> m = { { 1, 2 }, { 2, 4 }, { 3, 9 }, { 1, 5 }, { 2, 4 } };
    REQUIRE_THAT(m |= map::keys, EqualsRange(std::vector{ 1, 2, 3 }));
}

TEST_CASE("map::items")
{
    std::multimap<int, int> m = { { 1, 2 }, { 2, 4 }, { 3, 9 }, { 1, 5 }, { 2, 4 } };
    REQUIRE_THAT(m |= map::items, EqualsRange(std::vector<std::pair<int, std::vector<int>>>{ { 1, { 2, 5 } }, { 2, { 4, 4 } }, { 3, { 9 } } }));
}
