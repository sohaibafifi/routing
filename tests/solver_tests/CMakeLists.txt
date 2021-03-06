project(solver_tests)
include_directories(../lib/googletest/googletest/include ../lib/googletest/googletest)
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CPLEX_STUDIO_DIR /Applications/CPLEX_Studio1210/)
    set(CPLEX_STUDIO_ARCH x86-64_osx)
else ()
    set(CPLEX_STUDIO_DIR /opt/ibm/ILOG/CPLEX_Studio1210)
    set(CPLEX_STUDIO_ARCH x86-64_linux)
endif ()


add_executable(VrpLSSolverTest VrpLSSolverTest.cpp)
target_include_directories(VrpLSSolverTest PUBLIC ../../problems/vrp/src/)
target_include_directories(VrpLSSolverTest PUBLIC ../../core/src/)
target_link_libraries(VrpLSSolverTest gtest gtest_main)
target_link_libraries(VrpLSSolverTest  utilities)
target_link_libraries(VrpLSSolverTest  vrp)
if (EXISTS "${CPLEX_STUDIO_DIR}/cplex/include/ilcplex/ilocplexi.h")
    target_compile_definitions(VrpLSSolverTest PRIVATE IL_STD)
    target_compile_definitions(VrpLSSolverTest PRIVATE CPLEX)
    target_link_libraries(VrpLSSolverTest dl)
    target_link_libraries(VrpLSSolverTest cp)
    target_link_libraries(VrpLSSolverTest ilocplex)
    target_link_libraries(VrpLSSolverTest cplex)
    target_link_libraries(VrpLSSolverTest concert)
    target_link_libraries(VrpLSSolverTest pthread)
endif ()
add_test(VrpLSSolverTest VrpLSSolverTest)


add_executable(VrpGASolverTest VrpGASolverTest.cpp)
target_include_directories(VrpGASolverTest PUBLIC ../../problems/vrp/src/)
target_include_directories(VrpGASolverTest PUBLIC ../../core/src/)
target_link_libraries(VrpGASolverTest gtest gtest_main)
target_link_libraries(VrpGASolverTest  utilities)
target_link_libraries(VrpGASolverTest  vrp)
if (EXISTS "${CPLEX_STUDIO_DIR}/cplex/include/ilcplex/ilocplexi.h")
    target_compile_definitions(VrpGASolverTest PRIVATE IL_STD)
    target_compile_definitions(VrpGASolverTest PRIVATE CPLEX)
    target_link_libraries(VrpGASolverTest dl)
    target_link_libraries(VrpGASolverTest cp)
    target_link_libraries(VrpGASolverTest ilocplex)
    target_link_libraries(VrpGASolverTest cplex)
    target_link_libraries(VrpGASolverTest concert)
    target_link_libraries(VrpGASolverTest pthread)
endif ()
add_test(VrpGASolverTest VrpGASolverTest)

add_executable(VrpMASolverTest VrpMASolverTest.cpp)
target_include_directories(VrpMASolverTest PUBLIC ../../problems/vrp/src/)
target_include_directories(VrpMASolverTest PUBLIC ../../core/src/)
target_link_libraries(VrpMASolverTest gtest gtest_main)
target_link_libraries(VrpMASolverTest  utilities)
target_link_libraries(VrpMASolverTest  vrp)
if (EXISTS "${CPLEX_STUDIO_DIR}/cplex/include/ilcplex/ilocplexi.h")
    target_compile_definitions(VrpMASolverTest PRIVATE IL_STD)
    target_compile_definitions(VrpMASolverTest PRIVATE CPLEX)
    target_link_libraries(VrpMASolverTest dl)
    target_link_libraries(VrpMASolverTest cp)
    target_link_libraries(VrpMASolverTest ilocplex)
    target_link_libraries(VrpMASolverTest cplex)
    target_link_libraries(VrpMASolverTest concert)
    target_link_libraries(VrpMASolverTest pthread)
endif ()
add_test(VrpMASolverTest VrpMASolverTest)

