set(UNIT_TEST_SOURCE_LIST
  opt.test.cpp
  res.test.cpp
  var.test.cpp
)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/e2239ee6043f73722e7aa812a459f54a28552929.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(cpp_pipelines_tests ${UNIT_TEST_SOURCE_LIST})
include_directories(
  "${PROJECT_SOURCE_DIR}/include"
)

target_link_libraries(cpp_pipelines_tests gtest_main gmock_main)
add_test(NAME cpp_pipelines_tests COMMAND cpp_pipelines_tests)