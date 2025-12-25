include_directories(../lib/googletest/googletest/include ../lib/googletest/googletest)

foreach (problem cvrptw cvrpstw)

    add_executable(${problem}Test ProblemTest.hpp ${problem}/test.cpp)
    target_include_directories(${problem}Test PUBLIC ../../problems/vrp/src/)
    target_include_directories(${problem}Test PUBLIC ../../problems/cvrp/src/)
    target_include_directories(${problem}Test PUBLIC ../../problems/cvrptw/src/)
    target_include_directories(${problem}Test PUBLIC ../../problems/cvrpstw/src/)
    target_include_directories(${problem}Test PUBLIC ../../core/src/)
    target_include_directories(${problem}Test PUBLIC ../../)
    target_link_libraries(${problem}Test gtest gtest_main)
    target_link_libraries(${problem}Test vrp)
    target_link_libraries(${problem}Test cvrp)
    target_link_libraries(${problem}Test cvrptw)
    target_link_libraries(${problem}Test cvrpstw)
    add_test(${problem}Test ${problem}Test)


endforeach ()