add_executable(VrpVNSSolverTest VrpVNSSolverTest.cpp)
target_include_directories(VrpVNSSolverTest PUBLIC ../../problems/vrp/src/)
target_include_directories(VrpVNSSolverTest PUBLIC ../../core/src/)
target_link_libraries(VrpVNSSolverTest gtest gtest_main)
target_link_libraries(VrpVNSSolverTest  utilities)
target_link_libraries(VrpVNSSolverTest  vrp)
if (EXISTS "${CPLEX_STUDIO_DIR}/cplex/include/ilcplex/ilocplexi.h")
    target_compile_definitions(VrpVNSSolverTest PRIVATE IL_STD)
    target_compile_definitions(VrpVNSSolverTest PRIVATE CPLEX)
    target_link_libraries(VrpVNSSolverTest dl)
    target_link_libraries(VrpVNSSolverTest cp)
    target_link_libraries(VrpVNSSolverTest ilocplex)
    target_link_libraries(VrpVNSSolverTest cplex)
    target_link_libraries(VrpVNSSolverTest concert)
    target_link_libraries(VrpVNSSolverTest pthread)
endif ()
add_test(VrpVNSSolverTest VrpVNSSolverTest)


add_executable(CvrpLSSolverTest CvrpLSSolverTest.cpp)
target_include_directories(CvrpLSSolverTest PUBLIC ../../problems/vrp/src/)
target_include_directories(CvrpLSSolverTest PUBLIC ../../problems/cvrp/src/)
target_include_directories(CvrpLSSolverTest PUBLIC ../../core/src/)
target_link_libraries(CvrpLSSolverTest gtest gtest_main)
target_link_libraries(CvrpLSSolverTest  utilities)
target_link_libraries(CvrpLSSolverTest  vrp)
target_link_libraries(CvrpLSSolverTest  cvrp)
if (EXISTS "${CPLEX_STUDIO_DIR}/cplex/include/ilcplex/ilocplexi.h")
    target_compile_definitions(CvrpLSSolverTest PRIVATE IL_STD)
    target_compile_definitions(CvrpLSSolverTest PRIVATE CPLEX)
    target_link_libraries(CvrpLSSolverTest dl)
    target_link_libraries(CvrpLSSolverTest cp)
    target_link_libraries(CvrpLSSolverTest ilocplex)
    target_link_libraries(CvrpLSSolverTest cplex)
    target_link_libraries(CvrpLSSolverTest concert)
    target_link_libraries(CvrpLSSolverTest pthread)
endif ()
add_test(CvrpLSSolverTest CvrpLSSolverTest)



add_executable(CvrpVNSSolverTest CvrpVNSSolverTest.cpp)
target_include_directories(CvrpVNSSolverTest PUBLIC ../../problems/vrp/src/)
target_include_directories(CvrpVNSSolverTest PUBLIC ../../problems/cvrp/src/)
target_include_directories(CvrpVNSSolverTest PUBLIC ../../core/src/)
target_link_libraries(CvrpVNSSolverTest gtest gtest_main)
target_link_libraries(CvrpVNSSolverTest  utilities)
target_link_libraries(CvrpVNSSolverTest  vrp)
target_link_libraries(CvrpVNSSolverTest  cvrp)
if (EXISTS "${CPLEX_STUDIO_DIR}/cplex/include/ilcplex/ilocplexi.h")
    target_compile_definitions(CvrpVNSSolverTest PRIVATE IL_STD)
    target_compile_definitions(CvrpVNSSolverTest PRIVATE CPLEX)
    target_link_libraries(CvrpVNSSolverTest dl)
    target_link_libraries(CvrpVNSSolverTest cp)
    target_link_libraries(CvrpVNSSolverTest ilocplex)
    target_link_libraries(CvrpVNSSolverTest cplex)
    target_link_libraries(CvrpVNSSolverTest concert)
    target_link_libraries(CvrpVNSSolverTest pthread)
endif ()
add_test(CvrpVNSSolverTest CvrpVNSSolverTest)



