#include <catch2/catch_test_macros.hpp>
#include <cpp_pipelines/set.hpp>

using namespace cpp_pipelines;
using namespace std::string_literals;

TEST_CASE("set::sum", "[set]")
{
    REQUIRE(set::sum(std::set{ 1, 2, 3 }, std::set{ 2, 4, 5 }) == std::set{ 1, 2, 3, 4, 5 });
}

TEST_CASE("set::difference", "[set]")
{
    REQUIRE(set::difference(std::set{ 1, 2, 3 }, std::set{ 2, 4, 5 }) == std::set{ 1, 3 });
}

TEST_CASE("set::intersection", "[set]")
{
    REQUIRE(set::intersection(std::set{ 1, 2, 3 }, std::set{ 2, 4, 5 }) == std::set{ 2 });
}

TEST_CASE("set::includes", "[set]")
{
    REQUIRE(set::includes(std::set<int>{}, std::set<int>{}) == true);
    REQUIRE(set::includes(std::set{ 1, 2, 3 }, std::set{ 1, 2, 3 }) == true);
    REQUIRE(set::includes(std::set{ 1, 2, 3 }, std::set{ 2, 3 }) == true);
    REQUIRE(set::includes(std::set{ 1, 2, 3 }, std::set{ 3 }) == true);
    REQUIRE(set::includes(std::set{ 1, 2, 3 }, std::set{ 2, 4, 5 }) == false);
}
