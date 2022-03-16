#include <catch.hpp>
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
    REQUIRE((std::optional{ "Poseidon"s } >>= opt::transform(&std::string::size)) == std::optional{ 8 });
    auto god = God{ "Zeus" };
    REQUIRE((std::addressof(god >>= opt::lift >>= opt::transform(&God::name) >>= opt::value)) == std::addressof(god.name));
}