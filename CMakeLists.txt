cmake_minimum_required(VERSION 3.10)
project(eventqueue VERSION 0.1.0)

option(ENABLE_TESTING "Enable compilation of unit tests" OFF)

add_library(eventqueue
    "source/eventqueue.c"
)
target_include_directories(eventqueue PUBLIC "${CMAKE_SOURCE_DIR}/include")

if (${ENABLE_TESTING})
    enable_testing()

    add_executable(tests tests/tests.c)
    target_link_libraries(tests PUBLIC eventqueue)

    add_test(NAME tests COMMAND tests)
    add_test(NAME tests_memcheck COMMAND valgrind "${CMAKE_BINARY_DIR}/tests")
endif ()
