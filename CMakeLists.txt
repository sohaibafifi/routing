cmake_minimum_required(VERSION 3.12)
project(routing)
enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_OSX_ARCHITECTURES "x86_64")

set(CMAKE_VERBOSE_MAKEFILE on)


add_subdirectory(examples examples)