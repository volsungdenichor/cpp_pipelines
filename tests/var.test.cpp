#include <gmock/gmock.h>

#include <cpp_pipelines/output.hpp>
#include <cpp_pipelines/var.hpp>

using namespace ::testing;
using namespace cpp_pipelines;
using namespace std::string_literals;

TEST(var, ress)
{
    ASSERT_THAT((std::variant<int, char>{ 3 } >>= var::match([](int) { return 1; }, [](char) { return 2; })), 1);
    ASSERT_THAT((std::variant<int, char>{ 'x' } >>= var::match([](int) { return 1; }, [](char) { return 2; })), 2);
}
