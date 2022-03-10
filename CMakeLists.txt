cmake_minimum_required (VERSION 3.5)
project (cpp_pipelines)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

include_directories(cpp_pipelines PRIVATE include)
add_subdirectory (src)
add_subdirectory (tests)