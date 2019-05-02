cmake_minimum_required(VERSION 3.9)
project(hybrid)

set(CMAKE_CXX_STANDARD 11)
#set(CPLEX_STUDIO_DIR /home/afifi/libs/cplex)
set(CPLEX_STUDIO_DIR /opt/ibm/ILOG/CPLEX_Studio128)
set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -ffast-math")

link_directories(${CPLEX_STUDIO_DIR}/concert/lib/x86-64_linux/static_pic)
link_directories(${CPLEX_STUDIO_DIR}/cplex/lib/x86-64_linux/static_pic)
link_directories(${CPLEX_STUDIO_DIR}/cpoptimizer/lib/x86-64_linux/static_pic)

file(GLOB_RECURSE hybrid_SRC
        "src/**.h"
        "src/**.hpp"
        "src/**.cpp"
        )
add_executable(hybrid ${hybrid_SRC} src/problems/cvrptw/routines/operators/Constructor.cpp src/problems/cvrptw/routines/operators/Constructor.hpp src/problems/cvrptw/models/Client.cpp src/problems/cvrptw/models/Client.hpp src/problems/cvrptw/models/Visit.cpp src/problems/cvrptw/models/Visit.hpp src/problems/cvrptw/models/Depot.cpp src/problems/cvrptw/models/Depot.hpp src/problems/cvrptw/models/Vehicle.cpp src/problems/cvrptw/models/Vehicle.hpp src/problems/cvrptw/models/Problem.cpp
        src/problems/cvrptw/models/Problem.hpp src/problems/cvrptw/models/Tour.cpp src/problems/cvrptw/models/Tour.hpp src/problems/cvrptw/models/Solution.cpp src/problems/cvrptw/models/Solution.hpp src/problems/cvrptw/routines/Callbacks/HeuristicCallback.cpp src/problems/cvrptw/routines/Callbacks/HeuristicCallback.hpp src/problems/cvrptw/Reader.cpp src/problems/cvrptw/Reader.hpp src/problems/cvrptw/routines/operators/Destructor.cpp src/problems/cvrptw/routines/operators/Destructor.hpp src/ConfigureCallbacksCall.h src/data/attributes/InsertionCost.hpp src/problems/cvrp/models/InsertionCost.cpp src/problems/cvrp/models/InsertionCost.hpp src/problems/cvrptw/models/InsertionCost.cpp src/problems/cvrptw/models/InsertionCost.hpp src/problems/cvrptw/solvers/LSSolver.hpp src/solvers/Checker.hpp src/problems/cvrp/solvers/Checker.cpp src/problems/cvrp/solvers/Checker.hpp src/problems/cvrptw/solvers/Checker.cpp src/problems/cvrptw/solvers/Checker.hpp src/Configurations.hpp)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-sign-compare -pedantic")

target_include_directories(hybrid PUBLIC ${CPLEX_STUDIO_DIR}/concert/include)
target_include_directories(hybrid PUBLIC ${CPLEX_STUDIO_DIR}/cplex/include)
target_include_directories(hybrid PUBLIC ${CPLEX_STUDIO_DIR}/cpoptimizer/include)
target_link_libraries(hybrid PUBLIC rt)
target_link_libraries(hybrid PUBLIC dl)
target_link_libraries(hybrid PUBLIC cp)
target_link_libraries(hybrid PUBLIC ilocplex)
target_link_libraries(hybrid PUBLIC cplex)
target_link_libraries(hybrid PUBLIC concert)
target_link_libraries(hybrid PUBLIC pthread)
target_compile_definitions(hybrid PRIVATE _DOUBLE)
target_compile_definitions(hybrid PRIVATE IL_STD)
target_compile_definitions(hybrid PRIVATE MIPSOLVER)
