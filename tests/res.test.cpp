#include <catch2/catch_test_macros.hpp>
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

    REQUIRE((ok |= res::maybe_value) == std::optional{ "ok"s });

    REQUIRE((ok |= res::maybe_error) == std::nullopt);

    REQUIRE((err |= res::maybe_value) == std::nullopt);

    REQUIRE((err |= res::maybe_error) == std::optional{ "No value"s });

    REQUIRE((ok |= res::transform(add_brackets) |= res::value) == "(ok)");

    REQUIRE((err |= res::transform_error(add_brackets) |= res::error) == "(No value)");

    REQUIRE((ok |= res::value) == "ok");

    REQUIRE((ok |= res::value_or("?")) == "ok");

    REQUIRE((err |= res::value_or("?")) == "?");

    REQUIRE((err |= res::or_else([]() -> result<std::string, std::string> { return "144"; }) |= res::value) == "144");

    REQUIRE((ok |= res::and_then([](const std::string& _) -> result<std::size_t, std::string> { return _.size(); }) |= res::value) == 2);

    REQUIRE((err |= res::and_then([](const std::string& _) -> result<std::size_t, std::string> { return _.size(); }) |= res::error) == "No value");

    REQUIRE((ok |= res::match([](const auto&) { return 1; }, [](const auto&) { return 2; })) == 1);
    REQUIRE((err |= res::match([](const auto&) { return 1; }, [](const auto& e) { return 2; })) == 2);
}