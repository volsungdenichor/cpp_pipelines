#include <gmock/gmock.h>

#include <cpp_pipelines/output.hpp>
#include <cpp_pipelines/reference_wrapper_operators.hpp>
#include <cpp_pipelines/res.hpp>

using namespace ::testing;
using namespace cpp_pipelines;
using namespace std::string_literals;

TEST(res, has_value)
{
    result<int, std::string> r = 10;
    ASSERT_THAT(r.has_value(), true);
    ASSERT_THAT(r.has_error(), false);
    ASSERT_THAT(static_cast<bool>(r), true);
    ASSERT_THAT(r.value(), 10);
}

TEST(res, has_error)
{
    result<int, std::string> r = error("No value");
    ASSERT_THAT(r.has_error(), true);
    ASSERT_THAT(r.has_value(), false);
    ASSERT_THAT(!static_cast<bool>(r), true);
    ASSERT_THAT(r.error(), "No value");
}

TEST(res, pipelines)
{
    static const auto add_brackets = [](const std::string& _) { return "(" + _ + ")"; };

    result<std::string, std::string> ok = "ok";
    result<std::string, std::string> err = error("No value");

    ASSERT_THAT(ok >>= res::maybe_value, std::optional{ "ok"s });

    ASSERT_THAT(ok >>= res::maybe_error, std::nullopt);

    ASSERT_THAT(err >>= res::maybe_value, std::nullopt);

    ASSERT_THAT(err >>= res::maybe_error, std::optional{ "No value"s });

    ASSERT_THAT(ok >>= res::transform(add_brackets) >>= res::value, "(ok)");

    ASSERT_THAT(err >>= res::transform_error(add_brackets) >>= res::error, "(No value)");

    ASSERT_THAT(ok >>= res::value, "ok");

    ASSERT_THAT(ok >>= res::value_or("?"), "ok");

    ASSERT_THAT(err >>= res::value_or("?"), "?");

    ASSERT_THAT(err >>= res::or_else([]() -> result<std::string, std::string> { return "144"; }) >>= res::value, "144");

    ASSERT_THAT(ok >>= res::and_then([](const std::string& _) -> result<std::size_t, std::string> { return _.size(); }) >>= res::value, 2);

    ASSERT_THAT(err >>= res::and_then([](const std::string& _) -> result<std::size_t, std::string> { return _.size(); }) >>= res::error, "No value");

    ASSERT_THAT(ok >>= res::match([](const auto&) { return 1; }, [](const auto&) { return 2; }), 1);
    ASSERT_THAT(err >>= res::match([](const auto&) { return 1; }, [](const auto& e) { return 2; }), 2);
}