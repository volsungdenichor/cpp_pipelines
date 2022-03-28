#include <catch.hpp>
#include <cpp_pipelines/output.hpp>
#include <cpp_pipelines/reference_wrapper_operators.hpp>
#include <cpp_pipelines/res.hpp>

using namespace cpp_pipelines;
using namespace std::string_literals;

TEST_CASE("res: has_value", "[res]")
{
    result<int, std::string> r = 10;
    REQUIRE(r.has_value());
    REQUIRE(static_cast<bool>(r));
    REQUIRE(r.value() == 10);
}

TEST_CASE("res: has_error", "[res]")
{
    result<int, std::string> r = error("No value");
    REQUIRE(r.has_error());
    REQUIRE(!static_cast<bool>(r));
    REQUIRE(r.error() == "No value");
}

TEST_CASE("res: pipelines", "[res]")
{
    static const auto add_brackets = [](const std::string& _) { return "(" + _ + ")"; };

    result<std::string, std::string> ok = "ok";
    result<std::string, std::string> err = error("No value");
    REQUIRE((ok >>= res::maybe_value) == std::optional{ "ok"s });
    REQUIRE((ok >>= res::maybe_error) == std::nullopt);
    REQUIRE((err >>= res::maybe_value) == std::nullopt);
    REQUIRE((err >>= res::maybe_error) == std::optional{ "No value"s });
    REQUIRE((ok >>= res::transform(add_brackets) >>= res::value) == "(ok)"s);
    REQUIRE((err >>= res::transform_error(add_brackets) >>= res::error) == "(No value)"s);
}