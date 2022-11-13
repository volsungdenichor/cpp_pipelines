#include <algorithm>
#include <cpp_pipelines/debug.hpp>
#include <cpp_pipelines/format.hpp>
#include <cpp_pipelines/geo/interval.hpp>
#include <cpp_pipelines/geo/matrix.hpp>
#include <cpp_pipelines/geo/vertex_array.hpp>
#include <cpp_pipelines/operators.hpp>
#include <cpp_pipelines/out_argument.hpp>
#include <cpp_pipelines/seq.hpp>
#include <cpp_pipelines/tpl.hpp>
#include <functional>
#include <iostream>
#include <iterator>
#include <optional>
#include <sstream>
#include <vector>

void run()
{
    // using namespace cpp_pipelines;
    // std::cout << ((geo::make_interval(0, 5) + 1) * 10) << std::endl;
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