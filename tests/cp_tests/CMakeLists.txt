include_directories(../lib/googletest/googletest/include ../lib/googletest/googletest)

if(CPLEX_FOUND)

add_executable(cpBackendTest cp_backend_test.cpp)
target_link_libraries(cpBackendTest PUBLIC routing_plugins gtest gtest_main ${CPLEX_LIBRARIES})
add_test(NAME cpBackendTest COMMAND cpBackendTest)

add_executable(cpSolverTest cp_solver_test.cpp)
target_link_libraries(cpSolverTest PUBLIC routing_plugins gtest gtest_main ${CPLEX_LIBRARIES})
add_test(NAME cpSolverTest COMMAND cpSolverTest)

endif()

if(ORTOOLS_FOUND)

add_executable(ortoolsBackendTest ortools_backend_test.cpp)
target_link_libraries(ortoolsBackendTest PUBLIC routing_plugins gtest gtest_main ${ORTOOLS_LIBRARIES})
add_test(NAME ortoolsBackendTest COMMAND ortoolsBackendTest)

endif()
