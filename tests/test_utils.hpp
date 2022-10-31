#pragma once

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_templated.hpp>
#include <cpp_pipelines/output.hpp>
#include <utility>

namespace std
{
template <class... L, class... R>
bool operator==(const pair<L...>& lhs, const pair<R...>& rhs)
{
    return lhs.first == rhs.first && lhs.second == rhs.second;
}

template <class... Args>
ostream& operator<<(ostream& os, const vector<Args...>& item)
{
    return os << cpp_pipelines::delimit(item, ", ");
}
}  // namespace std

template <class Range>
struct EqualsRangeMatcher : Catch::Matchers::MatcherGenericBase
{
    EqualsRangeMatcher(const Range& range)
        : range{ range }
    {
    }

    template <class OtherRange>
    bool match(const OtherRange& other) const
    {
        return std::equal(std::begin(range), std::end(range), std::begin(other), std::end(other));
    }

    std::string describe() const override
    {
        return "Equals: " + Catch::rangeToString(range);
    }

private:
    const Range& range;
};

template <class Range>
auto EqualsRange(const Range& range) -> EqualsRangeMatcher<Range>
{
    return EqualsRangeMatcher<Range>{ range };
}

namespace cpp_pipelines
{
template <class Impl>
std::ostream& operator<<(std::ostream& os, const view_interface<Impl>& item)
{
    return os << delimit(item, ", ");
}

template <class Impl, class R>
bool operator==(const view_interface<Impl>& lhs, const R& rhs)
{
    return std::equal(std::begin(lhs), std::end(lhs), std::begin(rhs), std::end(rhs));
}

template <class L, class Impl>
bool operator==(const L& lhs, const view_interface<Impl>& rhs)
{
    return rhs == lhs;
}

}  // namespace cpp_pipelines