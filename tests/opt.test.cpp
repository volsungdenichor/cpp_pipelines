#include <catch2/catch_test_macros.hpp>
#include <cpp_pipelines/functions.hpp>
#include <cpp_pipelines/opt.hpp>

using namespace cpp_pipelines;
using namespace std::string_literals;

namespace
{
struct God
{
    std::string name;
};
}  // namespace

TEST_CASE("opt::pipelines", "[opt]")
{
    const auto god = God{ "Hades" };

    REQUIRE((opt::lift("Poseidon"s) >>= opt::transform(&std::string::size) >>= opt::value) == 8);

    REQUIRE((std::optional<std::string>{} >>= opt::transform(&std::string::size)) == std::nullopt);

    REQUIRE((std::addressof(opt::lift(god) >>= opt::transform(identity) >>= opt::value)) == std::addressof(god));

    REQUIRE((std::addressof(opt::lift(god) >>= opt::transform(&God::name) >>= opt::value)) == std::addressof(god.name));

    REQUIRE((opt::lift("Poseidon"s) >>= opt::filter([](const std::string&) { return true; }) >>= opt::or_else([]() { return std::optional{ "Ceres"s }; }) >>= opt::value) == "Poseidon");
    REQUIRE((opt::lift("Poseidon"s) >>= opt::filter([](const std::string&) { return false; }) >>= opt::or_else([]() { return std::optional{ "Ceres"s }; }) >>= opt::value) == "Ceres");

    REQUIRE((opt::lift("Poseidon"s) >>= opt::filter([](const std::string&) { return true; }) >>= opt::match([](const std::string&) { return 1; }, []() { return 2; })) == 1);
    REQUIRE((opt::lift("Poseidon"s) >>= opt::filter([](const std::string&) { return false; }) >>= opt::match([](const std::string&) { return 1; }, []() { return 2; })) == 2);
}