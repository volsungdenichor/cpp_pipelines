#include <cpp_pipelines/sub.hpp>

#include "test_utils.hpp"

using namespace cpp_pipelines;
using namespace std::string_literals;
using namespace std::string_view_literals;

TEST_CASE("sub::take", "[sub][slice]")
{
    const auto v = std::vector{ 1, 2, 3, 4, 5, 6 };
    REQUIRE_THAT(v |= sub::take(3), EqualsRange(std::vector{ 1, 2, 3 }));
}

TEST_CASE("sub::take_last", "[sub][slice]")
{
    const auto v = std::vector{ 1, 2, 3, 4, 5, 6 };
    REQUIRE_THAT(v |= sub::take_last(3), EqualsRange(std::vector{ 4, 5, 6 }));
}

TEST_CASE("sub::take_while", "[sub][slice]")
{
    const auto v = std::vector{ 1, 2, 3, 4, 5, 6 };
    REQUIRE_THAT(v |= sub::take_while([](int x) { return x < 4; }), EqualsRange(std::vector{ 1, 2, 3 }));
}

TEST_CASE("sub::take_last_while", "[sub][slice]")
{
    const auto v = std::vector{ 1, 2, 3, 4, 5, 6 };
    REQUIRE_THAT(v |= sub::take_last_while([](int x) { return x > 4; }), EqualsRange(std::vector{ 5, 6 }));
}

TEST_CASE("sub::drop", "[sub][slice]")
{
    const auto v = std::vector{ 1, 2, 3, 4, 5, 6 };
    REQUIRE_THAT(v |= sub::drop(3), EqualsRange(std::vector{ 4, 5, 6 }));
}

TEST_CASE("sub::drop_last", "[sub][slice]")
{
    const auto v = std::vector{ 1, 2, 3, 4, 5, 6 };
    REQUIRE_THAT(v |= sub::drop_last(3), EqualsRange(std::vector{ 1, 2, 3 }));
}

TEST_CASE("sub::drop_while", "[sub][slice]")
{
    const auto v = std::vector{ 1, 2, 3, 4, 5, 6 };
    REQUIRE_THAT(v |= sub::drop_while([](int x) { return x < 4; }), EqualsRange(std::vector{ 4, 5, 6 }));
}

TEST_CASE("sub::drop_last_while", "[sub][slice]")
{
    const auto v = std::vector{ 1, 2, 3, 4, 5, 6 };
    REQUIRE_THAT(v |= sub::drop_last_while([](int x) { return x > 4; }), EqualsRange(std::vector{ 1, 2, 3, 4 }));
}

TEST_CASE("sub::slice", "[sub][slice]")
{
    const auto v = std::vector{ 1, 2, 3, 4, 5, 6 };
    REQUIRE_THAT(v |= sub::slice({}, {}), EqualsRange(std::vector{ 1, 2, 3, 4, 5, 6 }));
    REQUIRE_THAT(v |= sub::slice(2, {}), EqualsRange(std::vector{ 3, 4, 5, 6 }));
    REQUIRE_THAT(v |= sub::slice({}, 3), EqualsRange(std::vector{ 1, 2, 3 }));
    REQUIRE_THAT(v |= sub::slice(1, 4), EqualsRange(std::vector{ 2, 3, 4 }));
    REQUIRE_THAT(v |= sub::slice(-2, {}), EqualsRange(std::vector{ 5, 6 }));
    REQUIRE_THAT(v |= sub::slice({}, -2), EqualsRange(std::vector{ 1, 2, 3, 4 }));
    REQUIRE_THAT(v |= sub::slice(10, 20), EqualsRange(std::vector<int>{}));
}

TEST_CASE("sub::trim_while", "[sub][trim_while]")
{
    const auto v = std::vector{ 0, 0, 1, 2, 3, 4, 0, 0 };
    REQUIRE_THAT(v |= sub::trim_while([](int x) { return x == 0; }), EqualsRange(std::vector{ 1, 2, 3, 4 }));
}
