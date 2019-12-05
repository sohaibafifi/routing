project(basic_tests)
include_directories(../lib/googletest/googletest/include ../lib/googletest/googletest)

add_executable(runBasicCheckTest VrpLSSolverTest.cpp)

target_link_libraries(runBasicCheckTest gtest gtest_main)
target_link_libraries(runBasicCheckTest  utilities)
target_link_libraries(runBasicCheckTest  vrp)
target_link_libraries(runBasicCheckTest  cvrp)