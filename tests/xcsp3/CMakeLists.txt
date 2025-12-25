include_directories(../lib/googletest/googletest/include ../lib/googletest/googletest)

if(ROUTING_BUILD_XCSP3)

add_executable(xcsp3SolverTest xcsp3_solver_test.cpp ../../plugins/solvers/XCSP3SolverPlugin/XCSP3Backend.cpp)
target_link_libraries(xcsp3SolverTest PUBLIC routing_plugins gtest gtest_main ${CPLEX_LIBRARIES})
target_compile_definitions(xcsp3SolverTest PUBLIC ROUTING_BUILD_XCSP3)
add_test(NAME xcsp3SolverTest COMMAND xcsp3SolverTest)


add_executable(xcsp3BackendTest xcsp3_backend_test.cpp ../../plugins/solvers/XCSP3SolverPlugin/XCSP3Backend.cpp)
target_link_libraries(xcsp3BackendTest PUBLIC routing_plugins gtest gtest_main ${CPLEX_LIBRARIES})
target_compile_definitions(xcsp3BackendTest PUBLIC ROUTING_BUILD_XCSP3)
add_test(NAME xcsp3BackendTest COMMAND xcsp3BackendTest)

endif()