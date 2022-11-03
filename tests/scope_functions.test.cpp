#include <cpp_pipelines/scope_functions.hpp>

#include "test_utils.hpp"

using namespace cpp_pipelines;
using namespace std::string_literals;
using namespace std::string_view_literals;

TEST_CASE("do_all")
{
    std::string result;
    std::tuple{ 1, 5 } >>= do_all(
        [&](int a, int b) { result += str(a, ",", b); },
        [&](int a, int b) { result += str("[", a, "][", b, "]"); },
        [&](int a, int b) { result += str("(", a, ")(", b, ")"); });

    REQUIRE(result == "1,5[1][5](1)(5)");
}
