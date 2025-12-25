cmake_minimum_required(VERSION 3.15)
project(routing_python LANGUAGES CXX)

# Find Python and nanobind
find_package(Python 3.8 COMPONENTS Interpreter Development.Module REQUIRED)

# Try to find nanobind via pip install or as submodule
execute_process(
    COMMAND "${Python_EXECUTABLE}" -m nanobind --cmake_dir
    OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE nanobind_ROOT)

if(nanobind_ROOT)
    list(APPEND CMAKE_PREFIX_PATH "${nanobind_ROOT}")
endif()

find_package(nanobind CONFIG)

if(NOT nanobind_FOUND)
    message(STATUS "nanobind not found via pip, trying FetchContent...")
    include(FetchContent)
    FetchContent_Declare(
        nanobind
        GIT_REPOSITORY https://github.com/wjakob/nanobind
        GIT_TAG v2.2.0
    )
    FetchContent_MakeAvailable(nanobind)
endif()

# Create the Python module
nanobind_add_module(
    _routing_core
    STABLE_ABI
    NB_STATIC
    src/bindings.cpp
    src/problem_bindings.cpp
    src/solution_bindings.cpp
    src/solver_bindings.cpp
    src/attribute_bindings.cpp
)

target_include_directories(_routing_core PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../plugins
)

target_link_libraries(_routing_core PRIVATE routing routing_plugins ${CPLEX_LIBRARIES})

# Propagate CPLEX compile definitions
if(CPLEX_FOUND)
    target_compile_definitions(_routing_core PRIVATE CPLEX_FOUND)
endif()

# Copy Python package files to build directory
configure_file(routing/__init__.py ${CMAKE_CURRENT_BINARY_DIR}/routing/__init__.py COPYONLY)
configure_file(routing/problem.py ${CMAKE_CURRENT_BINARY_DIR}/routing/problem.py COPYONLY)
configure_file(routing/solver.py ${CMAKE_CURRENT_BINARY_DIR}/routing/solver.py COPYONLY)

# Install the module
install(TARGETS _routing_core LIBRARY DESTINATION routing)
install(DIRECTORY routing/ DESTINATION routing FILES_MATCHING PATTERN "*.py")
