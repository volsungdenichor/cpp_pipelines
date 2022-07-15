#include <gmock/gmock.h>

#include <cpp_pipelines/functions.hpp>
#include <cpp_pipelines/opt.hpp>

using namespace ::testing;
using namespace cpp_pipelines;
using namespace std::string_literals;

namespace
{
struct God
{
    std::string name;
};
}  // namespace

TEST(opt, should_run)
{
    const auto god = God{ "Hades" };

    ASSERT_THAT(opt::lift("Poseidon"s) >>= opt::transform(&std::string::size) >>= opt::value, 8);

    ASSERT_THAT(std::optional<std::string>{} >>= opt::transform(&std::string::size), std::nullopt);

    ASSERT_THAT(opt::lift(god) >>= opt::transform(identity) >>= opt::value, Ref(god));

    ASSERT_THAT(opt::lift(god) >>= opt::transform(&God::name) >>= opt::value, Ref(god.name));

    ASSERT_THAT(opt::lift("Poseidon"s) >>= opt::filter([](const std::string&) { return true; }) >>= opt::or_else([]() { return std::optional{ "Ceres"s }; }) >>= opt::value, "Poseidon");
    ASSERT_THAT(opt::lift("Poseidon"s) >>= opt::filter([](const std::string&) { return false; }) >>= opt::or_else([]() { return std::optional{ "Ceres"s }; }) >>= opt::value, "Ceres");

    ASSERT_THAT(opt::lift("Poseidon"s) >>= opt::filter([](const std::string&) { return true; }) >>= opt::match([](const std::string&) { return 1; }, []() { return 2; }), 1);
    ASSERT_THAT(opt::lift("Poseidon"s) >>= opt::filter([](const std::string&) { return false; }) >>= opt::match([](const std::string&) { return 1; }, []() { return 2; }), 2);
}