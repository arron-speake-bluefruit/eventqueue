cmake_minimum_required(VERSION 3.10)
project(eventqueue VERSION 0.1.0)

option(ENABLE_TESTING "Enable compilation of unit tests" OFF)
option(BUILD_EXAMPLE "Enable compilation of example program" OFF)

add_library(eventqueue
    "source/eventqueue.c"
    "source/timer_heap.c"
    "source/eq_time.c"
)
target_include_directories(eventqueue PUBLIC "${CMAKE_SOURCE_DIR}/include")

if (${ENABLE_TESTING})
    enable_testing()

    set(tests
        timer_heap
        eventqueue
    )

    set(timer_heap_sources
        "tests/timer_heap_tests.c"
        "source/timer_heap.c"
    )

    set(eventqueue_sources
        "tests/eventqueue_tests.c"
        "source/timer_heap.c"
        "source/eventqueue.c"
        "tests/mock_time.c"
    )

    foreach (test ${tests})
        add_executable(${test}_tests ${${test}_sources})

        target_include_directories(${test}_tests PUBLIC "${CMAKE_SOURCE_DIR}/include")
        target_compile_options(${test}_tests PUBLIC -Wall -Wextra -Wpedantic)

        add_test(NAME ${test}_tests COMMAND ${test}_tests)
        add_test(NAME ${test}_memcheck
            COMMAND valgrind --leak-check=full --error-exitcode=1 "${CMAKE_BINARY_DIR}/${test}_tests"
        )
    endforeach ()
endif ()

if (${BUILD_EXAMPLE})
    add_executable(example "example/main.c")
    target_link_libraries(example PUBLIC eventqueue)
endif ()
