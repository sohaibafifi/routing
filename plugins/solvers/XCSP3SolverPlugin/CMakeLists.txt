add_library(XCSP3SolverPlugin INTERFACE)
target_include_directories(XCSP3SolverPlugin INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(XCSP3SolverPlugin INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}/XCSP3Backend.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/XCSP3Backend.cpp
)
