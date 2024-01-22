#include <algorithm>
#include <cassert>
#include <cpp_pipelines/arrays.hpp>
#include <cpp_pipelines/debug.hpp>
#include <cpp_pipelines/format.hpp>
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
    std::array<double, 2 * 3 * 2> buffer;
    std::iota(buffer.begin(), buffer.end(), 0);
    using matrix_ref = cpp_pipelines::array_ref<double, cpp_pipelines::shape<cpp_pipelines::dim<2, sizeof(double)>, cpp_pipelines::dim<3, 2 * sizeof(double)>, cpp_pipelines::dim<2, 2 * 3 * sizeof(double)>>>;
    matrix_ref m{ buffer.data() };
    std::cout << cpp_pipelines::delimit(m.slice<1>(0).values_and_coords(), "\n") << std::endl;
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
