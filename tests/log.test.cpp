#include <cpp_pipelines/log.hpp>

#include "test_utils.hpp"

using namespace cpp_pipelines;
using namespace std::string_literals;

auto func(int x) -> log::wrapper<std::string>
{
    return log::lift(str(10 * x), log::logs_t{ "function" });
}

struct append_to
{
    std::string& dest;

    void operator()(const log::log_t& log) const
    {
        dest += str(log, '\n');
    }
};

TEST_CASE("log::and_then", "[log]")
{
    std::string logs;

    const auto result = func(1)
        |= log::and_then([](const std::string& x) -> log::wrapper<std::string> {
               return log::lift(str("'", x, "'"), log::logs_t{ "and_then" });
           })
        |= log::value_and_flush(append_to{ logs });
    REQUIRE(result == "'10'");
    REQUIRE(logs == "function\nand_then\n");
}

TEST_CASE("log::transform", "[log]")
{
    std::string logs;

    const auto result = func(1)
        |= log::transform([](const std::string& x) -> std::string {
               return str("'", x, "'");
           })
        |= log::value_and_flush(append_to{ logs });
    REQUIRE(result == "'10'");
    REQUIRE(logs == "function\n");
}

TEST_CASE("log::append_logs", "[log]")
{
    std::string logs;

    const auto result = func(1)
        |= log::append_logs({ "log_0", "log_1" })
        |= log::value_and_flush(append_to{ logs });
    REQUIRE(result == "10");
    REQUIRE(logs == "function\nlog_0\nlog_1\n");
}

TEST_CASE("log::invoke", "[log]")
{
    std::string logs;

    const auto result = 2
        |= log::invoke(func, "func")
        |= log::value_and_flush(append_to{ logs });
    REQUIRE(result == "20");
    REQUIRE(logs == "func <- (2)\nfunction\nfunc -> 20\n");
}
