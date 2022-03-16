#include <catch.hpp>
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

TEST_CASE("opt::transform", "[opt]")
{
    REQUIRE((opt::lift("Poseidon"s) >>= opt::transform(&std::string::size)) == opt::lift(8));
    REQUIRE((std::optional<std::string>{} >>= opt::transform(&std::string::size)) == std::nullopt);
    const auto god = God{ "Hades" };
    REQUIRE((std::addressof(opt::lift(god) >>= opt::transform(identity) >>= opt::value)) == std::addressof(god));
    REQUIRE((std::addressof(opt::lift(god) >>= opt::transform(&God::name) >>= opt::value)) == std::addressof(god.name));
}