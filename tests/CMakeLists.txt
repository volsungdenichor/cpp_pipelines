set(UNIT_TEST_SOURCE_LIST
  opt.test.cpp
  res.test.cpp
)

set(TARGET_NAME tests)

add_executable(${TARGET_NAME}
  main.cpp
  ${UNIT_TEST_SOURCE_LIST})

include_directories(
    "${PROJECT_SOURCE_DIR}/include")

target_include_directories(${TARGET_NAME}
  PUBLIC ../3rd_party/catch2/)

add_test(
  NAME ${TARGET_NAME}
  COMMAND ${TARGET_NAME} -o report.xml -r junit)