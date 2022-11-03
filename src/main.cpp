
#include <algorithm>
#include <cpp_pipelines/debug.hpp>
#include <cpp_pipelines/seq.hpp>
#include <functional>
#include <iostream>
#include <iterator>
#include <optional>
#include <sstream>
#include <vector>

void run()
{
    using namespace cpp_pipelines;
    static constexpr auto sqr = [](auto x) { return x * x; };
    seq::range(10)
        |= seq::transform(sqr)
        |= seq::transform([](int x) { return str('"', x, '"'); })
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