add_executable(CvrpGASolverTest CvrpGASolverTest.cpp)
target_include_directories(CvrpGASolverTest PUBLIC ../../problems/vrp/src/)
target_include_directories(CvrpGASolverTest PUBLIC ../../problems/cvrp/src/)
target_include_directories(CvrpGASolverTest PUBLIC ../../core/src/)
target_link_libraries(CvrpGASolverTest gtest gtest_main)
target_link_libraries(CvrpGASolverTest utilities)
target_link_libraries(CvrpGASolverTest vrp)
target_link_libraries(CvrpGASolverTest cvrp)
if (EXISTS "${CPLEX_STUDIO_DIR}/cplex/include/ilcplex/ilocplexi.h")
    target_compile_definitions(CvrpGASolverTest PRIVATE IL_STD)
    target_compile_definitions(CvrpGASolverTest PRIVATE CPLEX)
    target_link_libraries(CvrpGASolverTest dl)
    target_link_libraries(CvrpGASolverTest cp)
    target_link_libraries(CvrpGASolverTest ilocplex)
    target_link_libraries(CvrpGASolverTest cplex)
    target_link_libraries(CvrpGASolverTest concert)
    target_link_libraries(CvrpGASolverTest pthread)
endif ()
add_test(CvrpGASolverTest CvrpGASolverTest)


add_executable(CvrptwGASolverTest CvrptwGASolverTest.cpp)
target_include_directories(CvrptwGASolverTest PUBLIC ../../problems/vrp/src/)
target_include_directories(CvrptwGASolverTest PUBLIC ../../problems/cvrp/src/)
target_include_directories(CvrptwGASolverTest PUBLIC ../../problems/cvrptw/src/)
target_include_directories(CvrptwGASolverTest PUBLIC ../../core/src/)
target_link_libraries(CvrptwGASolverTest gtest gtest_main)
target_link_libraries(CvrptwGASolverTest utilities)
target_link_libraries(CvrptwGASolverTest vrp)
target_link_libraries(CvrptwGASolverTest cvrp)
target_link_libraries(CvrptwGASolverTest cvrptw)
if (EXISTS "${CPLEX_STUDIO_DIR}/cplex/include/ilcplex/ilocplexi.h")
    target_compile_definitions(CvrptwGASolverTest PRIVATE IL_STD)
    target_compile_definitions(CvrptwGASolverTest PRIVATE CPLEX)
    target_link_libraries(CvrptwGASolverTest dl)
    target_link_libraries(CvrptwGASolverTest cp)
    target_link_libraries(CvrptwGASolverTest ilocplex)
    target_link_libraries(CvrptwGASolverTest cplex)
    target_link_libraries(CvrptwGASolverTest concert)
    target_link_libraries(CvrptwGASolverTest pthread)
endif ()
add_test(CvrptwGASolverTest CvrptwGASolverTest)


add_executable(CvrptwMIPSolverTest CvrptwMIPSolverTest.cpp)
target_include_directories(CvrptwMIPSolverTest PUBLIC ../../problems/vrp/src/)
target_include_directories(CvrptwMIPSolverTest PUBLIC ../../problems/cvrp/src/)
target_include_directories(CvrptwMIPSolverTest PUBLIC ../../problems/cvrptw/src/)
target_include_directories(CvrptwMIPSolverTest PUBLIC ../../core/src/)
target_link_libraries(CvrptwMIPSolverTest gtest gtest_main)
target_link_libraries(CvrptwMIPSolverTest utilities)
target_link_libraries(CvrptwMIPSolverTest vrp)
target_link_libraries(CvrptwMIPSolverTest cvrp)
target_link_libraries(CvrptwMIPSolverTest cvrptw)
if (EXISTS "${CPLEX_STUDIO_DIR}/cplex/include/ilcplex/ilocplexi.h")
    target_compile_definitions(CvrptwMIPSolverTest PRIVATE IL_STD)
    target_compile_definitions(CvrptwMIPSolverTest PRIVATE CPLEX)
    target_link_libraries(CvrptwMIPSolverTest dl)
    target_link_libraries(CvrptwMIPSolverTest cp)
    target_link_libraries(CvrptwMIPSolverTest ilocplex)
    target_link_libraries(CvrptwMIPSolverTest cplex)
    target_link_libraries(CvrptwMIPSolverTest concert)
    target_link_libraries(CvrptwMIPSolverTest pthread)
endif ()
add_test(CvrptwMIPSolverTest CvrptwMIPSolverTest)