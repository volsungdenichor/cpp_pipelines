
#include <algorithm>
#include <cpp_pipelines/debug.hpp>
#include <cpp_pipelines/operators.hpp>
#include <cpp_pipelines/out_argument.hpp>
#include <cpp_pipelines/seq.hpp>
#include <functional>
#include <iostream>
#include <iterator>
#include <optional>
#include <sstream>
#include <vector>

struct quote_fn
{
    template <class T>
    std::string operator()(const T& item) const
    {
        return cpp_pipelines::str('"', item, '"');
    }
};

static constexpr inline auto quote = quote_fn{};

void run()
{
    using namespace cpp_pipelines;
    static constexpr auto sqr = [](auto x) { return x * x; };
    seq::iota(0)
        |= seq::transform(sqr)
        |= seq::take_while(less(100))
        |= seq::transform(quote)
        |= seq::write(std::cout, "\n");
}

int main()
{
    try
    {
        run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "exception: " << e.what() << '\n';
    }
}