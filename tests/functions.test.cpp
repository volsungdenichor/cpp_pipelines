#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_templated.hpp>
#include <cpp_pipelines/debug.hpp>
#include <cpp_pipelines/functions.hpp>
#include <cpp_pipelines/output.hpp>

using namespace cpp_pipelines;
using namespace std::string_literals;

namespace
{
auto create_string() -> std::string
{
    return "?";
}

struct TestStruct
{
    int a;
    int b;
};

}  // namespace

template <class Type>
struct OfTypeMatcher : Catch::Matchers::MatcherGenericBase
{
    template <class T>
    bool match(const T& item) const
    {
        return std::is_same_v<Type, T>;
    }

    std::string describe() const override
    {
        std::stringstream ss;
        ss << "Of type " << type_name<Type>();
        return ss.str();
    }
};

template <class Type>
auto OfType() -> OfTypeMatcher<Type>
{
    return {};
}

TEST_CASE("identity - lvalue", "[functions]")
{
    const auto x = 10;
    decltype(auto) res = identity(x);
    REQUIRE(res == 10);
    REQUIRE(std::addressof(res) == std::addressof(x));
}

TEST_CASE("identity - rvalue", "[functions]")
{
    REQUIRE(identity(create_string()) == "?");
}

TEST_CASE("dereference", "[functions]")
{
    int value = 42;
    REQUIRE(dereference(&value) == value);
    REQUIRE(std::addressof(dereference(&value)) == std::addressof(value));
}

TEST_CASE("addressof", "[functions]")
{
    int value = 42;
    REQUIRE(addressof(value) == std::addressof(value));
}

TEST_CASE("decay_copy", "[functions]")
{
    int value = 42;
    REQUIRE(decay_copy(create_string()) == "?");
}

TEST_CASE("tuple creation", "[functions]")
{
    const auto rvalue = create_string;
    int lvalue = 42;
    const float const_lvalue = 53.0;

    REQUIRE_THAT(make_pair(lvalue, rvalue()), (OfType<std::pair<int, std::string>>()));
    REQUIRE_THAT(tie_as_pair(lvalue, const_lvalue), (OfType<std::pair<int&, const float&>>()));
    REQUIRE_THAT(forward_as_pair(lvalue, rvalue()), (OfType<std::pair<int&, std::string&&>>()));
    REQUIRE_THAT(to_pair(lvalue, rvalue()), (OfType<std::pair<int&, std::string>>()));

    REQUIRE_THAT(make_tuple(lvalue, rvalue()), (OfType<std::tuple<int, std::string>>()));
    REQUIRE_THAT(tie(lvalue, const_lvalue), (OfType<std::tuple<int&, const float&>>()));
    REQUIRE_THAT(forward_as_tuple(lvalue, rvalue()), (OfType<std::tuple<int&, std::string&&>>()));
    REQUIRE_THAT(to_tuple(lvalue, rvalue()), (OfType<std::tuple<int&, std::string>>()));
}

TEST_CASE("tie", "[functions]")
{
    const auto test_struct = TestStruct{ 3, 9 };
    REQUIRE(collect_results(tie)(&TestStruct::b, &TestStruct::a)(test_struct) == std::tie(test_struct.b, test_struct.a));
}

TEST_CASE("make_tuple", "[functions]")
{
    const auto test_struct = TestStruct{ 3, 9 };
    REQUIRE(collect_results(make_tuple)(&TestStruct::b, &TestStruct::a)(test_struct) == std::tuple{ test_struct.b, test_struct.a });
}

TEST_CASE("make_pair", "[functions]")
{
    const auto test_struct = TestStruct{ 3, 9 };
    REQUIRE(collect_results(make_pair)(&TestStruct::b, &TestStruct::a)(test_struct) == std::pair{ test_struct.b, test_struct.a });
}

TEST_CASE("get_element", "[functions][get_element]")
{
    const auto tuple = std::tuple{ 3, 'x', "alpha"s };
    REQUIRE(get_element<0>(tuple) == 3);
    REQUIRE(get_element<1>(tuple) == 'x');
    REQUIRE(get_element<2>(tuple) == "alpha"s);
}

TEST_CASE("tuple getters", "[functions][get_element]")
{
    const auto pair = std::pair{ 'x', 10 };
    REQUIRE(get_first(pair) == 'x');
    REQUIRE(get_second(pair) == 10);
    REQUIRE(get_key(pair) == 'x');
    REQUIRE(get_value(pair) == 10);
}

TEST_CASE("cast", "[functions]")
{
    REQUIRE(cast<int>(3.14) == 3);
    REQUIRE(cast<bool>(3) == true);
}

TEST_CASE("associate", "[functions]")
{
    const auto func = associate([](int x) { return str(10 * x); });
    REQUIRE(func(3) == std::pair{ 3, "30"s });
}

TEST_CASE("hash", "[functions]")
{
    REQUIRE(hash("56a"s) == std::hash<std::string>{}("56a"s));
}

TEST_CASE("bind", "[functions]")
{
    const auto f = [](int a, std::string b, float c) {
        return str(a, ",", b, ",", c);
    };
    REQUIRE(bind_front(f)(4, "abc", 2.71) == "4,abc,2.71"s);
    REQUIRE(bind_front(f, 4)("abc", 2.71) == "4,abc,2.71"s);
    REQUIRE(bind_front(f, 4, "abc")(2.71) == "4,abc,2.71"s);
    REQUIRE(bind_front(f, 4, "abc", 2.71)() == "4,abc,2.71"s);

    REQUIRE(bind_back(f)(4, "abc", 2.71) == "4,abc,2.71"s);
    REQUIRE(bind_back(f, 2.71)(4, "abc") == "4,abc,2.71"s);
    REQUIRE(bind_back(f, "abc", 2.71)(4) == "4,abc,2.71"s);
    REQUIRE(bind_back(f, 4, "abc", 2.71)() == "4,abc,2.71"s);
}

TEST_CASE("proj", "[functions]")
{
    const auto eq = proj(collect_results(tie)(&TestStruct::a, &TestStruct::b), std::equal_to<>{});
    const auto lt = proj(collect_results(tie)(&TestStruct::a, &TestStruct::b), std::less<>{});
    REQUIRE(eq(TestStruct{ 3, 1 }, TestStruct{ 3, 1 }) == true);

    REQUIRE(lt(TestStruct{ 2, 3 }, TestStruct{ 3, 1 }) == true);
    REQUIRE(lt(TestStruct{ 3, 2 }, TestStruct{ 3, 3 }) == true);
}