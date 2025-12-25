include_directories(../lib/googletest/googletest/include ../lib/googletest/googletest)

add_executable(composableApiTest test.cpp)
target_include_directories(composableApiTest PUBLIC ../../core/src/)
target_include_directories(composableApiTest PUBLIC ../../)
target_link_libraries(composableApiTest gtest gtest_main routing_plugins ${CPLEX_LIBRARIES})
add_test(composableApiTest composableApiTest)

