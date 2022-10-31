#pragma once

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_templated.hpp>
#include <utility>

namespace std
{
template <class... L, class... R>
bool operator==(const pair<L...>& lhs, const pair<R...>& rhs)
{
    return lhs.first == rhs.first && lhs.second == rhs.second;
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
