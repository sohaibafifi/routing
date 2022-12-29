cmake_minimum_required(VERSION 3.12)
project(tests)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_OSX_ARCHITECTURES "x86_64")
set(GOOGLETEST_VERSION 1.10.0)
enable_testing()
add_subdirectory(lib/googletest/googletest)
add_subdirectory(solver_tests)


