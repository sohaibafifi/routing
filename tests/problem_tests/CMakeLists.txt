include_directories(../lib/googletest/googletest/include ../lib/googletest/googletest)

foreach (problem cvrptw cvrpstw)

    add_executable(${problem}Test ProblemTest.hpp ${problem}/test.cpp)
    target_include_directories(${problem}Test PUBLIC ../../examples/problems/cvrp/src/)
    target_include_directories(${problem}Test PUBLIC ../../examples/problems/cvrptw/src/)
    target_include_directories(${problem}Test PUBLIC ../../examples/problems/cvrpstw/src/)
    target_include_directories(${problem}Test PUBLIC ../../core/src/)
    target_include_directories(${problem}Test PUBLIC ../../)
    target_link_libraries(${problem}Test gtest gtest_main routing_plugins ${CPLEX_LIBRARIES})
    add_test(${problem}Test ${problem}Test)


endforeach ()
