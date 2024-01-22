set(TARGET_NAME tests)

set(UNIT_TEST_SOURCE_LIST
  opt.test.cpp
  res.test.cpp
  var.test.cpp
  seq.test.cpp
  sub.test.cpp
  map.test.cpp
  scope_functions.test.cpp
  functions.test.cpp
  set.test.cpp
  format.test.cpp
)

Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.0.1 # or a later release
)

FetchContent_MakeAvailable(Catch2)

add_executable(${TARGET_NAME} ${UNIT_TEST_SOURCE_LIST})
include_directories(
  "${PROJECT_SOURCE_DIR}/include")

target_link_libraries(${TARGET_NAME} PRIVATE Catch2::Catch2WithMain)

add_test(
  NAME ${TARGET_NAME}
  COMMAND ${TARGET_NAME} -o report.xml -r junit)