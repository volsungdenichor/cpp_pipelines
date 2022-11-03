#include <catch2/catch_test_macros.hpp>
#include <cpp_pipelines/format.hpp>
#include <cpp_pipelines/opt.hpp>

using namespace cpp_pipelines;
using namespace std::string_literals;

TEST_CASE("format - invalid formatter", "[format]")
{
    REQUIRE_THROWS_AS(format("{}{")(3), format_error);
    REQUIRE_THROWS_AS(format("{}={}={}={}")(3, 5, 7), format_error);
}

TEST_CASE("format", "[format]")
{
    REQUIRE(format("{{{}}}-{}")(1, 42) == "{1}-42"s);
    REQUIRE(format("{}-{}")(1, 42) == "1-42"s);
}

TEST_CASE("format - indexed fields", "[format]")
{
    REQUIRE(format("{{{1}}}-{0}")(1, 42) == "{42}-1"s);
    REQUIRE(format("{1}-{1}")(1, 42) == "42-42"s);
}

TEST_CASE("format - operators", "[format]")
{
    REQUIRE((std::tuple{ 4, 'x', 3 } >>= format("{}, {}, {}")) == "4, x, 3"s);
    REQUIRE((987 |= format("__{}__")) == "__987__"s);
}