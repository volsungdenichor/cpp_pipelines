
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
    int a = 0;
    int b = 1;
    std::cout << (std::tie(a, b) == std::tuple{ 0, 1 }) << std::endl;
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